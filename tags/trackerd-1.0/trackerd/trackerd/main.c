/* $Id$ */

/*
 *  Copyright (c) 2004-2006 Axel Andersson
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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>
#include <openssl/ssl.h>
#include <wired/wired.h>

#include "clients.h"
#include "main.h"
#include "servers.h"
#include "settings.h"
#include "tracker.h"
#include "version.h"

static void						wt_cleanup(void);
static void						wt_usage(void);
static void						wt_version(void);

static void						wt_write_pid(void);
static void						wt_delete_pid(void);
static void						wt_delete_status(void);

static void						wt_init_signals(void);
static void						wt_block_signals(void);
static int						wt_wait_signals(void);
static void						wt_signal_thread(wi_runtime_instance_t *);
static void						wt_signal_crash(int);


static wi_boolean_t				wt_daemonize = true;

wi_boolean_t					wt_running = true;

wi_address_family_t				wt_address_family = WI_ADDRESS_NULL;

wi_lock_t						*wt_status_lock;
wi_date_t						*wt_start_date;
uint32_t						wt_current_servers, wt_total_clients;
uint32_t						wt_current_users;
uint32_t						wt_current_files;
uint64_t						wt_current_size;


int main(int argc, const char **argv) {
	wi_pool_t			*pool;
	wi_string_t			*string;
	int					ch, facility;
	wi_boolean_t		no_chroot, test_config;

	/* init libwired */
	wi_initialize();
	wi_load(argc, argv);
	
	pool					= wi_pool_init(wi_pool_alloc());
	wi_log_startup			= true;
	wi_log_syslog			= true;
	wi_log_syslog_facility	= LOG_DAEMON;

	/* init core systems */
	wt_init_version();
	wt_status_lock			= wi_lock_init(wi_lock_alloc());
	wt_start_date			= wi_date_init(wi_date_alloc());
	
	/* set defaults */
	wi_root_path			= wi_string_init_with_cstring(wi_string_alloc(), WT_ROOT);
	wi_settings_config_path	= wi_string_init_with_cstring(wi_string_alloc(), WT_CONFIG_PATH);
	no_chroot				= false;
	test_config				= false;

	/* parse command line switches */
	while((ch = getopt(argc, (char * const *) argv, "46Dd:f:hi:L:ls:tuVv")) != -1) {
		switch(ch) {
			case '4':
				wt_address_family = WI_ADDRESS_IPV4;
				break;

			case '6':
				wt_address_family = WI_ADDRESS_IPV6;
				break;

			case 'D':
				wt_daemonize = false;
				wi_log_stderr = true;
				break;

			case 'd':
				wi_release(wi_root_path);
				wi_root_path = wi_string_init_with_cstring(wi_string_alloc(), optarg);
				break;

			case 'f':
				wi_release(wi_settings_config_path);
				wi_settings_config_path = wi_string_init_with_cstring(wi_string_alloc(), optarg);
				break;

			case 'i':
				wi_log_limit = wi_string_uint32(wi_string_with_cstring(optarg));
				break;

			case 'L':
				wi_log_syslog = false;
				wi_log_file = true;

				wi_release(wi_log_path);
				wi_log_path = wi_string_init_with_cstring(wi_string_alloc(), optarg);
				break;

			case 'l':
				wi_log_level++;
				break;

			case 's':
				string = wi_string_with_cstring(optarg);
				facility = wi_log_syslog_facility_with_name(string);
				
				if(facility < 0) {
					wi_log_err(WI_STR("Could not find syslog facility \"%@\": %m"),
						string);
				}
				
				wi_log_syslog_facility = facility;
				break;

			case 't':
				test_config = true;
				break;

			case 'u':
				no_chroot = true;
				break;

			case 'V':
			case 'v':
				wt_version();
				break;

			case '?':
			case 'h':
			default:
				wt_usage();
				break;
		}
	}
	
	/* open log */
	wi_log_open();

	/* init subsystems */
	wt_init_ssl();
	wt_init_clients();
	wt_init_servers();

	/* read the config file */
	wt_settings_chroot = !no_chroot;
	wt_init_settings();

	if(!wt_read_config())
		exit(1);

	/* change root directory */
	if(!no_chroot) {
		if(!wi_change_root())
			wi_log_err(WI_STR("Could not change root to %@: %m"), wi_root_path);
	}

	/* apply config */
	wt_apply_config();

	if(test_config) {
		printf("Config OK\n");

		exit(0);
	}

	/* dump command line */
	if(wi_log_level >= WI_LOG_DEBUG) {
		wi_log_debug(WI_STR("Started as %@ %@"),
			wi_process_path(wi_process()),
			wi_array_components_joined_by_string(wi_process_arguments(wi_process()), WI_STR(" ")));
	}

	/* init tracker */
	wi_log_info(WI_STR("Starting Wired Tracker version %@"), wt_version_string);
	wt_init_tracker();

	/* detach (don't chdir, don't close i/o channels) */
	if(wt_daemonize) {
		if(!wi_daemon())
			wi_log_err(WI_STR("Could not become a daemon: %m"));
	}

	/* switch user/group */
	wi_switch_user(wt_settings.user, wt_settings.group);
		
	/* create tracker threads after privilege drop */
	wt_init_signals();
	wt_block_signals();
	wt_schedule_servers();
	wt_fork_tracker();
	wt_write_pid();
	wt_write_status(true);
	wi_log_startup = false;
	
	wi_release(pool);
	pool = wi_pool_init(wi_pool_alloc());
	
	/* enter the signal handling thread in the main thread */
	wt_signal_thread(NULL);

	/* dropped out */
	wt_cleanup();
	wi_log_close();
	wi_release(pool);

	return 0;
}



static void wt_cleanup(void) {
	wt_delete_pid();
	wt_delete_status();
}



static void wt_usage(void) {
	fprintf(stderr,
"Usage: trackerd [-Dllhtuv] [-d path] [-f file] [-i lines] [-L file] [-s facility]\n\
\n\
Options:\n\
    -4             listen on IPv4 addresses only\n\
    -6             listen on IPv6 addresses only\n\
    -D             do not daemonize\n\
    -d path        set the server root path\n\
    -f file        set the config file to load\n\
    -h             display this message\n\
    -i lines       set limit on number of lines for -L\n\
    -L             set alternate file for log output\n\
    -l             increase log level (can be used twice)\n\
    -s facility    set the syslog(3) facility\n\
    -t             run syntax test on config\n\
    -u             do not chroot(2) to root path\n\
    -v             display version information\n\
\n\
By Axel Andersson <%s>\n", WT_BUGREPORT);

	exit(2);
}



static void wt_version(void) {
	fprintf(stderr, "Wired Tracker %s, protocol %s, %s\n",
		wi_string_cstring(wt_version_string),
		wi_string_cstring(wt_protocol_version_string),
		SSLeay_version(SSLEAY_VERSION));

	exit(2);
}



#pragma mark -

static void wt_write_pid(void) {
	wi_string_t		*string;

	if(wt_settings.pid) {
		string = wi_string_with_format(WI_STR("%d\n"), getpid());

		if(!wi_string_write_to_file(string, wt_settings.pid))
			wi_log_warn(WI_STR("Could not write to %@: %m"), wt_settings.pid);
	}
}



static void wt_delete_pid(void) {
	if(wt_settings.pid) {
		if(!wi_file_delete(wt_settings.pid))
			wi_log_warn(WI_STR("Could not delete %@: %m"), wt_settings.pid);
	}
}



void wt_write_status(wi_boolean_t force) {
	static wi_time_interval_t	update;
	wi_string_t					*string;
	wi_time_interval_t			interval;

	interval = wi_time_interval();

	if(!force && interval - update < 1.0)
		return;

	update = interval;

	wi_process_set_name(wi_process(), wi_string_with_format(WI_STR("%u %@"),
		wt_current_servers,
		wt_current_servers == 1
			? WI_STR("server")
			: WI_STR("servers")));

	if(wt_settings.status) {
		string = wi_string_with_format(WI_STR("%.0f %u %u %u %u %llu\n"),
									   wi_date_time_interval(wt_start_date),
									   wt_current_servers,
									   wt_total_clients,
									   wt_current_users,
									   wt_current_files,
									   wt_current_size);
		
		if(!wi_string_write_to_file(string, wt_settings.status))
			wi_log_warn(WI_STR("Could not write to %@: %m"), wt_settings.status);
	}
}



static void wt_delete_status(void) {
	if(wt_settings.status) {
		if(!wi_file_delete(wt_settings.status))
			wi_log_warn(WI_STR("Could not delete %@: %m"), wt_settings.status);
	}
}



#pragma mark -

static void wt_init_signals(void) {
	signal(SIGPIPE, SIG_IGN);
	signal(SIGBUS, wt_signal_crash);
	signal(SIGSEGV, wt_signal_crash);
}



static void wt_block_signals(void) {
	wi_thread_block_signals(SIGHUP, SIGINT, SIGTERM, SIGQUIT, 0);
}



static int wt_wait_signals(void) {
	return wi_thread_wait_for_signals(SIGHUP, SIGINT, SIGTERM, SIGQUIT, 0);
}



static void wt_signal_thread(wi_runtime_instance_t *arg) {
	wi_pool_t		*pool;
	unsigned int	i = 0;
	int				signal;
	
	pool = wi_pool_init(wi_pool_alloc());

	while(wt_running) {
		if(!pool)
			pool = wi_pool_init(wi_pool_alloc());
		
		signal = wt_wait_signals();
		
		switch(signal) {
			case SIGHUP:
				wi_log_info(WI_STR("Signal HUP received, reloading configuration"));
				wt_read_config();
				wt_apply_config();
				break;

			case SIGINT:
				wi_log_info(WI_STR("Signal INT received, quitting"));
				wt_running = false;
				break;

			case SIGQUIT:
				wi_log_info(WI_STR("Signal QUIT received, quitting"));
				wt_running = false;
				break;

			case SIGTERM:
				wi_log_info(WI_STR("Signal TERM received, quitting"));
				wt_running = false;
				break;
		}
		
		if(++i % 10 == 0) {
			wi_release(pool);
			pool = NULL;
		}
	}
	
	wi_release(pool);
}



static void wt_signal_crash(int sigraised) {
	wt_cleanup();
	
	sleep(360);

	if(signal(sigraised, SIG_DFL) != SIG_ERR)
		raise(sigraised);
}
