/* $Id$ */

/*
 *  Copyright (c) 2003-2004 Axel Andersson
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

#ifndef WD_OPTIONS_H
#define WD_OPTIONS_H 1

#include <sys/types.h>
#include <stdbool.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>


#define ARRAY_SIZE(array)		(sizeof(array) / sizeof(*(array)))

#define WD_CONFIG_FILE			"etc/wired.conf"

#define WD_STRING_SIZE			256
#define WD_LIST_SIZE			32


struct settings {
	char						name[WD_STRING_SIZE];
	char						description[WD_STRING_SIZE];
	char						category[WD_STRING_SIZE];
	char						address[WD_STRING_SIZE];
	unsigned int				port;

	struct passwd				*user;
	struct group				*group;

	unsigned int				idletime;
	unsigned int				bantime;
	bool						zeroconf;
	
	unsigned int				totaldownloads;
	unsigned int				totaluploads;
	unsigned int				clientdownloads;
	unsigned int				clientuploads;
	unsigned int				totaldownloadspeed;
	unsigned int				totaluploadspeed;
	
	char						controlcipher[WD_STRING_SIZE];
	char						transfercipher[WD_STRING_SIZE];

	bool						hotline;
	unsigned int				hotline_port;

	char						pid[MAXPATHLEN];
	char						users[MAXPATHLEN];
	char						groups[MAXPATHLEN];
	char						news[MAXPATHLEN];
	char						files[MAXPATHLEN];
	char						status[MAXPATHLEN];
	char						banlist[MAXPATHLEN];
	char						certificate[MAXPATHLEN];
};

struct options {
	char						*name;
	void						*affects;
	int							(*action) (char *, char *, void *);
};


int								wd_read_config(void);
void							wd_reset_config(void);
int								wd_parse_config(char *, char **, char **);
int								wd_config_index(char *);
int								wd_setconfig(char *, char *);

int								wd_config_number(char *, char *, void *);
int								wd_config_string(char *, char *, void *);
int								wd_config_path(char *, char *, void *);
int								wd_config_user(char *, char *, void *);
int								wd_config_group(char *, char *, void *);
int								wd_config_boolean(char *, char *, void *);
int								wd_config_list(char *, char *, void *);


extern struct settings			wd_settings;
extern struct options			wd_options[];

extern const char				*wd_signals[];

#endif /* WD_OPTIONS_H */
