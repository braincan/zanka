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

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <regex.h>

#ifdef WI_SSL
#include <openssl/err.h>
#include <openssl/ssl.h>
#endif

#include <wired/wi-assert.h>
#include <wired/wi-error.h>
#include <wired/wi-hash.h>
#include <wired/wi-runtime.h>
#include <wired/wi-string.h>
#include <wired/wi-thread.h>

#include "wi-private.h"

#define _WI_ERROR_THREAD_KEY			"_wi_error_t"

#define _WI_ERROR_ASSERT(error) \
	WI_ASSERT((error) != NULL, "no wi_error_t created for thread", 0)


struct _wi_error {
	wi_runtime_base_t					base;
	
	wi_string_t							*string;
	wi_error_domain_t					domain;
	int32_t								code;
};


static wi_error_t *						_wi_error_alloc(void);
static wi_error_t *						_wi_error_init(wi_error_t *);
static void								_wi_error_dealloc(wi_runtime_instance_t *);

static wi_error_t *						_wi_get_error(void);


static const char 						*_wi_error_strings[] = {
	/* WI_ERROR_NONE */
	"No error",

	/* WI_ERROR_ADDRESS_NOAVAILABLEADDRESSES */
	"No available addresses",
	
	/* WI_ERROR_HOST_NOAVAILABLEADDRESSES */
	"No available addresses",
	
	/* WI_ERROR_LOG_NOSUCHFACILITY */
	"No such syslog facility",
	
	/* WI_ERROR_REGEXP_NOSLASH */
	"Missing \"/\"",
	/* WI_ERROR_REGEXP_INVALIDOPTION */
	"Invalid option",
	
	/* WI_ERROR_SETTINGS_SYNTAXERROR */
	"Syntax error",
	/* WI_ERROR_SETTINGS_UNKNOWNSETTING */
	"Unknown setting name",
	/* WI_ERROR_SETTINGS_NOSUCHUSER */
	"User not found",
	/* WI_ERROR_SETTINGS_NOSUCHGROUP */
	"Group not found",
	/* WI_ERROR_SETTINGS_INVALIDPORT */
	"Port is not in 1-65535 range",
	/* WI_ERROR_SETTINGS_NOSUCHSERVICE */
	"Service not found",

	/* WI_ERROR_SOCKET_NOVALIDCIPHER */
	"No valid cipher",
	/* WI_ERROR_SOCKET_NOSSL */
	"Socket has no SSL support",
	/* WI_ERROR_SOCKET_EOF */
	"End of file",
	
	/* WI_ERROR_TERMCAP_NOSUCHENTRY */
	"No such entry in termcap database",
	/* WI_ERROR_TERMCAP_TERMINFONOTFOUND */
	"Termcap databse not found",

	/* WI_ERROR_THREADS_NOTSUPP */
	"Threads are not supported in this build",
};

static wi_runtime_id_t					_wi_error_runtime_id = WI_RUNTIME_ID_NULL;
static wi_runtime_class_t				_wi_error_runtime_class = {
	"wi_error_t",
	_wi_error_dealloc,
	NULL,
	NULL,
	NULL,
	NULL
};


void wi_error_register(void) {
	_wi_error_runtime_id = wi_runtime_register_class(&_wi_error_runtime_class);
}



void wi_error_initialize(void) {
#ifdef WI_SSL
	SSL_load_error_strings();
#endif
}



#pragma mark -

static wi_error_t * _wi_error_alloc(void) {
	return wi_runtime_create_instance(_wi_error_runtime_id, sizeof(wi_error_t));
}



static wi_error_t * _wi_error_init(wi_error_t *error) {
	return error;
}



static void _wi_error_dealloc(wi_runtime_instance_t *instance) {
	wi_error_t		*error = instance;
	
	wi_release(error->string);
}



#pragma mark -

wi_error_t * _wi_get_error(void) {
	return wi_hash_data_for_key(wi_thread_hash(), WI_STR(_WI_ERROR_THREAD_KEY));
}



#pragma mark -

void wi_error_enter_thread(void) {
	wi_error_t		*error;

	error = _wi_error_init(_wi_error_alloc());
	wi_hash_set_data_for_key(wi_thread_hash(), error, WI_STR(_WI_ERROR_THREAD_KEY));
	wi_release(error);
	
	wi_error_set_error(WI_ERROR_DOMAIN_NONE, WI_ERROR_NONE);
}



void wi_error_set_error(wi_error_domain_t domain, int code) {
	wi_error_t		*error;

	error = _wi_get_error();

	_WI_ERROR_ASSERT(error);

	error->domain = domain;
	error->code = code;

	wi_release(error->string);
	error->string = NULL;
}



void wi_error_set_errno(int code) {
	wi_error_set_error(WI_ERROR_DOMAIN_ERRNO, code);
}



void wi_error_set_ssl_error(void) {
#ifdef WI_SSL
	if(ERR_peek_error() != 0)
		wi_error_set_error(WI_ERROR_DOMAIN_SSL, ERR_get_error());
	else
		wi_error_set_error(WI_ERROR_DOMAIN_ERRNO, errno);
#endif
}



void wi_error_set_regex_error(regex_t *regex, int code) {
	wi_error_t		*error;
	char			string[256];

	error = _wi_get_error();

	_WI_ERROR_ASSERT(error);

	error->domain = WI_ERROR_DOMAIN_REGEX;
	error->code = code;
	
	wi_release(error->string);

	regerror(code, regex, string, sizeof(string));

	error->string = wi_string_init_with_cstring(wi_string_alloc(), string);
}



void wi_error_set_lib_error(int code) {
	wi_error_set_error(WI_ERROR_DOMAIN_LIB, code);
}



#pragma mark -

wi_string_t * wi_error_string(void) {
	wi_error_t		*error;

	error = _wi_get_error();

	_WI_ERROR_ASSERT(error);

	if(!error->string) {
		switch(error->domain) {
			case WI_ERROR_DOMAIN_ERRNO:
				error->string = wi_string_init_with_cstring(wi_string_alloc(), strerror(error->code));
				break;

			case WI_ERROR_DOMAIN_GAI:
				error->string = wi_string_init_with_cstring(wi_string_alloc(), gai_strerror(error->code));
				break;

			case WI_ERROR_DOMAIN_REGEX:
				break;

			case WI_ERROR_DOMAIN_SSL:
		#ifdef WI_SSL
				error->string = wi_string_init_with_cstring(wi_string_alloc(), ERR_reason_error_string(error->code));
		#endif
				break;

			case WI_ERROR_DOMAIN_NONE:
			case WI_ERROR_DOMAIN_LIB:
				error->string = wi_string_init_with_cstring(wi_string_alloc(), _wi_error_strings[error->code]);
				break;
		}
	}

	return error->string;
}



wi_error_domain_t wi_error_domain(void) {
	wi_error_t		*error;

	error = _wi_get_error();

	_WI_ERROR_ASSERT(error);

	return error->domain;
}



int32_t wi_error_code(void) {
	wi_error_t		*error;

	error = _wi_get_error();

	_WI_ERROR_ASSERT(error);

	return error->code;
}
