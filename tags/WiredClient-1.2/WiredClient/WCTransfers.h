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

#import "WCTransfer.h"
#import "WCWindowController.h"

@class WCTableView, WCFile, WCTransfer, WCConnection, WCTransfer;

@interface WCTransfers : WCWindowController {
	IBOutlet WCTableView				*_transfersTableView;
	
	IBOutlet NSTableColumn				*_iconTableColumn;
	IBOutlet NSTableColumn				*_infoTableColumn;
	
	IBOutlet NSButton					*_startButton;
	IBOutlet NSButton					*_stopButton;
	IBOutlet NSButton					*_removeButton;
	
	NSMutableArray						*_transfers;
	WCTransfer							*_transfer;
	int									_running;

	WCTransfer							*_recursiveDownload;
	WCTransfer							*_recursiveUpload;

	NSImage								*_folderImage, *_lockedImage, *_unlockedImage;
	NSTimer								*_timer;
	NSLock								*_lock;
}


#define WCTransfersShouldStartTransfer  @"WCTransfersShouldStartTransfer"
#define WCTransfersShouldUpdateQueue	@"WCTransfersShouldUpdateQueue"

#define WCChecksumLength				1048576

#define WCTransferPboardType			@"WCTransferPboardType"


- (id)									initWithConnection:(WCConnection *)connection;

- (void)								update;
- (void)								updateButtons;

- (WCTransfer *)						transferWithPath:(NSString *)path;
- (WCTransfer *)						transferWithState:(WCTransferState)state;
- (unsigned int)						transfersCount;
- (void)								removeTransfer:(WCTransfer *)transfer;

- (void)								download:(WCFile *)file preview:(BOOL)preview;
- (void)								upload:(NSString *)path withDestination:(WCFile *)destination;
- (void)								request:(WCTransfer *)transfer;

- (IBAction)							start:(id)sender;
- (IBAction)							stop:(id)sender;
- (IBAction)							remove:(id)sender;

@end
