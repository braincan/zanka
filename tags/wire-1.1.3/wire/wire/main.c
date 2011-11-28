/* $Id$ */

/*
 *  Copyright (c) 2004-2007 Axel Andersson
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <readline/readline.h>
#include <wired/wired.h>

#include "chats.h"
#include "client.h"
#include "commands.h"
#include "files.h"
#include "ignores.h"
#include "main.h"
#include "terminal.h"
#include "transfers.h"
#include "users.h"
#include "version.h"
#include "windows.h"

static void							wr_cleanup(void);
static void							wr_usage(void);
static void							wr_version(void);

static void							wr_signals_init(void);
static void							wr_sig_winch(int);
static void							wr_sig_int(int);
static void							wr_sig_crash(int);

static void							wr_wi_log_callback(wi_log_level_t, wi_string_t *);

static wi_integer_t					wr_runloop(wi_array_t *, wi_time_interval_t);
static wi_integer_t					wr_runloop_stdin_callback(wi_socket_t *);


static wi_array_t					*wr_runloop_sockets;

volatile sig_atomic_t				wr_running = 1;

wi_boolean_t						wr_debug;
wi_date_t							*wr_start_date;


int main(int argc, const char **argv) {
	wi_pool_t			*pool;
	wi_string_t			*homepath, *wirepath, *path, *component;
	wi_file_t			*file;	
	int					ch;

	/* init libwired */
	wi_initialize();
	wi_load(argc, argv);
	
	pool = wi_pool_init(wi_pool_alloc());
	wi_log_callback = wr_wi_log_callback;
	
	/* init core systems */
	wr_version_init();
	wr_start_date = wi_date_init(wi_date_alloc());
	
	/* parse command line switches */
	while((ch = getopt(argc, (char * const *) argv, "DhVv")) != -1) {
		switch(ch) {
			case 'D':
				wr_debug = true;

				wi_log_level = WI_LOG_DEBUG;
				wi_log_file = true;
				wi_log_path = WI_STR("wire.out");
				wi_log_callback = NULL;
				break;

			case 'V':
			case 'v':
				wr_version();
				break;

			case '?':
			case 'h':
			default:
				wr_usage();
				break;
		}
	}

	argc -= optind;
	argv += optind;

	/* open log */
	wi_log_open();
	
	/* create ~/.wire */
	homepath = wi_user_home();
	wirepath = wi_string_by_appending_path_component(homepath, WI_STR(WR_WIRE_PATH));
	wi_fs_create_directory(wirepath, 0700);
	
	/* init subsystems */
	wr_signals_init();
	wr_terminal_init();
	wr_readline_init();
	wr_chats_init();
	wr_windows_init();
	wr_client_init();
	wr_runloop_init();
	wr_users_init();
	wr_ignores_init();
	wr_files_init();
	wr_transfers_init();
	wr_servers_init();
	
	/* open default settings */
	path = wi_string_by_appending_path_component(homepath, WI_STR(WR_WIRE_CONFIG_PATH));
	file = wi_file_for_reading(path);

	if(file)
		wr_parse_file(file);
	else
		wr_printf_prefix(WI_STR("%@: %m"), path);

	/* read specified bookmark */
	if(*argv) {
		component	= wi_string_with_cstring(*argv);
		path		= wi_string_by_appending_path_component(wirepath, component);
		file		= wi_file_for_reading(path);

		if(file)
			wr_parse_file(file);
		else
			wr_printf_prefix(WI_STR("%@: %m"), path);
	}
	
	/* clean up pool after startup */
	wi_pool_drain(pool);
	
	/* enter event loop */
	wr_runloop_run();

	/* clean up */
	wr_cleanup();
	wi_release(pool);
	
	return 0;
}



static void wr_cleanup(void) {
	wr_terminal_close();
	wr_readline_close();
}



static void wr_usage(void) {
	fprintf(stderr,
"Usage: wire [-Dhv] [bookmark]\n\
\n\
Options:\n\
    -D             enable debug mode\n\
    -h             display this message\n\
    -v             display version information\n\
\n\
If specified, ~/.wire/<bookmark> is loaded on startup.\n\
\n\
By Axel Andersson <%s>\n", WR_BUGREPORT);

	exit(2);
}



static void wr_version(void) {
	fprintf(stderr, "Wire %s, protocol %s, %s, readline %s\n",
		wi_string_cstring(wr_version_string),
		wi_string_cstring(wr_protocol_version_string),
		SSLeay_version(SSLEAY_VERSION),
		rl_library_version);

	exit(2);
}



#pragma mark -

static void wr_signals_init(void) {
	signal(SIGPIPE, SIG_IGN);
	signal(SIGWINCH, wr_sig_winch);
	signal(SIGINT, wr_sig_int);
	signal(SIGTERM, wr_sig_int);
	signal(SIGQUIT, wr_sig_int);
	signal(SIGILL, wr_sig_crash);
	signal(SIGBUS, wr_sig_crash);
	signal(SIGSEGV, wr_sig_crash);
	signal(SIGABRT, wr_sig_crash);
}



static void wr_sig_winch(int sigraised) {
	wr_terminal_resize();
}



static void wr_sig_int(int sigraised) {
	wr_running = 0;
}



static void wr_sig_crash(int sigraised) {
	wr_cleanup();
	
	if(signal(sigraised, SIG_DFL) != SIG_ERR)
		raise(sigraised);
}



#pragma mark -

static void wr_wi_log_callback(wi_log_level_t level, wi_string_t *string) {
	wr_printf_prefix(WI_STR("%@"), string);
}



#pragma mark -

char * wr_readline_bookmark_generator(const char *text, int state) {
	static wi_array_t		*bookmarks;
	static wi_uinteger_t	index, count, length;
	wi_string_t				*path, *string, *bookmark;
	char					*match = NULL;
	
	if(state == 0) {
		wi_release(bookmarks);

		path = wi_user_home();
		wi_string_append_path_component(path, WI_STR(WR_WIRE_PATH));
		
		bookmarks = wi_retain(wi_fs_directory_contents_at_path(path));
		index = 0;
		count = wi_array_count(bookmarks);
		length = strlen(text);
	}
	
	string = wi_string_with_cstring(text);

	while(index < count) {
		bookmark = wi_array_data_at_index(bookmarks, index);
		index++;

		if(wi_string_index_of_string(bookmark, string, WI_STRING_SMART_CASE_INSENSITIVE) == 0) {
			match = strdup(wi_string_cstring(bookmark));
			
			break;
		}
	}

	return match;
}



#pragma mark -

void wr_runloop_init(void) {
	wr_runloop_sockets = wi_array_init(wi_array_alloc());
}



void wr_runloop_add_socket(wi_socket_t *socket, wr_runloop_callback_func_t *callback) {
	wi_socket_set_data(socket, callback);
	wi_array_add_data(wr_runloop_sockets, socket);
}



void wr_runloop_remove_socket(wi_socket_t *socket) {
	wi_array_remove_data(wr_runloop_sockets, socket);
}



void wr_runloop_run(void) {
	wi_pool_t			*pool;
	wi_socket_t			*socket;
	wi_time_interval_t	interval, ping_interval;
	wi_uinteger_t		i = 0;
	wi_integer_t		result;
	
	pool = wi_pool_init(wi_pool_alloc());

	socket = wi_socket_init_with_descriptor(wi_socket_alloc(), STDIN_FILENO);
	wi_socket_set_direction(socket, WI_SOCKET_READ);
	wr_runloop_add_socket(socket, &wr_runloop_stdin_callback);
	wi_release(socket);
	
	ping_interval = wi_time_interval();
	
	while(wr_running) {
		result = wr_runloop(wr_runloop_sockets, 30.0);
		
		if(result < 0 && wr_connected) {
			interval = wi_time_interval();
			
			if(interval - ping_interval > 60.0) {
				wr_send_command(WI_STR("PING"));
				
				ping_interval = interval;
			}
			
			wi_pool_drain(pool);
		}
		
		if(++i % 100 == 0)
			wi_pool_drain(pool);
	}
	
	wi_release(pool);
}



void wr_runloop_run_for_socket(wi_socket_t *socket, wi_time_interval_t timeout, wi_uinteger_t message) {
	wi_array_t		*array;
	wi_integer_t	result;
	
	array = wi_array_init_with_data(wi_array_alloc(), socket, NULL);
	
	while(wr_running) {
		result = wr_runloop(array, timeout);
		
		if(result < 0 || (wi_uinteger_t) result == message || (result >= 500 && result < 600))
			break;
	}
	
	wi_release(array);
}



#pragma mark -

static wi_integer_t wr_runloop(wi_array_t *array, wi_time_interval_t timeout) {
	wi_socket_t					*socket;
	wr_runloop_callback_func_t	*callback;
	
	socket = wi_socket_wait_multiple(array, timeout);
	
	if(socket) {
		callback = wi_socket_data(socket);
		
		return (*callback)(socket);
	}
	
	return -1;
}



static wi_integer_t wr_runloop_stdin_callback(wi_socket_t *socket) {
	wr_readline_read();
	
	return 1;
}
