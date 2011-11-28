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

#import <WiredAdditions/WISplitView.h>

@interface WISplitView(Private)

- (void)_initSplitView;

- (void)_saveSplitViewPosition;
- (void)_loadSplitViewPosition;

@end



@implementation WISplitView(Private)

- (void)_initSplitView {
	[[NSNotificationCenter defaultCenter]
		addObserver:self
		   selector:@selector(_WI_applicationWillTerminate:)
			   name:NSApplicationWillTerminateNotification
			 object:NULL];
}



#pragma mark -

- (void)_saveSplitViewPosition {
	NSMutableArray	*frames;
	NSString		*key;
	int				i, subviewCount;
	
	if([[self autosaveName] length] > 0) {
		frames = [NSMutableArray array];
		subviewCount = [[self subviews] count];
		
		for(i = 0; i < subviewCount; i++)
			[frames addObject:NSStringFromRect([[[self subviews] objectAtIndex:i] frame])];

			key = [NSSWF:@"%@ %@ Views", NSStringFromClass([self class]), [self autosaveName]];
			[[NSUserDefaults standardUserDefaults] setObject:frames forKey:key];
	}
}



- (void)_loadSplitViewPosition {
	NSArray		*frames;
	NSString	*key;
	int			i, frameCount, subviewCount;
	
	key = [NSSWF:@"%@ %@ Views", NSStringFromClass([self class]), [self autosaveName]];
	frames = [[NSUserDefaults standardUserDefaults] arrayForKey:key];
	
	if(frames) {
		frameCount = [frames count];
		subviewCount = [[self subviews] count];
		
		for(i = 0; i < subviewCount && i < frameCount; i++)
			[[[self subviews] objectAtIndex:i] setFrame:NSRectFromString([frames objectAtIndex:i])];
	}
}

@end



@implementation WISplitView

- (id)initWithFrame:(NSRect)rect {
	if((self = [super initWithFrame:rect]))
		[self _initSplitView];
    
	return self;
}



- (id)initWithCoder:(NSCoder *)coder {
	if((self = [super initWithCoder:coder]))
		[self _initSplitView];
    
	return self;
}



- (void)dealloc; {
	[[NSNotificationCenter defaultCenter] removeObserver:self];

	[_autosaveName release];

	[super dealloc];
}



#pragma mark -

- (void)_WI_windowWillClose:(NSNotification *)notification {
	[self _saveSplitViewPosition];
}



- (void)_WI_applicationWillTerminate:(NSNotification *)notification {
	[self _saveSplitViewPosition];
}



#pragma mark

- (void)viewDidMoveToWindow {
	NSWindow	*window;
	
	window = [self window];
	
	if(window) {
		[[NSNotificationCenter defaultCenter]
			addObserver:self
			   selector:@selector(_WI_windowWillClose:)
				   name:NSWindowWillCloseNotification
				 object:window];
	}
}



#pragma mark -

- (void)setAutosaveName:(NSString *)value {
	[value retain];
	[_autosaveName release];
	_autosaveName = value;
	
	[self _loadSplitViewPosition];
}



- (NSString *)autosaveName;{
	return _autosaveName;
}

@end
