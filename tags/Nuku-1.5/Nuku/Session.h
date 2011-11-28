/*
 * Copyright � 2000-2002 Axel Andersson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
 
#import <Cocoa/Cocoa.h>
#import "Settings.h"

@class Controller;

@interface Session : NSObject {
	IBOutlet Controller			*controllerOutlet;
	
	IBOutlet NSWindow			*windowSession;
	IBOutlet NSWindow			*windowReport;
	
	IBOutlet NSView				*viewPrint;
	IBOutlet NSTextView			*viewText;
	
	IBOutlet NSPopUpButton		*selectSession;
    IBOutlet NSTextField		*fieldTime;
    IBOutlet NSTextField		*fieldKana;
	IBOutlet NSStepper			*stepperTime;
	IBOutlet NSStepper			*stepperKana;
	IBOutlet NSButton			*boxLock;
	IBOutlet NSButton			*boxAwait;
	
	IBOutlet NSTextField		*reportAnswered;
	IBOutlet NSTextField		*reportMissed;
	IBOutlet NSTextField		*reportCorrect;
	IBOutlet NSTextField		*reportIncorrect;
	IBOutlet NSTextField		*reportPercent;
	IBOutlet NSTextField		*reportTime;
	IBOutlet NSTextView			*reportComment;

	Settings					*settings;
	
	BOOL						done;
}


- (IBAction)					newSession:(id)sender;
- (IBAction)					start:(id)sender;

- (IBAction)					switchType:(id)sender;
- (IBAction)					enterTime:(id)sender;
- (IBAction)					enterKana:(id)sender;
- (IBAction)					stepTime:(id)sender;
- (IBAction)					stepKana:(id)sender;

- (void)						showReport;
- (IBAction)					print:(id)sender;

- (BOOL)						isDone;
- (void)						done;

@end
