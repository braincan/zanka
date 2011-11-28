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

#import "WCUser.h"
#import "WCUserInfo.h"

#define WCUserInfoTransferViewTag		1


@interface WCUserInfo(Private)

- (id)_initUserInfoWithConnection:(WCServerConnection *)connection user:(WCUser *)user;

- (void)_reload;

- (unsigned int)_drawTransfers:(NSString *)string atOffset:(float *)offset;
- (void)_drawTransfer:(NSString *)transfer atOffset:(float *)offset;

- (void)_resizeTextField:(NSTextField *)textField withTextField:(NSTextField *)titleTextField atOffset:(float *)offset;
- (void)_moveView:(NSView *)view byYOffset:(float)offset;

@end


@implementation WCUserInfo(Private)

- (id)_initUserInfoWithConnection:(WCServerConnection *)connection user:(WCUser *)user {
	self = [super initWithWindowNibName:@"UserInfo" connection:connection];

	_user = [user retain];

	[self setReleasedWhenClosed:YES];
	[self window];

	[self _reload];
	
	[self retain];
	
	return self;
}



#pragma mark -

- (void)_reload {
	[[self connection] addObserver:self
						  selector:@selector(userInfoReceivedUserInfo:)
							  name:WCUserInfoReceivedUserInfo];

	[[self connection] sendCommand:WCInfoCommand withArgument:[NSSWF:@"%u", [_user userID]]];
}



#pragma mark -

- (unsigned int)_drawTransfers:(NSString *)string atOffset:(float *)offset {
	NSEnumerator	*enumerator;
	NSArray			*transfers;
	NSString		*transfer;
	
	if([string length] == 0)
		return 0;
	
	transfers	= [string componentsSeparatedByString:WCGroupSeparator];
	enumerator	= [transfers reverseObjectEnumerator];

	while((transfer = [enumerator nextObject]))
		[self _drawTransfer:transfer atOffset:offset];
	
	return [transfers count];
}



- (void)_drawTransfer:(NSString *)transfer atOffset:(float *)offset {
	WIProgressIndicator	*progressIndicator;
	NSTextField			*textField;
	NSArray				*fields;
	NSString			*path, *transferred, *size, *speed;

	fields		= [transfer componentsSeparatedByString:WCRecordSeparator];
	path		= [fields safeObjectAtIndex:0];
	transferred	= [fields safeObjectAtIndex:1];
	size		= [fields safeObjectAtIndex:2];
	speed		= [fields safeObjectAtIndex:3];

	textField = [[NSTextField alloc] init];
	[textField setTag:WCUserInfoTransferViewTag];
	[textField setEditable:NO];
	[textField setDrawsBackground:NO];
	[textField setBordered:NO];
	[textField setSelectable:YES];
	[textField setFont:[NSFont systemFontOfSize:11.0]];
	[textField setStringValue:[NSSWF:
		NSLS(@"%@ of %@, %@/s", "User info transfer (transferred, total, speed)"),
		[NSString humanReadableStringForSize:[transferred unsignedLongLongValue]],
		[NSString humanReadableStringForSize:[size unsignedLongLongValue]],
		[NSString humanReadableStringForSize:[speed unsignedLongLongValue]]]];
	[textField setFrame:NSMakeRect(85, *offset, 193, 14)];
	*offset += 16.0;
	[[[self window] contentView] addSubview:textField];
	[textField release];

	progressIndicator = [[WIProgressIndicator alloc] init];
	[progressIndicator setTag:WCUserInfoTransferViewTag];
	[progressIndicator setControlSize:NSSmallControlSize];
	[progressIndicator setIndeterminate:NO];
	[progressIndicator setMaxValue:1.0];
	[progressIndicator setDoubleValue:[transferred doubleValue] / [size doubleValue]];
	[progressIndicator setFrame:NSMakeRect(87, *offset, 193, 12)];
	*offset += 14.0;
	[[[self window] contentView] addSubview:progressIndicator];
	[progressIndicator release];

	textField = [[NSTextField alloc] init];
	[textField setTag:WCUserInfoTransferViewTag];
	[textField setEditable:NO];
	[textField setDrawsBackground:NO];
	[textField setBordered:NO];
	[textField setSelectable:YES];
	[textField setFont:[NSFont systemFontOfSize:11.0]];
	[textField setStringValue:[path lastPathComponent]];
	[textField setFrame:NSMakeRect(85, *offset, 193, 14)];
	*offset += 16.0;
	[[[self window] contentView] addSubview:textField];
	[textField release];
}



- (void)_resizeTextField:(NSTextField *)textField withTextField:(NSTextField *)titleTextField atOffset:(float *)offset {
	NSSize		size;
	NSPoint		point;
	double		height;
	
	if([[textField stringValue] length] == 0) {
		point = [textField frame].origin;
		[textField setFrameOrigin:NSMakePoint(point.x, -100.0)];

		point = [titleTextField frame].origin;
		[titleTextField setFrameOrigin:NSMakePoint(point.x, -100.0)];
	} else {
		[textField sizeToFitFromContent];
		height = [textField frame].size.height;
		size = [titleTextField frame].size;
		[titleTextField setFrameSize:NSMakeSize(size.width, height)];
		
		point = [textField frame].origin;
		[textField setFrameOrigin:NSMakePoint(point.x, *offset)];
		point = [titleTextField frame].origin;
		[titleTextField setFrameOrigin:NSMakePoint(point.x, *offset)];
		
		*offset += height + 2.0;
	}
}



#pragma mark -

- (void)_moveView:(NSView *)view byYOffset:(float)offset {
	NSRect		rect;
	
	rect = [view frame];
	rect.origin.y = offset;
	[view setFrame:rect];
}

@end


@implementation WCUserInfo

+ (id)userInfoWithConnection:(WCServerConnection *)connection user:(WCUser *)user {
	return [[[self alloc] _initUserInfoWithConnection:connection user:user] autorelease];
}



- (void)dealloc {
	[_user release];

	[super dealloc];
}



#pragma mark -

- (void)windowDidLoad {
	[self windowTemplate];
	
	[[self window] setTitle:[NSSWF:
		NSLS(@"%@ Info", @"User info window title (nick)"), [_user nick]]];
	
	[self setShouldCascadeWindows:YES];
	[self setShouldSaveWindowFrameOriginOnly:YES];
	[self setWindowFrameAutosaveName:@"UserInfo"];
	
	[super windowDidLoad];
}



- (void)windowWillClose:(NSNotification *)notification {
	[NSObject cancelPreviousPerformRequestsWithTarget:self];
}



- (void)connectionWillTerminate:(NSNotification *)notification {
	[self close];
}



- (void)userInfoReceivedUserInfo:(NSNotification *)notification {
	NSArray				*fields;
	NSString			*uid, *host, *version, *cipher, *cipherBits;
	NSString			*loginTime, *idleTime, *downloads, *uploads;
	NSDate				*date;
	NSRect				rect;
	NSTimeInterval		interval;
	unsigned int		count, transfers = 0;
	float				offset = 18.0, height;

	fields		= [[notification userInfo] objectForKey:WCArgumentsKey];
	uid			= [fields safeObjectAtIndex:0];
	host		= [fields safeObjectAtIndex:7];
	version		= [fields safeObjectAtIndex:8];
	cipher		= [fields safeObjectAtIndex:9];
	cipherBits	= [fields safeObjectAtIndex:10];
	loginTime	= [fields safeObjectAtIndex:11];
	idleTime	= [fields safeObjectAtIndex:12];
	downloads	= [fields safeObjectAtIndex:13];
	uploads		= [fields safeObjectAtIndex:14];

	if([uid unsignedIntValue] != [_user userID])
		return;
	
	// --- set fields
	[_iconImageView setImage:[_user iconWithIdleTint:NO]];
	[_nickTextField setStringValue:[_user nick]];
	[_statusTextField setStringValue:[_user status]];
	[_loginTextField setStringValue:[_user login]];
	[_idTextField setStringValue:uid];
	[_addressTextField setStringValue:[_user address]];
	[_hostTextField setStringValue:host];
	[_versionTextField setStringValue:[version wiredVersion]];
	[_cipherTextField setStringValue:[NSSWF:@"%@/%@ %@",
		cipher,
		cipherBits,
		NSLS(@"bits", "Cipher string")]];
	
	date = [NSDate dateWithISO8601String:loginTime];
	interval = [[NSDate date] timeIntervalSince1970] - [date timeIntervalSince1970];

	[_loginTimeTextField setStringValue:[NSSWF:
		NSLS(@"%@,\nsince %@", @"Time stamp (time counter, time string)"),
		[NSString humanReadableStringForTimeInterval:interval],
		[date commonDateStringWithSeconds:NO relative:YES capitalized:NO]]];

	date = [NSDate dateWithISO8601String:idleTime];
	interval = [[NSDate date] timeIntervalSince1970] - [date timeIntervalSince1970];

	[_idleTimeTextField setStringValue:[NSSWF:
		NSLS(@"%@,\nsince %@", @"Time stamp (time counter, time string)"),
		[NSString humanReadableStringForTimeInterval:interval],
		[date commonDateStringWithSeconds:NO relative:YES capitalized:NO]]];

	// --- remove previous transfer views
	[[[[self window] contentView] subviewsWithTag:WCUserInfoTransferViewTag]
		makeObjectsPerformSelector:@selector(removeFromSuperviewWithoutNeedingDisplay)];

	// --- show uploads
	count = [self _drawTransfers:uploads atOffset:&offset];
	[self _moveView:_uploadsTitleTextField byYOffset:(count > 0) ? offset - 16.0 : -100.0];
	transfers += count;
	
	// --- show downloads
	count = [self _drawTransfers:downloads atOffset:&offset];
	[self _moveView:_downloadsTitleTextField byYOffset:(count > 0) ? offset - 16.0 : -100.0];
	transfers += count;
	
	// --- resize fields
	[self _resizeTextField:_idleTimeTextField withTextField:_idleTimeTitleTextField atOffset:&offset];
	[self _resizeTextField:_loginTimeTextField withTextField:_loginTimeTitleTextField atOffset:&offset];
	[self _resizeTextField:_cipherTextField withTextField:_cipherTitleTextField atOffset:&offset];
	[self _resizeTextField:_versionTextField withTextField:_versionTitleTextField atOffset:&offset];
	[self _resizeTextField:_hostTextField withTextField:_hostTitleTextField atOffset:&offset];
	[self _resizeTextField:_addressTextField withTextField:_addressTitleTextField atOffset:&offset];
	[self _resizeTextField:_idTextField withTextField:_idTitleTextField atOffset:&offset];
	[self _resizeTextField:_loginTextField withTextField:_loginTitleTextField atOffset:&offset];
	[self _resizeTextField:_statusTextField withTextField:_statusTitleTextField atOffset:&offset];

	[self _moveView:_nickTextField byYOffset:offset + 23.0];
	[self _moveView:_iconImageView byYOffset:offset + 12.0];
	
	// --- resize window
	rect = [[self window] frame];
	height = rect.size.height;
	rect.size.height = offset + 84.0;
	rect.origin.y -= rect.size.height - height;
	[[self window] setFrame:rect display:YES animate:YES];

	if(![[self window] isOnScreen])
		[self showWindow:self];
	
	// --- reschedule
	[[self connection] removeObserver:self name:WCUserInfoReceivedUserInfo];

	[self performSelector:@selector(_reload) withObject:NULL afterDelay:(transfers > 0) ? 5.0 : 10.0];
}

@end
