/* $Id$ */

/*
 *  Copyright (c) 2003-2009 Axel Andersson
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

enum _WCFileType {
	WCFileFile,
	WCFileDirectory,
	WCFileUploads,
	WCFileDropBox
};
typedef enum _WCFileType	WCFileType;


@interface WCFile : WIObject <NSCoding> {
	WCFileType				_type;
	WIFileOffset			_size;
	WIFileOffset			_free;
	NSString				*_path;
	NSDate					*_creationDate;
	NSString				*_creationDateString;
	NSDate					*_modificationDate;
	NSString				*_modificationDateString;
	NSString				*_checksum;
	NSString				*_comment;

	NSString				*_name;
	NSString				*_extension;
	NSString				*_kind;

@public
	WIFileOffset			_offset;
	WIFileOffset			_transferred;
}


+ (NSImage *)iconForFolderType:(WCFileType)type width:(double)width;
+ (NSString *)kindForFolderType:(WCFileType)type;
+ (WCFileType)folderTypeForString:(NSString *)string;

+ (id)fileWithRootDirectory;
+ (id)fileWithDirectory:(NSString *)path;
+ (id)fileWithPath:(NSString *)path;
+ (id)fileWithPath:(NSString *)path type:(WCFileType)type;
+ (id)fileWithListArguments:(NSArray *)arguments;
+ (id)fileWithInfoArguments:(NSArray *)arguments;

- (WCFileType)type;
- (NSString *)path;
- (NSDate *)creationDate;
- (NSDate *)modificationDate;
- (NSString *)checksum;
- (NSString *)comment;

- (NSString *)name;
- (NSString *)extension;
- (NSString *)kind;
- (BOOL)isFolder;
- (BOOL)isUploadsFolder;
- (NSImage *)iconWithWidth:(double)width;

- (void)setSize:(WIFileOffset)size;
- (WIFileOffset)size;
- (void)setOffset:(WIFileOffset)offset;
- (WIFileOffset)offset;
- (void)setTransferred:(WIFileOffset)transferred;
- (WIFileOffset)transferred;
- (void)setFree:(WIFileOffset)free;
- (WIFileOffset)free;

- (NSComparisonResult)compareName:(WCFile *)file;
- (NSComparisonResult)compareKind:(WCFile *)file;
- (NSComparisonResult)compareCreationDate:(WCFile *)file;
- (NSComparisonResult)compareModificationDate:(WCFile *)file;
- (NSComparisonResult)compareSize:(WCFile *)file;

@end
