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

@class WCConnection;

@interface WCError : NSObject {
	WCConnection							*_connection;
	
	NSLock									*_lock;
	
	int										_error;
}


enum WCErrorType {
	WCUndefinedError						 = -1,
	WCNoError								 = 0,
	
	WCConnectionErrorConnectFailed			 = 1,
	WCConnectionErrorResolveFailed,
	WCConnectionErrorServerDisconnected,
	WCConnectionErrorReadFailed,
	WCConnectionErrorWriteFailed,
	WCConnectionErrorSSLConnectFailed,
	WCConnectionErrorSSLFailed,
	WCConnectionErrorLoginTimeOut,

	WCServerErrorCommandFailed,
	WCServerErrorCommandNotRecognized,
	WCServerErrorCommandNotImplemented,
	WCServerErrorSyntaxError,
	WCServerErrorLoginFailed,
	WCServerErrorBanned,
	WCServerErrorClientNotFound,
	WCServerErrorAccountNotFound,
	WCServerErrorAccountExists,
	WCServerErrorCannotBeDisconnected,
	WCServerErrorPermissionDenied,
	WCServerErrorFileNotFound,
	WCServerErrorFileExists,
	WCServerErrorChecksumMismatch,
	WCServerErrorQueueLimitExceeded,
	
	WCApplicationErrorOpenFailed,
	WCApplicationErrorCreateFailed,
	WCApplicationErrorFileNotFound,
	WCApplicationErrorFolderNotFound,
	WCApplicationErrorFileExists,
	WCApplicationErrorFolderExists,
	WCApplicationErrorChecksumMismatch,
	WCApplicationErrorTransferExists,
	WCApplicationErrorTransferFailed,
	WCApplicationErrorCannotDownload,
	WCApplicationErrorCannotUpload,
	WCApplicationErrorCannotUploadAnywhere,
	WCApplicationErrorCannotCreateFolders,
	WCApplicationErrorCannotDeleteFiles,
	WCApplicationErrorCannotDeleteFolders,
	WCApplicationErrorCannotEditAccounts,
	WCApplicationErrorProtocolMismatch
	
};
typedef enum WCErrorType					WCErrorType;


- (id)										initWithConnection:(WCConnection *)connection;

- (void)									setError:(WCErrorType)error;
- (void)									raiseError;
- (void)									raiseErrorInWindow:(NSWindow *)window;
- (void)									raiseErrorWithArgument:(NSString *)argument;
- (void)									raiseErrorInWindow:(NSWindow *)window withArgument:(NSString *)argument;

@end
