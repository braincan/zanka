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

#import "WCWindowController.h"

@class WCToolbar, WCIconMatrix;

@interface WCPreferences : WCWindowController {
	IBOutlet NSTabView			*_preferencesTabView;
		
	IBOutlet NSTextField		*_fontTextField;
	IBOutlet NSButton			*_showConnectAtStartupButton;
	IBOutlet NSButton			*_confirmDisconnectButton;

	IBOutlet NSTextField		*_nickTextField;
	IBOutlet WCIconMatrix		*_iconMatrix;

	IBOutlet NSTableView		*_bookmarkTableView;
	IBOutlet NSTableColumn		*_bookmarkNameTableColumn;
	IBOutlet NSTableColumn		*_bookmarkAddressTableColumn;
	IBOutlet NSTableColumn		*_bookmarkLoginTableColumn;
	IBOutlet NSPanel			*_bookmarkPanel;
	IBOutlet NSTextField		*_bookmarkNameTextField;
	IBOutlet NSTextField		*_bookmarkAddressTextField;
	IBOutlet NSTextField		*_bookmarkLoginTextField;
	IBOutlet NSSecureTextField	*_bookmarkPasswordTextField;

	IBOutlet NSColorWell		*_chatTextColorWell;
	IBOutlet NSColorWell		*_chatBackgroundColorWell;
	IBOutlet NSColorWell		*_chatEventColorWell;
	IBOutlet NSColorWell		*_chatURLColorWell;
	IBOutlet NSTextField		*_nickCompleteWithTextField;
	IBOutlet NSButton			*_showEventTimestampsButton;
	IBOutlet NSButton			*_showChatTimestampsButton;

	IBOutlet NSColorWell		*_messageTextColorWell;
	IBOutlet NSColorWell		*_messageBackgroundColorWell;
	IBOutlet NSButton			*_showMessagesInForegroundButton;
	
	IBOutlet NSColorWell		*_newsTextColorWell;
	IBOutlet NSColorWell		*_newsBackgroundColorWell;
	IBOutlet NSButton			*_loadNewsOnLoginButton;
	IBOutlet NSPopUpButton		*_soundsPopUpButton;
	
	IBOutlet NSTextField		*_downloadFolderTextField;
	IBOutlet NSButton			*_openFoldersInNewWindowsButton;
	IBOutlet NSButton			*_queueTransfersButton;
	IBOutlet NSButton			*_encryptTransfersButton;
	
	IBOutlet NSTableView		*_trackerTableView;
	IBOutlet NSTableColumn		*_trackerNameTableColumn;
	IBOutlet NSTableColumn		*_trackerAddressTableColumn;
	IBOutlet NSPanel			*_trackerPanel;
	IBOutlet NSTextField		*_trackerNameTextField;
	IBOutlet NSTextField		*_trackerAddressTextField;
	
	IBOutlet NSTableView		*_ignoreTableView;
	IBOutlet NSTableColumn		*_ignoreNickTableColumn;
	IBOutlet NSTableColumn		*_ignoreLoginTableColumn;
	IBOutlet NSTableColumn		*_ignoreAddressTableColumn;
	IBOutlet NSPanel			*_ignorePanel;
	IBOutlet NSTextField		*_ignoreNickTextField;
	IBOutlet NSTextField		*_ignoreLoginTextField;
	IBOutlet NSTextField		*_ignoreAddressTextField;

	WCToolbar					*_toolbar;
	
	NSFont						*_currentFont;
	int							_currentRow;
}


#define							WCPreferencesDidChange		@"WCPreferencesDidChange"
#define							WCNickDidChange				@"WCNickDidChange"
#define							WCIconDidChange				@"WCIconDidChange"
	

- (void)						selectTab:(NSString *)identifier;

- (IBAction)					ok:(id)sender;

- (IBAction)					showFontPanel:(id)sender;
- (IBAction)					selectDownloadFolder:(id)sender;
- (IBAction)					selectSound:(id)sender;

- (IBAction)					addBookmark:(id)sender;
- (IBAction)					editBookmark:(id)sender;
- (IBAction)					deleteBookmark:(id)sender;

- (IBAction)					okBookmark:(id)sender;
- (IBAction)					cancelBookmark:(id)sender;

- (IBAction)					addTracker:(id)sender;
- (IBAction)					editTracker:(id)sender;
- (IBAction)					deleteTracker:(id)sender;

- (IBAction)					okTracker:(id)sender;
- (IBAction)					cancelTracker:(id)sender;

- (IBAction)					addIgnore:(id)sender;
- (IBAction)					editIgnore:(id)sender;
- (IBAction)					deleteIgnore:(id)sender;

- (IBAction)					okIgnore:(id)sender;
- (IBAction)					cancelIgnore:(id)sender;

@end
