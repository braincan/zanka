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

#import "WCAccount.h"
#import "WCAccounts.h"
#import "WCApplication.h"
#import "WCConnection.h"
#import "WCConsole.h"
#import "WCFile.h"
#import "WCFiles.h"
#import "WCMain.h"
#import "WCMessages.h"
#import "WCNews.h"
#import "WCPreferences.h"
#import "WCPublicChat.h"
#import "WCSettings.h"
#import "WCToolbar.h"
#import "WCToolbarItem.h"
#import "WCTransfers.h"

@implementation WCToolbar

- (id)initWithConnection:(WCConnection *)connection {
	self = [super init];
	
	// --- get parameters
	_connection = [connection retain];
	
	// --- initiate our dictionary of toolbar items
	_toolbarItems = [[NSMutableDictionary alloc] init];

	// --- subscribe to these
	[[NSNotificationCenter defaultCenter]
		addObserver:self
		selector:@selector(applicationDidChangeStatus:)
		name:WCApplicationDidChangeStatus
		object:NULL];

	[[NSNotificationCenter defaultCenter]
		addObserver:self
		selector:@selector(connectionShouldTerminate:)
		name:WCConnectionShouldTerminate
		object:NULL];

	[[NSNotificationCenter defaultCenter]
		addObserver:self
		selector:@selector(connectionPrivilegesDidChange:)
		name:WCConnectionPrivilegesDidChange
		object:NULL];
	
	return self;
}



- (id)initWithPreferences:(WCPreferences *)preferences {
	self = [super init];
	
	// --- get parameters
	_preferences = [preferences retain];
	
	// --- initiate our dictionary of toolbar items
	_toolbarItems = [[NSMutableDictionary alloc] init];
	
	return self;
}



- (void)dealloc {
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	
	[_connection release];
	[_preferences release];
	[_toolbar release];
	[_toolbarItems release];
	
	[super dealloc];
}



#pragma mark -

- (void)applicationDidChangeStatus:(NSNotification *)notification {
	if(!_connection)
		return;
	
	[_toolbar validateVisibleItems];
}



- (void)connectionShouldTerminate:(NSNotification *)notification {
	if([notification object] == _connection)
		[self release];
}



- (void)connectionPrivilegesDidChange:(NSNotification *)notification {
	if([notification object] != _connection)
		return;
	
	[_toolbar validateVisibleItems];
}



#pragma mark -

- (NSToolbar *)chatToolbar {
	_toolbar = [[NSToolbar alloc] initWithIdentifier:WCChatToolbar];
	
	[self addItem:@"News"
		  name:NSLocalizedString(@"News", @"News toolbar item")
		  settingSelector:@selector(setImage:)
		  content:[NSImage imageNamed:@"News"]
		  action:@selector(news:)];

	[self addItem:@"Messages"
		  name:NSLocalizedString(@"Messages", @"Messages toolbar item")
		  settingSelector:@selector(setImage:)
		  content:[NSImage imageNamed:@"Messages"]
		  action:@selector(messages:)];

	[self addItem:@"Files"
		  name:NSLocalizedString(@"Files", @"Files toolbar item")
		  settingSelector:@selector(setImage:)
		  content:[NSImage imageNamed:@"Folder"]
		  action:@selector(files:)];

	[self addItem:@"Transfers"
		  name:NSLocalizedString(@"Transfers", @"Transfers toolbar item")
		  settingSelector:@selector(setImage:)
		  content:[NSImage imageNamed:@"Transfers"]
		  action:@selector(transfers:)];

	[self addItem:@"Console"
		  name:NSLocalizedString(@"Console", @"Console toolbar item")
		  settingSelector:@selector(setImage:)
		  content:[NSImage imageNamed:@"Console"]
		  action:@selector(console:)];

	[self addItem:@"Accounts"
		  name:NSLocalizedString(@"Accounts", @"Accounts toolbar item")
		  settingSelector:@selector(setImage:)
		  content:[NSImage imageNamed:@"Accounts"]
		  action:@selector(accounts:)];
/*
	[self addItem:@"PostNews"
		  name:NSLocalizedString(@"Post News", @"Post news toolbar item")
		  settingSelector:@selector(setImage:)
		  content:[NSImage imageNamed:@"PostNews"]
		  action:@selector(postNews:)];

	[self addItem:@"Broadcast"
		  name:NSLocalizedString(@"Broadcast", @"Broadcast toolbar item")
		  settingSelector:@selector(setImage:)
		  content:[NSImage imageNamed:@"Broadcast"]
		  action:@selector(broadcast:)];
*/
	[self addItem:@"Disconnect"
		  name:NSLocalizedString(@"Disconnect", @"Disconnect toolbar item")
		  settingSelector:@selector(setImage:)
		  content:[NSImage imageNamed:@"Disconnect"]
		  action:@selector(disconnect:)];

	[self addItem:@"LargerText"
		  name:NSLocalizedString(@"Larger Text", @"Larger text toolbar item")
		  settingSelector:@selector(setImage:)
		  content:[NSImage imageNamed:@"LargerText"]
		  action:@selector(largerText:)];

	[self addItem:@"SmallerText"
		  name:NSLocalizedString(@"Smaller Text", @"Smaller text toolbar item")
		  settingSelector:@selector(setImage:)
		  content:[NSImage imageNamed:@"SmallerText"]
		  action:@selector(smallerText:)];

	[_toolbar setDelegate:self];
	[_toolbar setVisible:NO];
	[_toolbar setAllowsUserCustomization:YES];
	[_toolbar setAutosavesConfiguration:YES];
	[_toolbar setDisplayMode:NSToolbarDisplayModeDefault];
	
	return _toolbar;
}



- (NSToolbar *)preferencesToolbar {
	_toolbar = [[NSToolbar alloc] initWithIdentifier:WCPreferencesToolbar];

	[self addItem:@"General"
		  name:NSLocalizedString(@"General", @"General toolbar item")
		  settingSelector:@selector(setImage:)
		  content:[NSImage imageNamed:@"General"]
		  action:@selector(preferences:)];
	
	[self addItem:@"Personal"
		  name:NSLocalizedString(@"Personal", @"Personal toolbar item")
		  settingSelector:@selector(setImage:)
		  content:[NSImage imageNamed:@"Personal"]
		  action:@selector(preferences:)];
	
	[self addItem:@"Bookmarks"
		  name:NSLocalizedString(@"Bookmarks", @"Bookmarks toolbar item")
		  settingSelector:@selector(setImage:)
		  content:[NSImage imageNamed:@"Bookmarks"]
		  action:@selector(preferences:)];

	[self addItem:@"Chat"
		  name:NSLocalizedString(@"Chat", @"Chat toolbar item")
		  settingSelector:@selector(setImage:)
		  content:[NSImage imageNamed:@"Chat"]
		  action:@selector(preferences:)];

	[self addItem:@"News"
		  name:NSLocalizedString(@"News", @"News toolbar item")
		  settingSelector:@selector(setImage:)
		  content:[NSImage imageNamed:@"News"]
		  action:@selector(preferences:)];

	[self addItem:@"Messages"
		  name:NSLocalizedString(@"Messages", @"Messages toolbar item")
		  settingSelector:@selector(setImage:)
		  content:[NSImage imageNamed:@"Messages"]
		  action:@selector(preferences:)];

	[self addItem:@"Files"
		  name:NSLocalizedString(@"Files", @"Files toolbar item")
		  settingSelector:@selector(setImage:)
		  content:[NSImage imageNamed:@"Folder"]
		  action:@selector(preferences:)];

	[self addItem:@"Trackers"
		  name:NSLocalizedString(@"Trackers", @"Trackers toolbar item")
		  settingSelector:@selector(setImage:)
		  content:[NSImage imageNamed:@"Trackers"]
		  action:@selector(preferences:)];

	[self addItem:@"Ignores"
		  name:NSLocalizedString(@"Ignores", @"Ignores toolbar item")
		  settingSelector:@selector(setImage:)
		  content:[NSImage imageNamed:@"Ignores"]
		  action:@selector(preferences:)];
	
	[_toolbar setDelegate:self];
	[_toolbar setAllowsUserCustomization:NO];
	[_toolbar setAutosavesConfiguration:NO];
	[_toolbar setDisplayMode:NSToolbarDisplayModeDefault];
	
	return _toolbar;
}



#pragma mark -

- (void)addItem:(NSString *)identifier name:(NSString *)name settingSelector:(SEL)settingSelector content:(id)content action:(SEL)action {
	WCToolbarItem	*item;
	
	item = [[WCToolbarItem alloc] initWithItemIdentifier:identifier];
	
	[item setLabel:name];
	[item setPaletteLabel:name];
	[item setToolTip:name];
	[item setTarget:self];
	[item setAction:action];
	[item performSelector:settingSelector withObject:content];
	
	[_toolbarItems setObject:item forKey:identifier];
	
	[item release];
}



- (BOOL)validateToolbarItem:(WCToolbarItem *)item {
	// --- if broadcast is available
	if([[item itemIdentifier] isEqualToString:@"Broadcast"])
		return [[_connection account] broadcast];
	
	// --- if clicking disconnect will terminate connection without confirmation
	if([[item itemIdentifier] isEqualToString:@"Disconnect"] &&
	   ![[WCSettings objectForKey:WCConfirmDisconnect] boolValue])
		return [NSApp isActive];

	return YES;
}



#pragma mark -

- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar itemForItemIdentifier:(NSString *)identifier willBeInsertedIntoToolbar:(BOOL)willBeInsertedIntoToolbar {
	return [_toolbarItems objectForKey:identifier];
}



- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar {
	NSString		*identifier;
	
	// --- figure out what toolbar it wants
	identifier = [toolbar identifier];
	
	if([identifier isEqualToString:WCChatToolbar]) {
		return [NSArray arrayWithObjects:
			@"News",
			@"Messages",
			@"Files",
			@"Transfers",
			@"Console",
			@"Accounts",
//			NSToolbarSeparatorItemIdentifier,
//			@"PostNews",
//			@"Broadcast",
			NSToolbarFlexibleSpaceItemIdentifier,
			@"Disconnect",
			NULL];
	}
	else if([identifier isEqualToString:WCPreferencesToolbar]) {
		return [NSArray arrayWithObjects:
			@"General",
			@"Personal",
			@"Bookmarks",
			@"Chat",
			@"Messages",
			@"News",
			@"Files",
//			@"Trackers",
			@"Ignores",
			NULL];
	}
	
	return NULL;
}



- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar {
	NSString		*identifier;
	
	// --- figure out what toolbar it wants
	identifier = [toolbar identifier];
	
	if([identifier isEqualToString:WCChatToolbar]) {
		return [NSArray arrayWithObjects:
			@"News",
			@"Messages",
			@"Files",
			@"Transfers",
			@"Console",
			@"Accounts",
//			@"Broadcast",
//			@"PostNews",
			@"LargerText",
			@"SmallerText",
			@"Disconnect",
			NSToolbarSeparatorItemIdentifier,
			NSToolbarSpaceItemIdentifier,
			NSToolbarFlexibleSpaceItemIdentifier,
			NSToolbarCustomizeToolbarItemIdentifier,
			NULL];
	}
	
	return NULL;
}



#pragma mark -

- (IBAction)news:(id)sender {
	[[_connection news] showWindow:self];
}



- (IBAction)messages:(id)sender {
	[[_connection messages] showWindow:self];
}



- (IBAction)files:(id)sender {
	WCFile		*root;
	
	root = [[WCFile alloc] initWithType:WCFileTypeDirectory];
	[root setPath:@"/"];

	[[WCFiles alloc] initWithConnection:_connection path:root];
	
	[root release];
}



- (IBAction)transfers:(id)sender {
	[[_connection transfers] showWindow:self];
}



- (IBAction)console:(id)sender {
	[[_connection console] showWindow:self];
}



- (IBAction)accounts:(id)sender {
	[[_connection accounts] showWindow:self];
}



- (IBAction)postNews:(id)sender {
	[[_connection news] post:self];
}



- (IBAction)broadcast:(id)sender {
	[[_connection messages] showBroadcast];
}



- (IBAction)smallerText:(id)sender {
	[WCSharedMain smallerText:sender];
}



- (IBAction)largerText:(id)sender {
	[WCSharedMain largerText:sender];
}



- (IBAction)disconnect:(id)sender {
	[[[_connection chat] window] performClose:self];
}



#pragma mark -

- (IBAction)preferences:(id)sender {
	[[_preferences window] setTitle:[sender label]];
	[_preferences selectTab:[sender itemIdentifier]];
}

@end
