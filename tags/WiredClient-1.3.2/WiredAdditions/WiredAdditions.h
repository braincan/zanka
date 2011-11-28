/* $Id$ */

/*
 *  Copyright (c) 2003-2007 Axel Andersson
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

// --- 10.5 stuff while building on 10.4
#ifndef NSINTEGER_DEFINED
#if __LP64__ || NS_BUILD_32_LIKE_64
typedef long					NSInteger;
typedef unsigned long			NSUInteger;
#else
typedef int						NSInteger;
typedef unsigned int			NSUInteger;
#endif

#define NSIntegerMax			LONG_MAX
#define NSIntegerMin			LONG_MIN
#define NSUIntegerMax			ULONG_MAX

#define NSINTEGER_DEFINED		1
#endif

#ifndef CGFLOAT_DEFINED
#if defined(__LP64__) && __LP64__
typedef double					CGFloat;
#define CGFLOAT_MIN				DBL_MIN
#define CGFLOAT_MAX				DBL_MAX
#define CGFLOAT_IS_DOUBLE		1
#else
typedef float					CGFloat;
#define CGFLOAT_MIN				FLT_MIN
#define CGFLOAT_MAX				FLT_MAX
#define CGFLOAT_IS_DOUBLE		0
#endif

#define CGFLOAT_DEFINED			1
#endif

#ifndef MAC_OS_X_VERSION_10_5
#define MAC_OS_X_VERSION_10_5	1050
#endif

#import <WiredAdditions/WIFunctions.h>
#import <WiredAdditions/WIMacros.h>
#import <WiredAdditions/WITypes.h>
#import <WiredAdditions/WIObject.h>

#import <WiredAdditions/WIColorCell.h>
#import <WiredAdditions/WIGraphView.h>
#import <WiredAdditions/WIIconCell.h>
#import <WiredAdditions/WIImageViewWithImagePicker.h>
#import <WiredAdditions/WIMatrix.h>
#import <WiredAdditions/WIMultiImageCell.h>
#import <WiredAdditions/WIOutlineView.h>
#import <WiredAdditions/WIPDFView.h>
#import <WiredAdditions/WIProgressIndicator.h>
#import <WiredAdditions/WISplitView.h>
#import <WiredAdditions/WITableHeaderView.h>
#import <WiredAdditions/WITableView.h>
#import <WiredAdditions/WITextAttachment.h>
#import <WiredAdditions/WITextView.h>
#import <WiredAdditions/WIWindow.h>
#import <WiredAdditions/WIWindowController.h>

#import <WiredAdditions/WIApplication.h>
#import <WiredAdditions/WIDateFormatter.h>
#import <WiredAdditions/WIEventQueue.h>
#import <WiredAdditions/WIExceptionHandler.h>
#import <WiredAdditions/WISettings.h>
#import <WiredAdditions/WITextFilter.h>
#import <WiredAdditions/WIURL.h>

#import <WiredAdditions/NSBezierPath-WIAdditions.h>
#import <WiredAdditions/NSColor-WIAdditions.h>
#import <WiredAdditions/NSFont-WIAdditions.h>
#import <WiredAdditions/NSImage-WIAdditions.h>
#import <WiredAdditions/NSMenu-WIAdditions.h>
#import <WiredAdditions/NSMenuItem-WIAdditions.h>
#import <WiredAdditions/NSPopUpButton-WIAdditions.h>
#import <WiredAdditions/NSSound-WIAdditions.h>
#import <WiredAdditions/NSSplitView-WIAdditions.h>
#import <WiredAdditions/NSTabView-WIAdditions.h>
#import <WiredAdditions/NSTableView-WIAdditions.h>
#import <WiredAdditions/NSTextField-WIAdditions.h>
#import <WiredAdditions/NSTextView-WIAdditions.h>
#import <WiredAdditions/NSToolbar-WIAdditions.h>
#import <WiredAdditions/NSToolbarItem-WIAdditions.h>
#import <WiredAdditions/NSView-WIAdditions.h>
#import <WiredAdditions/NSWindow-WIAdditions.h>
#import <WiredAdditions/NSWindowController-WIAdditions.h>
#import <WiredAdditions/NSWorkspace-WIAdditions.h>

#import <WiredAdditions/NSApplication-WIAdditions.h>
#import <WiredAdditions/NSArray-WIAdditions.h>
#import <WiredAdditions/NSData-WIAdditions.h>
#import <WiredAdditions/NSDate-WIAdditions.h>
#import <WiredAdditions/NSDictionary-WIAdditions.h>
#import <WiredAdditions/NSError-WIAdditions.h>
#import <WiredAdditions/NSEvent-WIAdditions.h>
#import <WiredAdditions/NSFileManager-WIAdditions.h>
#import <WiredAdditions/NSInvocation-WIAdditions.h>
#import <WiredAdditions/NSNetService-WIAdditions.h>
#import <WiredAdditions/NSNotificationCenter-WIAdditions.h>
#import <WiredAdditions/NSNumber-WIAdditions.h>
#import <WiredAdditions/NSObject-WIAdditions.h>
#import <WiredAdditions/NSProcessInfo-WIAdditions.h>
#import <WiredAdditions/NSScanner-WIAdditions.h>
#import <WiredAdditions/NSSet-WIAdditions.h>
#import <WiredAdditions/NSString-WIAdditions.h>
#import <WiredAdditions/NSThread-WIAdditions.h>
