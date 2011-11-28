/* $Id$ */

/*
 *  Copyright (c) 2007 Axel Andersson
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

#import "FHWindowController.h"

@class FHTableView;
@class FHHandler;

@interface FHBrowserController : FHWindowController {
	IBOutlet NSBox					*_contentBox;
	IBOutlet WISplitView			*_splitView;
	IBOutlet NSView					*_leftView;
	IBOutlet FHTableView			*_tableView;
	IBOutlet NSTableColumn			*_fileTableColumn;
	IBOutlet NSView					*_rightView;
	IBOutlet NSScrollView			*_scrollView;

	IBOutlet NSBox					*_statusBox;
	IBOutlet NSTextField			*_leftStatusTextField;
	IBOutlet NSTextField			*_rightStatusTextField;

	IBOutlet NSPanel				*_openURLPanel;
	IBOutlet NSTextView				*_openURLTextView;
	IBOutlet NSPopUpButton			*_openURLPopUpButton;

	IBOutlet NSPanel				*_openSpotlightPanel;
	IBOutlet NSTextView				*_openSpotlightTextView;

	IBOutlet NSPanel				*_screenPanel;
	IBOutlet NSPopUpButton			*_screenPopUpButton;
	IBOutlet NSPopUpButton			*_screenBackgroundPopUpButton;
	IBOutlet NSMenuItem				*_screenBackgroundBlackMenuItem;
	IBOutlet NSMenuItem				*_screenBackgroundGrayMenuItem;
	IBOutlet NSMenuItem				*_screenBackgroundWhiteMenuItem;
	IBOutlet NSButton				*_screenAutoSwitchButton;
	IBOutlet NSTextField			*_screenAutoSwitchTextField;

	FHHandler						*_handler;

	NSMutableDictionary				*_toolbarItems;

	NSSize							_tableViewSize;
	BOOL							_deletingFile;
	BOOL							_switchingURL;
	NSInteger						_previousRow;
}


#define FHBrowserControllerDidLoadHandler	@"FHBrowserControllerDidLoadHandler"


- (IBAction)openParent:(id)sender;
- (IBAction)openFile:(id)sender;
- (IBAction)openDirectory:(id)sender;

- (IBAction)autoSwitch:(id)sender;

- (void)loadURL:(WIURL *)url;
- (WIURL *)URL;
- (FHHandler *)handler;

@end
