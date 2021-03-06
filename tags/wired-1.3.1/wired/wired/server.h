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

#ifndef WD_SERVER_H
#define WD_SERVER_H 1

#include <wired/wired.h>

#include "chats.h"

#define WD_ZEROCONF_NAME			"_wired._tcp"
#define WD_SERVER_PORT				2000
#define WD_TRACKER_PORT				2002

#define WD_MESSAGE_SEPARATOR		'\4'
#define WD_MESSAGE_SEPARATOR_STR	"\4"
#define WD_FIELD_SEPARATOR			'\34'
#define WD_GROUP_SEPARATOR			'\35'
#define WD_RECORD_SEPARATOR			'\36'


void								wd_init_server(void);
void								wd_fork_server(void);
void								wd_config_server(void);

void								wd_init_ssl(void);

void								wd_reply(unsigned int, wi_string_t *, ...);
void								wd_reply_error(void);
void								wd_sreply(wi_socket_t *, unsigned int, wi_string_t *, ...);

void								wd_broadcast(wd_cid_t, unsigned int, wi_string_t *, ...);
void								wd_broadcast_lock(void);
void								wd_broadcast_unlock(void);


extern wi_string_t					*wd_banner;

#endif /* WD_SERVER_H */
