/* $Id$ */

/*
 *  Copyright (c) 2003-2006 Axel Andersson
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

#if HAVE_SYSLOG_FACILITYNAMES
#define SYSLOG_NAMES
#endif

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <errno.h>

#include <wired/wi-compat.h>
#include <wired/wi-file.h>
#include <wired/wi-lock.h>
#include <wired/wi-log.h>
#include <wired/wi-process.h>
#include <wired/wi-string.h>

#include "wi-private.h"

#define _WI_LOG_DATE_SIZE		32


static void						_wi_log_vlog(int, wi_string_t *, va_list);
static void						_wi_log_date(char *);
static void						_wi_log_truncate(const char *);


wi_boolean_t					wi_log_stdout = false;
wi_boolean_t					wi_log_stderr = false;
wi_boolean_t					wi_log_startup = false;
wi_boolean_t					wi_log_tool = false;
wi_boolean_t					wi_log_syslog = false;
wi_boolean_t					wi_log_file = false;

wi_log_level_t					wi_log_level = WI_LOG_NORMAL;
int								wi_log_syslog_facility = LOG_DAEMON;
uint32_t						wi_log_limit = 0;

wi_string_t						*wi_log_path = NULL;

wi_log_callback_func_t			*wi_log_callback = NULL;

static int						_wi_log_lines;

static wi_lock_t				*_wi_log_lock;


void wi_log_register(void) {
}



void wi_log_initialize(void) {
	_wi_log_lock = wi_lock_init(wi_lock_alloc());
}



#pragma mark -

void wi_log_open(void) {
	wi_string_t		*name;
	
	if(wi_log_syslog) {
		name = wi_process_name(wi_process());
		
		openlog(wi_string_cstring(name), LOG_PID | LOG_NDELAY, wi_log_syslog_facility);
	}
}



void wi_log_close(void) {
	if(wi_log_syslog)
		closelog();
}



#pragma mark -

int wi_log_syslog_facility_with_name(wi_string_t *name) {
#if HAVE_SYSLOG_FACILITYNAMES
	const char		*cstring;
	int				i;
	
	cstring = wi_string_cstring(name);
	
	for(i = 0; facilitynames[i].c_name != NULL; i++) {
		if(strcmp(cstring, facilitynames[i].c_name) == 0)
			break;
	}

	if(!facilitynames[i].c_name) {
		wi_error_set_lib_error(WI_ERROR_LOG_NOSUCHFACILITY);
		
		return -1;
	}

	return facilitynames[i].c_val;
#else
	wi_error_set_errno(ENOTSUP);
	
	return -1;
#endif
}



#pragma mark -

static void _wi_log_vlog(int priority, wi_string_t *fmt, va_list ap) {
	wi_string_t		*string;
	FILE			*fp;
	const char		*cstring, *name, *path;
	char			date[_WI_LOG_DATE_SIZE];
	
	string = wi_string_init_with_format_and_arguments(wi_string_alloc(), fmt, ap);
	cstring = wi_string_cstring(string);

	wi_lock_lock(_wi_log_lock);
	
	name = wi_string_cstring(wi_process_name(wi_process()));
	
	_wi_log_date(date);

	if(wi_log_stdout || wi_log_stderr)
		fprintf(wi_log_stdout ? stdout : stderr, "%s %s[%d]: %s\n", date, name, getpid(), cstring);
	else if(wi_log_startup && priority < LOG_INFO)
		fprintf(stderr, "%s: %s\n", name, cstring);
	else if(wi_log_tool)
		fprintf((priority < LOG_INFO) ? stderr : stdout, "%s: %s\n", name, cstring);

	if(wi_log_syslog)
		syslog(priority, "%s", cstring);

	if(wi_log_file && wi_log_path) {
		path = wi_string_cstring(wi_full_path(wi_log_path));

		fp = fopen(path, "a");

		if(fp) {
			fprintf(fp, "%s %s[%d]: %s\n", date, name, getpid(), cstring);
			fclose(fp);
			
			if(_wi_log_lines > 0 && wi_log_limit > 0) {
				if(_wi_log_lines % (int) ((float) wi_log_limit / 10.0f) == 0) {
					_wi_log_truncate(path);
					
					_wi_log_lines = wi_log_limit;
				}
			}
			
			_wi_log_lines++;
		} else {
			fprintf(stderr, "%s: %s: %s\n", name, path, strerror(errno));
		}
	}

	if(wi_log_callback)
		(*wi_log_callback)(string);

	if(wi_log_startup && priority == LOG_ERR)
		exit(1);

	wi_lock_unlock(_wi_log_lock);
	
	wi_release(string);
}



static void _wi_log_date(char *string) {
    struct tm   tm;
	time_t      now;

	now = time(NULL);
	localtime_r(&now, &tm);
	strftime(string, _WI_LOG_DATE_SIZE, "%b %e %H:%M:%S", &tm);
}



static void _wi_log_truncate(const char *path) {
	wi_file_t		*file = NULL;
	FILE			*fp = NULL, *tmp = NULL;
	struct stat		sb;
	char			buffer[BUFSIZ];
	int				position, lines, n, ch = EOF;

	if(stat(path, &sb) < 0 || sb.st_size == 0)
		return;

	fp = fopen(path, "r");

	if(!fp)
		goto end;

	file = wi_file_init_temporary_file(wi_file_alloc());
	tmp = fdopen(wi_file_descriptor(file), "w+");

	if(!tmp)
		goto end;

	lines = wi_log_limit;

	for(position = sb.st_size - 2; lines > 0 && position >= 0; position--) {
		if(fseeko(fp, position, SEEK_SET) < 0)
			goto end;

		ch = fgetc(fp);

		if(ch == '\n')
			lines--;
		else if(ch == EOF && ferror(fp))
			goto end;
	}

	if(position < 0 && lines > 0 && ch != EOF)
		ungetc(ch, fp);

	while((n = fread(buffer, 1, sizeof(buffer), fp)))
		fwrite(buffer, 1, n, tmp);

	fp = freopen(path, "w", fp);

	if(!fp)
		goto end;

	rewind(tmp);

	while((n = fread(buffer, 1, sizeof(buffer), tmp)))
		fwrite(buffer, 1, n, fp);

end:
	if(fp)
		fclose(fp);

	wi_release(file);
}



#pragma mark -

void wi_log_debug(wi_string_t *fmt, ...) {
	va_list     ap;

	if(wi_log_level >= WI_LOG_DEBUG) {
		va_start(ap, fmt);
		_wi_log_vlog(LOG_DEBUG, fmt, ap);
		va_end(ap);
	}
}



void wi_log_info(wi_string_t *fmt, ...) {
	va_list     ap;

	va_start(ap, fmt);
	_wi_log_vlog(LOG_INFO, fmt, ap);
	va_end(ap);
}



void wi_log_warn(wi_string_t *fmt, ...) {
	va_list     ap;

	va_start(ap, fmt);
	_wi_log_vlog(LOG_WARNING, fmt, ap);
	va_end(ap);
}



void wi_log_err(wi_string_t *fmt, ...) {
	va_list     ap;

	va_start(ap, fmt);
	_wi_log_vlog(LOG_ERR, fmt, ap);
	va_end(ap);
}



void wi_log_l(wi_string_t *fmt, ...) {
	va_list     ap;

	if(wi_log_level >= WI_LOG_VERBOSE1) {
		va_start(ap, fmt);
		_wi_log_vlog(LOG_INFO, fmt, ap);
		va_end(ap);
	}
}



void wi_log_ll(wi_string_t *fmt, ...) {
	va_list     ap;

	if(wi_log_level >= WI_LOG_VERBOSE2) {
		va_start(ap, fmt);
		_wi_log_vlog(LOG_INFO, fmt, ap);
		va_end(ap);
	}
}
