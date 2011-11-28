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

#import <WiredAdditions/NSEvent-WIAdditions.h>
#import <WiredAdditions/NSImage-WIAdditions.h>
#import <WiredAdditions/WIImageViewWithImagePicker.h>

/*
 *     Generated by class-dump 3.1.1.
 *
 *     class-dump is Copyright (C) 1997-1998, 2000-2001, 2004-2006 by Steve Nygard.
 */

@interface NSIPRecentPicture : NSObject {
    NSString *_originalImageName;
    NSImage *_originalImage;
    NSRect _crop;
    NSData *_smallIconData;
}

+ (id)pictureDirPath;
+ (int)maxRecents;
+ (id)_infoFilePath;
+ (BOOL)purgeExtras;
+ (void)_saveChanges;
+ (id)recentPictures;
+ (id)recentSmallIcons;
+ (id)currentPicture;
+ (void)noCurrentPicture;
+ (void)removeAllButCurrent;
- (id)initWithOriginalImage:(id)fp8 crop:(NSRect)fp12 smallIcon:(id)fp28;
- (id)initWithOriginalImage:(id)fp8;
- (id)initWithInfo:(id)fp8;
- (void)dealloc;
- (void)finalize;
- (id)_infoToSave;
- (id)originalImagePath;
- (id)originalImage;
- (id)croppedImage;
- (id)smallIcon;
- (NSRect)crop;
- (void)setCrop:(NSRect)fp8 smallIcon:(id)fp24;
- (void)_removePermanently;
- (void)setCurrent;

@end



@interface NSImagePickerController : NSWindowController {
    id _imageView;
    id _layerSuperview;
    NSSlider *_slider;
    id _recentMenu;
    NSButton *_cameraButton;
    NSButton *_smallerButton;
    NSButton *_largerButton;
    NSButton *_chooseButton;
    NSButton *_setButton;
    NSTextField *_cameraLabel;
    NSTabView *_countdownTabView;
    id _countdownView;
    NSBox *_imagePickerViewBox;
    NSImage *_originalImage;
    NSSize _originalSize;
    NSSize _targetSize;
    NSSize _minSize;
    NSSize _maxSize;
    float _defaultSliderPos;
    NSIPRecentPicture *_recentPicture;
    BOOL _changed;
    BOOL _changesAccepted;
    BOOL _takingPicture;
    NSTimer *_pictureDelayTimer;
    id _target;
    SEL _action;
    void *_userInfo;
    id _delegate;
    float _maxWindowSize;
}

+ (id)sharedImagePickerControllerCreate:(BOOL)fp8 withTexturedWindow:(BOOL)fp12;
+ (id)sharedImagePickerControllerCreate:(BOOL)fp8;
+ (id)recentPicturesPopUp;
- (id)initAtPoint:(NSPoint)fp8 inWindow:(id)fp16 withTexturedWindow:(BOOL)fp20;
- (id)initAtPoint:(NSPoint)fp8 inWindow:(id)fp16;
- (void)setTarget:(id)fp8 selector:(SEL)fp12 userInfo:(void *)fp16;
- (id)image;
- (id)originalImage;
- (void)setMaxImageSize:(float)fp8;
- (float)maxImageSize;
- (void)setDelegate:(id)fp8;
- (void)selectionChanged;
- (void)hideRecentsPopUp;
- (void)setAcceptsDrags:(BOOL)fp8;

@end



@interface WIImageViewWithImagePicker(Private)

- (void)_showImagePicker;

@end



@implementation WIImageViewWithImagePicker(Private)

- (void)_initImageViewWithImagePicker {
	Class		pickerClass, recentPictureClass;
	
	pickerClass = NSClassFromString(@"NSImagePickerController");
	recentPictureClass = NSClassFromString(@"NSIPRecentPicture");
	
	_pickerIsAvailable = ([pickerClass respondsToSelector:@selector(sharedImagePickerControllerCreate:withTexturedWindow:)] &&
						  [pickerClass instancesRespondToSelector:@selector(window)] &&
						  [pickerClass instancesRespondToSelector:@selector(initAtPoint:inWindow:)] &&
						  [pickerClass instancesRespondToSelector:@selector(setDelegate:)] &&
						  [pickerClass instancesRespondToSelector:@selector(selectionChanged)] &&
						  [recentPictureClass instancesRespondToSelector:@selector(initWithOriginalImage:)] &&
						  [recentPictureClass instancesRespondToSelector:@selector(setCurrent)] &&
						  [recentPictureClass respondsToSelector:@selector(_saveChanges)]);
}



- (void)_showImagePicker {
	NSOpenPanel		*openPanel;
	NSImage			*image;
	NSData			*data;
	NSSize			size;
	NSPoint			point;
	
	if(_pickerIsAvailable) {
		if(!_picker) {
			_picker = [[NSClassFromString(@"NSImagePickerController") sharedImagePickerControllerCreate:YES withTexturedWindow:NO] retain];
			
			point = [NSEvent mouseLocation];
			point.y -= [[_picker window] frame].size.height;
			
			[_picker initAtPoint:point inWindow:NULL];
			[_picker setDelegate:self];
		}
		
		[_picker selectionChanged];
		[[_picker window] makeKeyAndOrderFront:NULL];
	} else {
		openPanel = [NSOpenPanel openPanel];
		
		if([openPanel runModalForDirectory:NULL file:NULL types:[NSImage imageFileTypes]] == NSOKButton) {
			data = [NSData dataWithContentsOfFile:[openPanel filename]];
			
			if(data)
				image = [NSImage imageWithData:data];
			else
				image = NULL;

			if(image) {
				size = [image size];
				
				if ((_maxImageSize.width > 0.0 && size.width > _maxImageSize.width) ||
					(_maxImageSize.height > 0.0 && size.height > _maxImageSize.height)) {
					image = [image scaledImageWithSize:_maxImageSize];
				}
			}
			
			[self setImage:image];
			
			[[self target] performSelector:[self action] withObject:self];
		}
	}
}

@end


@implementation WIImageViewWithImagePicker

- (id)initWithCoder:(NSCoder *)coder {
	self = [super initWithCoder:coder];
	
	[self _initImageViewWithImagePicker];
    
	return self;
}



- (id)initWithFrame:(NSRect)frame {
	self = [super initWithFrame:frame];
	
	[self _initImageViewWithImagePicker];

	return self;
}



- (void)dealloc {
	[_defaultImage release];
	
	[_picker release];
	
	[super dealloc];
}



#pragma mark -

- (void)setDefaultImage:(NSImage *)image {
	[image retain];
	[_defaultImage release];
	
	_defaultImage = image;
	
	if(![self image])
		[self setImage:_defaultImage];
}



- (NSImage *)defaultImage {
	return _defaultImage;
}



- (void)setMaxImageSize:(NSSize)maxImageSize {
	_maxImageSize = maxImageSize;
}



- (NSSize)maxImageSize {
	return _maxImageSize;
}



#pragma mark -

- (void)setImage:(NSImage *)image {
	if(image)
		[super setImage:image];
	else
		[super setImage:_defaultImage];
}



- (NSImage *)image {
	NSImage		*image;
	
	image = [super image];
	
	if(image == _defaultImage)
		return NULL;
	
	return image;
}



#pragma mark -

- (void)mouseDown:(NSEvent *)event {
	NSEvent		*nextEvent;
	
	if(![self isEnabled]) {
		[super mouseDown:event];
	} else {
		nextEvent = [[self window] nextEventMatchingMask:NSLeftMouseUpMask | NSLeftMouseDraggedMask | NSPeriodicMask
											   untilDate:[NSDate distantFuture]
												  inMode:NSEventTrackingRunLoopMode
												 dequeue:NO];
		
		if([nextEvent type] != NSLeftMouseDragged)
			[super mouseDown:event];
		
		if([event clickCount] == 2)
			[self _showImagePicker];
	}
}



- (void)keyDown:(NSEvent *)event {
	switch([event character]) {
		case NSEnterCharacter:
		case NSCarriageReturnCharacter:
			[self _showImagePicker];
			break;
		
		default:
			[super keyDown:event];
			break;
	}
}



#pragma mark -

- (void)imagePicker:(id)imagePicker selectedImage:(NSImage *)image {
	NSIPRecentPicture	*recentPicture;
	NSSize				size;
	
	size = [image size];
	
	if((_maxImageSize.width > 0.0 && size.width > _maxImageSize.width) ||
	   (_maxImageSize.height > 0.0 && size.height > _maxImageSize.height)) {
		image = [image scaledImageWithSize:_maxImageSize];
	}
	
	[self setImage:image];

	[[self target] performSelector:[self action] withObject:self];
	
	recentPicture = [[NSClassFromString(@"NSIPRecentPicture") alloc] initWithOriginalImage:image];
	[recentPicture setCurrent];
	[NSClassFromString(@"NSIPRecentPicture") _saveChanges];
	[recentPicture autorelease];
	
	[_picker release];
	_picker = NULL;
}



- (void)imagePickerCanceled:(id)imagePicker {
	[[_picker window] close];

	[_picker release];
	_picker = NULL;
}



- (NSImage *)displayImageInPicker:(id)imagePicker {
	return [self image];
}

@end