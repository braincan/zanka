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

@interface WCSettings : WISettings

#define WCNick									@"WCNick"
#define WCStatus								@"WCStatus"
#define WCCustomIcon							@"WCCustomIcon"

#define WCShowConnectAtStartup					@"WCShowConnectAtStartup"
#define WCShowDockAtStartup						@"WCShowDockAtStartup"
#define WCShowTrackersAtStartup					@"WCShowTrackersAtStartup"

#define WCAutoHideOnSwitch						@"WCAutoHideOnSwitch"
#define WCConfirmDisconnect						@"WCConfirmDisconnect"

#define WCChatTextColor							@"WCChatTextColor"
#define WCChatBackgroundColor					@"WCChatBackgroundColor"
#define WCChatEventsColor						@"WCChatEventsColor"
#define WCChatURLsColor							@"WCChatURLsColor"
#define WCChatFont								@"WCChatFont"
#define WCChatUserListFont						@"WCChatUserListFont"
#define WCChatUserListAlternateRows				@"WCChatUserListAlternateRows"
#define WCChatUserListIconSize					@"WCChatUserListIconSize"
#define WCChatUserListIconSizeLarge					1
#define WCChatUserListIconSizeSmall					0

#define WCMessagesTextColor						@"WCMessagesTextColor"
#define WCMessagesBackgroundColor				@"WCMessagesBackgroundColor"
#define WCMessagesFont							@"WCMessagesFont"
#define WCMessagesListFont						@"WCMessagesListFont"
#define WCMessagesListAlternateRows				@"WCMessagesListAlternateRows"

#define WCNewsTextColor							@"WCNewsTextColor"
#define WCNewsBackgroundColor					@"WCNewsBackgroundColor"
#define WCNewsTitlesColor						@"WCNewsTitlesColor"
#define WCNewsFont								@"WCNewsFont"
#define WCNewsTitlesFont						@"WCNewsTitlesFont"

#define WCFilesFont								@"WCFilesFont"
#define WCFilesAlternateRows					@"WCFilesAlternateRows"

#define WCTransfersShowProgressBar				@"WCTransfersShowProgressBar"
#define WCTransfersAlternateRows				@"WCTransfersAlternateRows"

#define WCPreviewTextColor						@"WCPreviewTextColor"
#define WCPreviewBackgroundColor				@"WCPreviewBackgroundColor"
#define WCPreviewFont							@"WCPreviewFont"

#define WCTrackersAlternateRows					@"WCTrackersAlternateRows"

#define WCBookmarks								@"WCBookmarks"
#define WCBookmarksName								@"Name"
#define WCBookmarksAddress							@"Address"
#define WCBookmarksLogin							@"Login"
#define WCBookmarksPassword							@"Password"
#define WCBookmarksIdentifier						@"Identifier"
#define WCBookmarksNick								@"Nick"
#define WCBookmarksStatus							@"Status"
#define WCBookmarksAutoJoin							@"AutoJoin"

#define WCChatStyle								@"WCChatStyle"
#define WCChatStyleWired							0
#define WCChatStyleIRC								1
#define WCHistoryScrollback						@"WCHistoryScrollback"
#define WCHistoryScrollbackModifier				@"WCHistoryScrollbackModifier"
#define WCHistoryScrollbackModifierNone				0
#define WCHistoryScrollbackModifierCommand			1
#define WCHistoryScrollbackModifierOption			2
#define WCHistoryScrollbackModifierControl			3
#define WCTabCompleteNicks						@"WCTabCompleteNicks"
#define WCTabCompleteNicksString				@"WCTabCompleteNicksString"
#define WCTimestampChat							@"WCTimestampChat"
#define WCTimestampChatInterval					@"WCTimestampChatInterval"
#define WCTimestampEveryLine					@"WCTimestampEveryLine"
#define WCTimestampEveryLineColor				@"WCTimestampEveryLineColor"
#define WCShowSmileys							@"WCShowSmileys"

#define WCHighlights							@"WCHighlights"
#define WCHighlightsPattern							@"WCHighlightsPattern"
#define WCHighlightsColor							@"WCHighlightsColor"

#define WCIgnores								@"WCIgnores"
#define WCIgnoresNick								@"Nick"
#define WCIgnoresLogin								@"Login"
#define WCIgnoresAddress							@"Address"

#define WCEvents								@"WCEvents"
#define WCEventsEvent								@"WCEventsEvent"
#define WCEventsServerConnected							1
#define WCEventsServerDisconnected						2
#define WCEventsError									3
#define WCEventsUserJoined								4
#define WCEventsUserChangedNick							5
#define WCEventsUserLeft								6
#define WCEventsChatReceived							7
#define WCEventsMessageReceived							8
#define WCEventsNewsPosted								9
#define WCEventsBroadcastReceived						10
#define WCEventsTransferStarted							11
#define WCEventsTransferFinished						12
#define WCEventsUserChangedStatus						13
#define WCEventsPlaySound							@"WCEventsPlaySound"
#define WCEventsSound								@"WCEventsSound"
#define WCEventsBounceInDock						@"WCEventsBounceInDock"
#define WCEventsPostInChat							@"WCEventsPostInChat"
#define WCEventsShowDialog							@"WCEventsShowDialog"

#define WCDownloadFolder						@"WCDownloadFolder"
#define WCOpenFoldersInNewWindows				@"WCOpenFoldersInNewWindows"
#define WCQueueTransfers						@"WCQueueTransfers"
#define WCEncryptTransfers						@"WCEncryptTransfers"
#define WCRemoveTransfers						@"WCRemoveTransfers"
#define WCFilesStyle							@"WCFilesStyle"
#define WCFilesStyleList							0
#define WCFilesStyleBrowser							1

#define WCTrackerBookmarks						@"WCTrackerBookmarks"
#define WCTrackerBookmarksName						@"Name"
#define WCTrackerBookmarksAddress					@"Address"

#define WCWindowTemplates						@"WCWindowTemplates"
#define WCWindowTemplatesDefault					@"WCWindowTemplatesDefault"

#define WCSSLControlCiphers						@"WCSSLControlCiphers"
#define WCSSLNullControlCiphers					@"WCSSLNullControlCiphers"
#define WCSSLTransferCiphers					@"WCSSLTransferCiphers"
#define WCSSLNullTransferCiphers				@"WCSSLNullTransferCiphers"


+ (NSDictionary *)eventForTag:(int)tag;
+ (void)setEvent:(NSDictionary *)event forTag:(int)tag;

+ (NSDictionary *)bookmarkAtIndex:(unsigned int)index;
+ (void)addBookmark:(NSDictionary *)bookmark;
+ (void)setBookmark:(NSDictionary *)bookmark atIndex:(unsigned int)index;
+ (void)removeBookmarkAtIndex:(unsigned int)index;

+ (NSDictionary *)trackerBookmarkAtIndex:(unsigned int)index;
+ (void)addTrackerBookmark:(NSDictionary *)bookmark;
+ (void)setTrackerBookmark:(NSDictionary *)bookmark atIndex:(unsigned int)index;
+ (void)removeTrackerBookmarkAtIndex:(unsigned int)index;

+ (NSDictionary *)highlightAtIndex:(unsigned int)index;
+ (void)addHighlight:(NSDictionary *)highlight;
+ (void)setHighlight:(NSDictionary *)highlight atIndex:(unsigned int)index;
+ (void)removeHighlightAtIndex:(unsigned int)index;

+ (NSDictionary *)ignoreAtIndex:(unsigned int)index;
+ (void)addIgnore:(NSDictionary *)ignore;
+ (void)setIgnore:(NSDictionary *)ignore atIndex:(unsigned int)index;
+ (void)removeIgnoreAtIndex:(unsigned int)index;

+ (NSDictionary *)windowTemplateForKey:(NSString *)key;
+ (void)setWindowTemplate:(NSDictionary *)windowTemplate forKey:(NSString *)key;

@end
