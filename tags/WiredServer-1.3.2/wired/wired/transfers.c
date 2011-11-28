/* $Id$ */

/*
 *  Copyright (c) 2003-2007 Axel Andersson
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include <sys/fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <wired/wired.h>

#include "files.h"
#include "main.h"
#include "server.h"
#include "settings.h"
#include "transfers.h"

#define WD_TRANSFERS_TIMER_INTERVAL		60.0
#define WD_TRANSFERS_WAITING_INTERVAL	20.0

#define WD_TRANSFER_BUFFER_SIZE			8192


static void								wd_transfers_update_waiting(wi_timer_t *);

static void								wd_transfers_update_queue(void);
static wi_integer_t						wd_transfers_compare_user(wi_runtime_instance_t *, wi_runtime_instance_t *);

static wd_transfer_t *					wd_transfer_alloc(void);
static wd_transfer_t *					wd_transfer_init_with_user(wd_transfer_t *, wd_user_t *);
static wd_transfer_t *					wd_transfer_init_download_with_user(wd_transfer_t *, wd_user_t *);
static wd_transfer_t *					wd_transfer_init_upload_with_user(wd_transfer_t *, wd_user_t *);
static void								wd_transfer_dealloc(wi_runtime_instance_t *);
static wi_string_t *					wd_transfer_description(wi_runtime_instance_t *);

static void								wd_transfer_set_state(wd_transfer_t *, wd_transfer_state_t);

static void								wd_transfer_download(wd_transfer_t *);
static void								wd_transfer_upload(wd_transfer_t *);


wi_array_t								*wd_transfers;

static wi_timer_t						*wd_transfers_timer;

static wi_lock_t						*wd_transfers_status_lock;
static wi_uinteger_t					wd_transfers_active_downloads, wd_transfers_active_uploads;

static wi_lock_t						*wd_transfers_update_queue_lock;

static wi_runtime_id_t					wd_transfer_runtime_id = WI_RUNTIME_ID_NULL;
static wi_runtime_class_t				wd_transfer_runtime_class = {
	"wd_transfer_t",
	wd_transfer_dealloc,
	NULL,
	NULL,
	wd_transfer_description,
	NULL
};


void wd_transfers_init(void) {
	wd_transfer_runtime_id = wi_runtime_register_class(&wd_transfer_runtime_class);

	wd_transfers = wi_array_init(wi_array_alloc());

	wd_transfers_timer = wi_timer_init_with_function(wi_timer_alloc(),
													 wd_transfers_update_waiting,
													 WD_TRANSFERS_TIMER_INTERVAL,
													 true);
	
	wd_transfers_status_lock = wi_lock_init(wi_lock_alloc());
	wd_transfers_update_queue_lock = wi_lock_init(wi_lock_alloc());
}



void wd_transfers_schedule(void) {
	wi_timer_schedule(wd_transfers_timer);
}



static void wd_transfers_update_waiting(wi_timer_t *timer) {
	wd_transfer_t		*transfer;
	wi_time_interval_t	interval;
	wi_boolean_t		update = false;
	wi_uinteger_t		i, count;
	
	wi_array_wrlock(wd_transfers);
	
	count = wi_array_count(wd_transfers);

	if(count > 0) {
		interval = wi_time_interval();

		for(i = 0; i < count; i++) {
			transfer = WI_ARRAY(wd_transfers, i);

			if(transfer->state == WD_TRANSFER_WAITING) {
				if(transfer->waiting_time + WD_TRANSFERS_WAITING_INTERVAL < interval) {
					wi_lock_lock(wd_transfers_status_lock);
					
					if(transfer->type == WD_TRANSFER_DOWNLOAD) {
						wd_transfers_active_downloads--;
						wd_user_decrease_downloads(transfer->user);
					} else {
						wd_transfers_active_uploads--;
						wd_user_decrease_uploads(transfer->user);
					}

					wi_lock_unlock(wd_transfers_status_lock);
					
					wi_array_remove_data_at_index(wd_transfers, i);
					
					count--;
					i--;
					update = true;
				}
			}
		}
	}

	wi_array_unlock(wd_transfers);

	if(update)
		wd_transfers_update_queue();
}



#pragma mark -

void wd_transfers_queue_download(wi_string_t *path, wi_file_offset_t offset) {
	wd_user_t			*user = wd_users_user_for_thread();
	wi_string_t			*realpath;
	wd_transfer_t		*transfer;
	struct stat			sb;
	
	realpath = wd_files_real_path(path);
	wi_string_resolve_aliases_in_path(realpath);
	
	if(!wi_file_stat(realpath, &sb)) {
		wi_log_err(WI_STR("Could not open %@: %m"), realpath);
		wd_reply_error();

		return;
	}
	
	transfer				= wi_autorelease(wd_transfer_init_download_with_user(wd_transfer_alloc(), user));
	transfer->path			= wi_retain(path);
	transfer->realpath		= wi_retain(realpath);
	transfer->size			= sb.st_size;
	transfer->offset		= offset;
	transfer->transferred	= offset;
	
	wi_lock_lock(wd_transfers_update_queue_lock);
	
	wi_array_wrlock(wd_transfers);
	wi_array_add_data(wd_transfers, transfer);
	wi_array_unlock(wd_transfers);
	
	wd_transfers_update_queue();

	wi_lock_unlock(wd_transfers_update_queue_lock);
}



void wd_transfers_queue_upload(wi_string_t *path, wi_file_offset_t size, wi_string_t *checksum) {
	wd_user_t			*user = wd_users_user_for_thread();
	wi_string_t			*realpath, *filechecksum;
	wd_transfer_t		*transfer;
	wi_file_offset_t	offset;
	struct stat			sb;
	
	realpath = wd_files_real_path(path);
	wi_string_resolve_aliases_in_path(realpath);
	
	if(wi_file_stat(realpath, &sb)) {
		wd_reply(521, WI_STR("File or Directory Exists"));

		return;
	}
	
	if(!wi_string_has_suffix(realpath, WI_STR(WD_TRANSFERS_PARTIAL_EXTENSION)))
		wi_string_append_string(realpath, WI_STR(WD_TRANSFERS_PARTIAL_EXTENSION));
	
	if(!wi_file_stat(realpath, &sb)) {
		offset = 0;
	} else {
		offset = sb.st_size;
		
		if(sb.st_size >= WD_FILES_CHECKSUM_SIZE) {
			filechecksum = wi_file_sha1(realpath, WD_FILES_CHECKSUM_SIZE);
			
			if(!wi_is_equal(filechecksum, checksum)) {
				wd_reply(522, WI_STR("Checksum Mismatch"));
				
				return;
			}
		}
	}
	
	transfer				= wi_autorelease(wd_transfer_init_upload_with_user(wd_transfer_alloc(), user));
	transfer->path			= wi_retain(path);
	transfer->realpath		= wi_retain(realpath);
	transfer->size			= size;
	transfer->offset		= offset;
	transfer->transferred	= offset;
	
	wi_lock_lock(wd_transfers_update_queue_lock);

	wi_array_wrlock(wd_transfers);
	wi_array_add_data(wd_transfers, transfer);
	wi_array_unlock(wd_transfers);
	
	wd_transfers_update_queue();

	wi_lock_unlock(wd_transfers_update_queue_lock);
}



void wd_transfers_remove_user(wd_user_t *user) {
	wd_transfer_t	*transfer;
	wi_boolean_t	update = false;
	wi_uinteger_t	i, count;

	wi_lock_lock(wd_transfers_update_queue_lock);
	wi_array_wrlock(wd_transfers);
	
	count = wi_array_count(wd_transfers);
	
	for(i = 0; i < count; i++) {
		transfer = WI_ARRAY(wd_transfers, i);

		if(transfer->state <= WD_TRANSFER_RUNNING && transfer->user == user) {
			if(transfer->state == WD_TRANSFER_RUNNING) {
				wi_array_unlock(wd_transfers);
				
				wd_transfer_set_state(transfer, WD_TRANSFER_STOP);
				
				wi_condition_lock_lock_when_condition(transfer->state_lock, WD_TRANSFER_STOPPED, 1.0);
				wi_condition_lock_unlock(transfer->state_lock);
				
				wi_array_wrlock(wd_transfers);
			
				count--;
				i--;
			}
			else if(transfer->state != WD_TRANSFER_STOP) {
				wi_array_remove_data_at_index(wd_transfers, i);
				
				count--;
				i--;
				update = true;
			}
		}
	}
	
	wi_array_unlock(wd_transfers);
	
	if(update)
		wd_transfers_update_queue();

	wi_lock_unlock(wd_transfers_update_queue_lock);
}



wd_transfer_t * wd_transfers_transfer_with_hash(wi_string_t *hash) {
	wd_transfer_t	*transfer, *value = NULL;
	wi_uinteger_t	i, count;

	wi_array_rdlock(wd_transfers);
	
	count = wi_array_count(wd_transfers);
	
	for(i = 0; i < count; i++) {
		transfer = WI_ARRAY(wd_transfers, i);
		
		if(wi_is_equal(transfer->hash, hash)) {
			value = wi_autorelease(wi_retain(transfer));

			break;          
		}
	}

	wi_array_unlock(wd_transfers);

	return value;
}



#pragma mark -

static void wd_transfers_update_queue(void) {
	wi_set_t			*users;
	wi_array_t			*sorted_users, *transfers_queue;
	wd_transfer_t		*transfer;
	wd_user_t			*user;
	wi_uinteger_t		position;
	wi_uinteger_t		i, count;
	wi_uinteger_t		total_downloads, total_uploads, user_downloads, user_uploads;
	wi_boolean_t		queue;
	
	wi_array_rdlock(wd_transfers);
	wi_lock_lock(wd_transfers_status_lock);
	
	total_downloads = wd_settings.totaldownloads;
	user_downloads = wd_settings.clientdownloads;
	total_uploads = wd_settings.totaluploads;
	user_uploads = wd_settings.clientuploads;

	users = wi_set_init(wi_set_alloc());
	count = wi_array_count(wd_transfers);
	
	for(i = 0; i < count; i++) {
		transfer = WI_ARRAY(wd_transfers, i);
		
		if(transfer->state == WD_TRANSFER_QUEUED) {
			wi_array_add_data(wd_user_transfers_queue(transfer->user), transfer);
			wi_set_add_data(users, transfer->user);
		}
	}
	
	count = wi_set_count(users);
	
	if(count > 0) {
		sorted_users = wi_set_all_data(users);
		wi_array_sort(sorted_users, wd_transfers_compare_user);
		
		position = 1;
		
		while(count > 0) {
			for(i = 0; i < count; i++) {
				user = WI_ARRAY(sorted_users, i);
				transfers_queue = wd_user_transfers_queue(user);
				transfer = WI_ARRAY(transfers_queue, 0);
				
				if(transfer->type == WD_TRANSFER_DOWNLOAD) {
					queue = (total_downloads > 0 || user_downloads > 0) &&
							(wd_transfers_active_downloads >= total_downloads ||
							 wd_user_downloads(transfer->user) >= user_downloads);
				} else {
					queue = (total_uploads > 0 || user_uploads > 0) &&
							(wd_transfers_active_uploads >= total_uploads ||
							 wd_user_uploads(transfer->user) >= user_uploads);
				}
				
				if(queue) {
					if(transfer->queue != position) {
						transfer->queue = position;
						
						wd_user_lock_socket(transfer->user);
						wd_sreply(wd_user_socket(transfer->user), 401, WI_STR("%#@%c%u"),
								  transfer->path,	WD_FIELD_SEPARATOR,
								  transfer->queue);
						wd_user_unlock_socket(transfer->user);
					}

					position++;
				} else {
					transfer->queue = 0;
					transfer->state = WD_TRANSFER_WAITING;
					transfer->waiting_time = wi_time_interval();
					
					wd_user_lock_socket(transfer->user);
					wd_sreply(wd_user_socket(transfer->user), 400, WI_STR("%#@%c%llu%c%#@"),
							  transfer->path,		WD_FIELD_SEPARATOR,
							  transfer->offset,		WD_FIELD_SEPARATOR,
							  transfer->hash);
					wd_user_unlock_socket(transfer->user);
					
					if(transfer->type == WD_TRANSFER_DOWNLOAD) {
						wd_transfers_active_downloads++;
						wd_user_increase_downloads(transfer->user);
					} else {
						wd_transfers_active_uploads++;
						wd_user_increase_uploads(transfer->user);
					}
				}
				
				wi_array_remove_data_at_index(transfers_queue, 0);
				
				if(wi_array_count(transfers_queue) == 0) {
					wi_array_remove_data_at_index(sorted_users, i);
				
					count--;
					i--;
				}
			}
		}
	}
	
	wi_release(users);
	
	wi_lock_unlock(wd_transfers_status_lock);
	wi_array_unlock(wd_transfers);
}



static wi_integer_t wd_transfers_compare_user(wi_runtime_instance_t *instance1, wi_runtime_instance_t *instance2) {
	wd_user_t			*user1 = instance1;
	wd_user_t			*user2 = instance2;
	wd_transfer_t		*transfer1, *transfer2;
	
	transfer1 = WI_ARRAY(wd_user_transfers_queue(user1), 0);
	transfer2 = WI_ARRAY(wd_user_transfers_queue(user2), 0);
	
	if(transfer1->queue_time > transfer2->queue_time)
		return 1;
	else if(transfer2->queue_time > transfer1->queue_time)
		return -1;
	
	return 0;
}



#pragma mark -

wd_transfer_t * wd_transfer_alloc(void) {
	return wi_runtime_create_instance(wd_transfer_runtime_id, sizeof(wd_transfer_t));
}



static wd_transfer_t * wd_transfer_init_with_user(wd_transfer_t *transfer, wd_user_t *user) {
	transfer->state			= WD_TRANSFER_QUEUED;
	transfer->queue_time	= wi_time_interval();
	transfer->user			= wi_retain(user);
	transfer->hash			= wi_retain(wi_data_sha1(wi_data_with_random_bytes(1024)));
	transfer->state_lock	= wi_condition_lock_init_with_condition(wi_condition_lock_alloc(), transfer->state);

	return transfer;
}



static wd_transfer_t * wd_transfer_init_download_with_user(wd_transfer_t *transfer, wd_user_t *user) {
	transfer				= wd_transfer_init_with_user(transfer, user);
	transfer->type			= WD_TRANSFER_DOWNLOAD;
	
	return transfer;
}



static wd_transfer_t * wd_transfer_init_upload_with_user(wd_transfer_t *transfer, wd_user_t *user) {
	transfer				= wd_transfer_init_with_user(transfer, user);
	transfer->type			= WD_TRANSFER_UPLOAD;
	
	return transfer;
}



static void wd_transfer_dealloc(wi_runtime_instance_t *instance) {
	wd_transfer_t		*transfer = instance;

	wi_release(transfer->socket);
	wi_release(transfer->user);
	wi_release(transfer->hash);

	wi_release(transfer->path);
	wi_release(transfer->realpath);

	wi_release(transfer->state_lock);
}



static wi_string_t * wd_transfer_description(wi_runtime_instance_t *instance) {
	wd_transfer_t		*transfer = instance;
	
	return wi_string_with_format(WI_STR("<%@ %p>{path = %@, user = %@}"),
		wi_runtime_class_name(transfer),
		transfer,
		transfer->path,
		transfer->user);
}



#pragma mark -

static void wd_transfer_set_state(wd_transfer_t *transfer, wd_transfer_state_t state) {
	wi_condition_lock_lock(transfer->state_lock);
	transfer->state = state;
	wi_condition_lock_unlock_with_condition(transfer->state_lock, transfer->state);
}



static inline void wd_transfer_limit_download_speed(wd_transfer_t *transfer, wd_account_t *account, ssize_t bytes, wi_time_interval_t now, wi_time_interval_t then) {
	wi_uinteger_t	limit, totallimit;
	
	if(account->download_speed > 0 || wd_settings.totaldownloadspeed > 0) {
		totallimit = (wd_settings.totaldownloadspeed > 0)
			? (float) wd_settings.totaldownloadspeed / (float) wd_current_downloads
			: 0;
		
		if(totallimit > 0 && account->download_speed > 0)
			limit = WI_MIN(totallimit, account->download_speed);
		else if(totallimit > 0)
			limit = totallimit;
		else
			limit = account->download_speed;

		if(limit > 0) {
			while(transfer->speed > limit) {
				usleep(10000);
				now += 0.01;
				
				transfer->speed = bytes / (now - then);
			}
		}
	}
}



static inline void wd_transfer_limit_upload_speed(wd_transfer_t *transfer, wd_account_t *account, ssize_t bytes, wi_time_interval_t now, wi_time_interval_t then) {
	wi_uinteger_t	limit, totallimit;
	
	if(account->upload_speed > 0 || wd_settings.totaluploadspeed > 0) {
		totallimit = (wd_settings.totaluploadspeed > 0)
			? (float) wd_settings.totaluploadspeed / (float) wd_current_uploads
			: 0;
		
		if(totallimit > 0 && account->upload_speed > 0)
			limit = WI_MIN(totallimit, account->upload_speed);
		else if(totallimit > 0)
			limit = totallimit;
		else
			limit = account->upload_speed;

		if(limit > 0) {
			while(transfer->speed > limit) {
				usleep(10000);
				now += 0.01;
				
				transfer->speed = bytes / (now - then);
			}
		}
	}
}



#pragma mark -

void wd_transfer_thread(wi_runtime_instance_t *argument) {
	wi_pool_t			*pool;
	wd_transfer_t		*transfer = argument;
	
	pool = wi_pool_init(wi_pool_alloc());
	
	wd_transfer_set_state(transfer, WD_TRANSFER_RUNNING);

	if(transfer->type == WD_TRANSFER_DOWNLOAD)
		wd_transfer_download(transfer);
	else
		wd_transfer_upload(transfer);

	wi_lock_lock(wd_transfers_update_queue_lock);

	wi_array_wrlock(wd_transfers);
	wi_array_remove_data(wd_transfers, transfer);
	wi_array_unlock(wd_transfers);

	wd_transfers_update_queue();

	wi_lock_unlock(wd_transfers_update_queue_lock);
	
	wi_release(pool);
}



static void wd_transfer_download(wd_transfer_t *transfer) {
	wi_pool_t				*pool;
	wd_account_t			*account;
	SSL						*ssl;
	char					buffer[WD_TRANSFER_BUFFER_SIZE];
	const char				*file;
	wi_time_interval_t		interval, speedinterval, statusinterval, accountinterval;
	wi_socket_state_t		state;
	ssize_t					bytes, speedbytes, statsbytes;
	wi_uinteger_t			i = 0;
	int						fd, sd, result, error, line;

	pool = wi_pool_init(wi_pool_alloc());
	
	/* start download */
	wi_log_l(WI_STR("Sending \"%@\" to %@"),
		transfer->path,
		wd_user_identifier(transfer->user));

	sd = wi_socket_descriptor(transfer->socket);
	ssl = wi_socket_ssl(transfer->socket);
	interval = speedinterval = statusinterval = accountinterval = wi_time_interval();
	speedbytes = statsbytes = 0;
	account = wd_user_account(transfer->user);

	/* open local file */
	fd = open(wi_string_cstring(transfer->realpath), O_RDONLY, 0);

	if(fd < 0) {
		wi_log_err(WI_STR("Could not open %@: %s"),
			transfer->realpath, strerror(errno));

		goto end;
	}

	lseek(fd, transfer->offset, SEEK_SET);

	/* update status */
	wi_lock_lock(wd_status_lock);
	wd_current_downloads++;
	wd_total_downloads++;
	wd_write_status(true);
	wi_lock_unlock(wd_status_lock);

	while(transfer->state == WD_TRANSFER_RUNNING) {
		/* read data */
		bytes = read(fd, buffer, sizeof(buffer));

		if(bytes <= 0) {
			if(bytes < 0) {
				wi_log_err(WI_STR("Could not read download from %@: %m"),
					transfer->realpath, strerror(errno));
			}
			
			break;
		}

		/* wait to write */
		do {
			state = wi_socket_wait_descriptor(sd, 0.1, false, true);
		} while(state == WI_SOCKET_TIMEOUT && transfer->state == WD_TRANSFER_RUNNING);
		
		if(transfer->state != WD_TRANSFER_RUNNING)
			break;

		if(state == WI_SOCKET_ERROR) {
			wi_log_err(WI_STR("Could not wait for download to %@: %m"),
				wd_user_ip(transfer->user));

			break;
		}

		/* write data */
		result = SSL_write(ssl, buffer, bytes);
		
		if(result <= 0) {
			if(result < 0) {
				if(ERR_peek_error() == 0) {
					error = SSL_get_error(ssl, result);
					
					if((error != SSL_ERROR_SYSCALL && error != SSL_ERROR_ZERO_RETURN) ||
					   (error == SSL_ERROR_SYSCALL && errno != EPIPE)) {
						wi_log_err(WI_STR("Could not write download to %@: %s"),
							wd_user_ip(transfer->user), strerror(errno));
					}
				} else {
					error = ERR_get_error_line(&file, &line);

					wi_log_err(WI_STR("Could not write download to %@: %s:%u: %s: %s (%u)"),
						wd_user_ip(transfer->user),
						file,
						line,
						ERR_func_error_string(error),
						ERR_reason_error_string(error),
						ERR_GET_REASON(error));
				}
			}
			
			break;
		}

		/* update counters */
		interval = wi_time_interval();
		transfer->transferred += bytes;
		speedbytes += bytes;
		statsbytes += bytes;

		/* update speed */
		transfer->speed = speedbytes / (interval - speedinterval);

		wd_transfer_limit_download_speed(transfer, account, speedbytes, interval, speedinterval);
		
		if(interval - speedinterval > 30.0) {
			speedbytes = 0;
			speedinterval = interval;
		}

		/* update status */
		if(interval - statusinterval > wd_current_downloads) {
			wi_lock_lock(wd_status_lock);
			wd_downloads_traffic += statsbytes;
			wd_write_status(false);
			wi_lock_unlock(wd_status_lock);

			statsbytes = 0;
			statusinterval = interval;
		}
		
		/* update account */
		if(interval - accountinterval > 15.0) {
			account = wd_user_account(transfer->user);
			accountinterval = interval;
		}
		
		if(++i % 100 == 0)
			wi_pool_drain(pool);
	}

	wi_log_l(WI_STR("Sent %llu/%llu bytes of \"%@\" to %@"),
		transfer->transferred - transfer->offset,
		transfer->size,
		transfer->path,
		wd_user_identifier(transfer->user));
	
	/* update status */
	wd_transfer_set_state(transfer, WD_TRANSFER_STOPPED);

	wi_lock_lock(wd_transfers_status_lock);
	wd_transfers_active_downloads--;
	wd_user_decrease_downloads(transfer->user);
	wi_lock_unlock(wd_transfers_status_lock);
	
	wi_lock_lock(wd_status_lock);
	wd_current_downloads--;
	wd_downloads_traffic += statsbytes;
	wd_write_status(true);
	wi_lock_unlock(wd_status_lock);

end:
	wi_socket_close(transfer->socket);

	if(fd >= 0)
		close(fd);
	
	wi_release(pool);
}



static void wd_transfer_upload(wd_transfer_t *transfer) {
	wi_pool_t				*pool;
	wi_string_t				*path;
	wd_account_t			*account;
	SSL						*ssl;
	char					buffer[WD_TRANSFER_BUFFER_SIZE];
	const char				*file;
	wi_time_interval_t		interval, speedinterval, statusinterval, accountinterval;
	wi_socket_state_t		state;
	ssize_t					result, speedbytes, statsbytes;
	wi_uinteger_t			i = 0;
	int						sd, fd, bytes, error, line;

	pool = wi_pool_init(wi_pool_alloc());

	/* start upload */
	wi_log_l(WI_STR("Receiving \"%@\" from %@"),
		transfer->path,
		wd_user_identifier(transfer->user));
	
	sd = wi_socket_descriptor(transfer->socket);
	ssl = wi_socket_ssl(transfer->socket);
	interval = speedinterval = statusinterval = accountinterval = wi_time_interval();
	speedbytes = statsbytes = 0;
	account = wd_user_account(transfer->user);

	/* open the file */
	fd = open(wi_string_cstring(transfer->realpath), O_WRONLY | O_APPEND | O_CREAT, 0666);

	if(fd < 0) {
		wi_log_err(WI_STR("Could not open %@: %s"),
			transfer->realpath, strerror(errno));

		goto end;
	}

	/* update status */
	wi_lock_lock(wd_status_lock);
	wd_current_uploads++;
	wd_total_uploads++;
	wd_write_status(true);
	wi_lock_unlock(wd_status_lock);

	while(transfer->state == WD_TRANSFER_RUNNING) {
		/* wait to read */
		do {
			state = wi_socket_wait_descriptor(sd, 0.1, true, false);
		} while(state == WI_SOCKET_TIMEOUT && transfer->state == WD_TRANSFER_RUNNING);
		
		if(transfer->state != WD_TRANSFER_RUNNING)
			break;

		if(state == WI_SOCKET_ERROR) {
			wi_log_err(WI_STR("Could not wait for upload from %@: %m"),
				wd_user_ip(transfer->user));

			break;
		}

		/* read data */
		bytes = SSL_read(ssl, buffer, sizeof(buffer));

		if(bytes <= 0) {
			if(bytes < 0) {
				if(ERR_peek_error() == 0) {
					wi_log_err(WI_STR("Could not read upload from %@: %s"),
						wd_user_ip(transfer->user), strerror(errno));
				} else {
					error = ERR_get_error_line(&file, &line);

					wi_log_err(WI_STR("Could not read upload from %@: %s:%u: %s: %s (%u)"),
						wd_user_ip(transfer->user),
						file,
						line,
						ERR_func_error_string(error),
						ERR_reason_error_string(error),
						ERR_GET_REASON(error));
				}
			}
			
			break;
		}

		/* write data */
		result = write(fd, buffer, bytes);
		
		if(result <= 0) {
			if(result < 0) {
				wi_log_err(WI_STR("Could not write upload to %@: %s"),
					transfer->realpath, strerror(errno));
			}
			
			break;
		}

		/* update counters */
		interval = wi_time_interval();
		transfer->transferred += bytes;
		speedbytes += bytes;
		statsbytes += bytes;

		/* update speed */
		transfer->speed = speedbytes / (interval - speedinterval);

		wd_transfer_limit_upload_speed(transfer, account, speedbytes, interval, speedinterval);
		
		if(interval - speedinterval > 30.0) {
			speedbytes = 0;
			speedinterval = interval;
		}

		/* update status */
		if(interval - statusinterval > wd_current_uploads) {
			wi_lock_lock(wd_status_lock);
			wd_uploads_traffic += statsbytes;
			wd_write_status(false);
			wi_lock_unlock(wd_status_lock);

			statsbytes = 0;
			statusinterval = interval;
		}
		
		/* update account */
		if(interval - accountinterval > 15.0) {
			account = wd_user_account(transfer->user);
			accountinterval = interval;
		}
		
		if(++i % 100 == 0)
			wi_pool_drain(pool);
	}

	wi_log_l(WI_STR("Received %llu/%llu bytes of \"%@\" from %@"),
		transfer->transferred - transfer->offset,
		transfer->size,
		transfer->path,
		wd_user_identifier(transfer->user));

	/* update status */
	wd_transfer_set_state(transfer, WD_TRANSFER_STOPPED);
	
	wi_lock_lock(wd_transfers_status_lock);
	wd_transfers_active_uploads--;
	wd_user_decrease_uploads(transfer->user);
	wi_lock_unlock(wd_transfers_status_lock);

	wi_lock_lock(wd_status_lock);
	wd_uploads_traffic += statsbytes;
	wd_current_uploads--;
	wd_write_status(true);
	wi_lock_unlock(wd_status_lock);

	if(transfer->transferred == transfer->size) {
		path = wi_string_by_deleting_path_extension(transfer->realpath);

		if(wi_file_rename(transfer->realpath, path)) {
			path = wi_string_by_appending_string(transfer->path, WI_STR(WD_TRANSFERS_PARTIAL_EXTENSION));

			wd_files_move_comment(path, transfer->path);
		} else {
			wi_log_warn(WI_STR("Could not move %@ to %@: %m"),
				transfer->realpath, path);
		}
	}

end:
	wi_socket_close(transfer->socket);

	if(fd >= 0)
		close(fd);
	
	wi_release(pool);
}
