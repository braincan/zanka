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

#import <sys/types.h>
#import <sys/socket.h>
#import <netinet/in.h>
#import <arpa/inet.h>
#import "NSNumberAdditions.h"
#import "NSTableViewAdditions.h"
#import "NSURLAdditions.h"
#import "WCClient.h"
#import "WCConnection.h"
#import "WCIconCell.h"
#import "WCMain.h"
#import "WCPreferences.h"
#import "WCSecureSocket.h"
#import "WCSettings.h"
#import "WCTracker.h"
#import "WCTrackers.h"
	 
@implementation WCTrackers

- (id)init {
	self = [super initWithWindowNibName:@"Trackers"];

	// --- initiate our array of trackers
	_trackers = [[NSMutableArray alloc] init];
	
	// --- initiate our rendezvous tracker
	_rendezvousTracker = [[WCTracker alloc] initWithType:WCTrackerTypeRendezvous];
	[_rendezvousTracker setName:@"Rendezvous"];
	
	// --- get the icons
	_rendezvousImage = [[NSImage imageNamed:@"Rendezvous"] retain];

	// --- initiate our rendezvous browser
	_browser = [[NSNetServiceBrowser alloc] init];
	
	// --- start the browser
	[_browser setDelegate:self];
	[_browser searchForServicesOfType:@"_wired._tcp." inDomain:@""];
	
	// --- load the window
	[self window];
	
	// --- subscribe to these
	[[NSNotificationCenter defaultCenter]
		addObserver:self
		   selector:@selector(connectionHasClosed:)
			   name:WCConnectionHasClosed
			 object:NULL];

	[[NSNotificationCenter defaultCenter]
		addObserver:self
		   selector:@selector(connectionShouldTerminate:)
			   name:WCConnectionShouldTerminate
			 object:NULL];

	[[NSNotificationCenter defaultCenter]
		addObserver:self
		   selector:@selector(trackersShouldAddCategory:)
			   name:WCTrackersShouldAddCategory
			 object:NULL];
	
	[[NSNotificationCenter defaultCenter]
		addObserver:self
		   selector:@selector(trackersShouldAddServer:)
			   name:WCTrackersShouldAddServer
			 object:NULL];
	
	[[NSNotificationCenter defaultCenter]
		addObserver:self
		   selector:@selector(trackersShouldCompleteServers:)
			   name:WCTrackersShouldCompleteServers
			 object:NULL];

	return self;
}



- (void)dealloc {
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	
	[_rendezvousImage release];
	
	[_sortUpImage release];
	[_sortDownImage release];
	
	[_trackers release];
	[_browser release];
	[_rendezvousTracker release];
	
	[super dealloc];
}



#pragma mark -

- (void)windowDidLoad {
	WCIconCell		*iconCell;
	
	// --- set up our custom cell type for the transfer text
	iconCell = [[WCIconCell alloc] initWithImageWidth:16 whitespace:NO];
	[[_trackersOutlineView tableColumnWithIdentifier:@"Name"] setDataCell:iconCell];
	[iconCell release];
	
	// --- add all our trackers from preferences
	[self updateTrackers];
	
	// --- get the sort images
	_sortUpImage	= [[NSImage imageNamed:@"SortUp"] retain];
	_sortDownImage	= [[NSImage imageNamed:@"SortDown"] retain];
	
	// --- connect on double-click
	[_trackersOutlineView setDoubleAction:@selector(open:)];
	
	// --- always show rendezvous tracker
	[_trackersOutlineView expandItem:_rendezvousTracker];

	// --- window position
	[self setShouldCascadeWindows:NO];
	[self setWindowFrameAutosaveName:@"Trackers"];

	// --- table view position
	[_trackersOutlineView setAutosaveName:@"Trackers"];
	[_trackersOutlineView setAutosaveTableColumns:YES];
	
	// --- table columns
	[_trackersOutlineView updateViewOptions];

	// --- simulate a click on the name column to sort by name
	[self outlineView:_trackersOutlineView shouldSelectTableColumn:
		[_trackersOutlineView tableColumnWithIdentifier:@"Name"]];

	// --- start off in the table view
	[[self window] makeFirstResponder:_trackersOutlineView];
}



- (void)connectionShouldTerminate:(NSNotification *)notification {
	NSEnumerator	*enumerator;
	WCTracker		*each;
	
	// --- reset states
	enumerator = [_trackers objectEnumerator];
	
	while((each = [enumerator nextObject]))
		[each setState:WCTrackerStateIdle];
	
	// --- stop spinning
	[_progressIndicator performSelectorOnMainThread:@selector(stopAnimation:)
										 withObject:self
									  waitUntilDone:YES];
}



- (void)connectionHasClosed:(NSNotification *)notification {
	NSEnumerator	*enumerator;
	WCTracker		*each;
	
	// --- reset states
	enumerator = [_trackers objectEnumerator];
	
	while((each = [enumerator nextObject]))
		[each setState:WCTrackerStateIdle];
	
	// --- stop spinning
	[_progressIndicator performSelectorOnMainThread:@selector(stopAnimation:)
										 withObject:self
									  waitUntilDone:YES];
}



- (void)trackersShouldAddCategory:(NSNotification *)notification {
	NSEnumerator	*enumerator, *categoryEnumerator;
	NSArray			*components;
	NSString		*argument, *name, *eachName;
	WCConnection	*connection;
	WCTracker		*tracker, *category, *eachCategory;
	
	// --- get parameters
	connection		= [[notification object] objectAtIndex:0];
	argument		= [[notification object] objectAtIndex:1];
	
	// --- add it under the root tracker by default
	tracker = [connection tracker];

	// --- separate categories
	components = [argument componentsSeparatedByString:@"/"];
	name = [components lastObject];
	components = [components subarrayWithRange:NSMakeRange(0, [components count] - 1)];
	
	if([components count] > 0) {
		// --- now loop over every component in foo/bar/baz
		enumerator = [components objectEnumerator];
		
		while((eachName = [enumerator nextObject])) {
			// --- find the category with that name
			categoryEnumerator = [[tracker children] objectEnumerator];
			tracker = NULL;
			
			while((eachCategory = [categoryEnumerator nextObject])) {
				if([eachCategory type] == WCTrackerTypeCategory &&
				   [[eachCategory name] isEqualToString:eachName]) {
					tracker = eachCategory;
					
					break;
				}
			}
		}
	}
	
	// --- create new category
	category = [[WCTracker alloc] initWithType:WCTrackerTypeCategory];
	[category setName:name];
	[tracker addChild:category];
	[category release];
}



- (void)trackersShouldAddServer:(NSNotification *)notification {
	NSEnumerator		*enumerator, *categoryEnumerator;
	NSScanner			*scanner;
	NSString			*argument, *eachName, *category, *name, *url, *users, *speed, *guest, *download, *files, *size, *description;
	NSArray				*fields, *components;
	WCConnection		*connection;
	WCTracker			*server, *tracker, *eachCategory;
	unsigned long long  size_l;
	
	// --- get parameters
	connection		= [[notification object] objectAtIndex:0];
	argument		= [[notification object] objectAtIndex:1];
	
	// --- separate the fields
	fields			= [argument componentsSeparatedByString:WCFieldSeparator];
	category		= [fields objectAtIndex:0];
	url				= [fields objectAtIndex:1];
	name			= [fields objectAtIndex:2];
	users			= [fields objectAtIndex:3];
	speed			= [fields objectAtIndex:4];
	guest			= [fields objectAtIndex:5];
	download		= [fields objectAtIndex:6];
	files			= [fields objectAtIndex:7];
	size			= [fields objectAtIndex:8];
	description		= [fields objectAtIndex:9];
	
	// --- add it under the root tracker by default
	tracker = [connection tracker];
	
	if([category length] > 0) {
		// --- separate categories
		components = [category componentsSeparatedByString:@"/"];

		// --- now loop over every component in foo/bar/baz
		enumerator = [components objectEnumerator];
		
		while((eachName = [enumerator nextObject])) {
			// --- find the category with that name
			categoryEnumerator = [[tracker children] objectEnumerator];
			tracker = NULL;
			
			while((eachCategory = [categoryEnumerator nextObject])) {
				if([eachCategory type] == WCTrackerTypeCategory &&
				   [[eachCategory name] isEqualToString:eachName]) {
					tracker = eachCategory;
					
					break;
				}
			}
		}
	}
	
	// --- scan size
	scanner = [NSScanner scannerWithString:size];
	[scanner scanLongLong:&size_l];
	
	// --- create server
	server = [[WCTracker alloc] initWithType:WCTrackerTypeServer];
	[server setName:name];
	[server setURL:[NSURL URLWithString:url]];
	[server setUsers:[users intValue]];
	[server setSpeed:[speed intValue]];
	[server setGuest:([guest intValue] == 1)];
	[server setDownload:([download intValue] == 1)];
	[server setFiles:[files intValue]];
	[server setSize:size_l];
	[server setDescription:description];
	[tracker addChild:server];
	[server release];
}



- (void)trackersShouldCompleteServers:(NSNotification *)notification {
	NSString		*argument, *identifier;
	WCConnection	*connection;
	
	// --- get parameters
	connection		= [[notification object] objectAtIndex:0];
	argument		= [[notification object] objectAtIndex:1];
	
	// --- get identifier
	identifier = [_lastTableColumn identifier];
	
	// --- sort children
	if([identifier isEqualToString:@"Name"])
		[[[connection tracker] children] sortUsingSelector:@selector(nameSort:)];
	else if([identifier isEqualToString:@"Users"])
		[[[connection tracker] children] sortUsingSelector:@selector(usersSort:)];
	else if([identifier isEqualToString:@"Speed"])
		[[[connection tracker] children] sortUsingSelector:@selector(speedSort:)];
	else if([identifier isEqualToString:@"Guest"])
		[[[connection tracker] children] sortUsingSelector:@selector(guestSort:)];
	else if([identifier isEqualToString:@"Download"])
		[[[connection tracker] children] sortUsingSelector:@selector(downloadSort:)];
	else if([identifier isEqualToString:@"Files"])
		[[[connection tracker] children] sortUsingSelector:@selector(filesSort:)];
	else if([identifier isEqualToString:@"Size"])
		[[[connection tracker] children] sortUsingSelector:@selector(sizeSort:)];
	else if([identifier isEqualToString:@"Description"])
		[[[connection tracker] children] sortUsingSelector:@selector(descriptionSort:)];

	// --- set done
	[[connection tracker] setState:WCTrackerStateDone];
	
	// --- open tab
	[_trackersOutlineView expandItem:[connection tracker]];
	
	// --- refresh
	[self updateStatus];
	
	// --- shut down connection when we've gotten everything
	[[NSNotificationCenter defaultCenter]
		postNotificationName:WCConnectionShouldTerminate
		object:connection];
}



#pragma mark -

- (void)updateTrackers {
	NSEnumerator		*enumerator;
	NSDictionary		*each;
	WCTracker			*tracker;

	// --- clear all
	[_trackers removeAllObjects];
	
	// --- add rendezvous at top
	[_trackers addObject:_rendezvousTracker];
	
	// --- then add from bookmarks
	enumerator = [[WCSettings objectForKey:WCTrackerBookmarks] objectEnumerator];
	
	while((each = [enumerator nextObject])) {
		tracker = [[WCTracker alloc] initWithType:WCTrackerTypeTracker];
		[tracker setName:[each objectForKey:@"Name"]];
		[tracker setURL:[NSURL URLWithString:[NSString stringWithFormat:
			@"wiredtracker://%@/", [each objectForKey:@"Address"]]]];
		
		[_trackers addObject:tracker];
		[tracker release];
	}

	// --- reload table
	[_trackersOutlineView reloadData];
	[self updateStatus];
}



- (void)updateStatus {
	WCTracker	*tracker;
	int			row, count;
	
	// --- get row
	row = [_trackersOutlineView selectedRow];

	if(row < 0) {
		[_statusTextField setStringValue:@""];
		
		return;
	}
	
	// --- get tracker
	tracker = [_trackersOutlineView itemAtRow:row];
	count	= [tracker servers];
	
	// --- display status
	switch([tracker type]) {
		case WCTrackerTypeRendezvous:
			[_statusTextField setStringValue:[NSString stringWithFormat:
				NSLocalizedString(@"Rendezvous local service discovery %C %d %@", @"Description of Rendezvous tracker (servers, 'server(s)'"),
				0x2014,
				count,
				count == 1
					? NSLocalizedString(@"server", @"Server singular")
					: NSLocalizedString(@"servers", @"Server plural")]];
			break;
			
		case WCTrackerTypeTracker:
			[_statusTextField setStringValue:[NSString stringWithFormat:
				@"%@ %C %d %@",
				[tracker name],
				0x2014,
				count,
				count == 1
					? NSLocalizedString(@"server", @"Server singular")
					: NSLocalizedString(@"servers", @"Server plural")]];
			break;
			
		case WCTrackerTypeCategory:
			[_statusTextField setStringValue:[NSString stringWithFormat:
				@"%@ %C %d %@",
				[tracker name],
				0x2014,
				count,
				count == 1
					? NSLocalizedString(@"server", @"Server singular")
					: NSLocalizedString(@"servers", @"Server plural")]];
			break;

		case WCTrackerTypeServer:
			[_statusTextField setStringValue:[NSString stringWithFormat:
				@"%@ %C %@",
				[tracker name],
				0x2014,
				[tracker URL]
					? [[tracker URL] humanReadable]
					: NSLocalizedString(@"Local server via Rendezvous", @"Description of server via Rendezvous tracker")]];
			break;
	}
}



#pragma mark -

- (IBAction)open:(id)sender {
	NSURL			*url = NULL;
	WCTracker		*server;
	int				row;
	
	// --- get row
	row = [_trackersOutlineView selectedRow];

	if(row < 0)
		return;
	
	// --- get server
	server = [_trackersOutlineView itemAtRow:row];
	
	if([server URL]) {
		url = [server URL];
	}
	else if([server service]) {
		NSArray				*addresses;
		NSString			*ip;
		struct sockaddr_in	*addr;
		int					port;
	
		// --- get address
		addresses = [[server service] addresses];

		if([addresses count] == 0)
			return;
	
		// --- extract URL
		addr	= (struct sockaddr_in *) [[addresses objectAtIndex:0] bytes];
		ip		= [NSString stringWithCString:(char *) inet_ntoa(addr->sin_addr)];
		port	= ntohs(addr->sin_port);
		url		= [NSURL URLWithString:[NSString stringWithFormat:
					@"wired://%@:%d/", ip, port]];
	}

	// --- connect
	if(url && [[url scheme] isEqualToString:@"wired"]) {
		[WCSharedMain showConnect:url];
		
		if(([[NSApp currentEvent] modifierFlags] & NSAlternateKeyMask) == 0)
			[WCSharedMain connectWithWindow:self];
	}
}



- (IBAction)search:(id)sender {
	_filter = [_searchTextField stringValue];
	
	[_trackersOutlineView reloadData];
}



#pragma mark -

- (NSPanel *)viewOptionsPanel {
	return _viewOptionsPanel;
}




- (NSTableView *)tableView {
	return _trackersOutlineView;
}



#pragma mark -

- (void)netServiceBrowser:(NSNetServiceBrowser *)netServiceBrowser didFindService:(NSNetService *)netService moreComing:(BOOL)moreComing {
	WCTracker		*server;

	// --- resolve service
	[netService setDelegate:self];
	[netService resolve];
	
	// --- create server record
	server = [[WCTracker alloc] initWithType:WCTrackerTypeServer];
	[server setName:[netService name]];
	[server setService:netService];
	[_rendezvousTracker addChild:server];
	[server release];
	
	// --- show if last
	if(!moreComing) {
		[_trackersOutlineView reloadData];
		[self updateStatus];
	}
}



- (void)netServiceBrowser:(NSNetServiceBrowser *)netServiceBrowser didRemoveService:(NSNetService *)netService moreComing:(BOOL)moreComing {
	NSEnumerator	*enumerator;
	WCTracker		*each;

	// --- loop over all services and compare the fields
	enumerator = [[_rendezvousTracker children] objectEnumerator];
	
	while((each = [enumerator nextObject])) {
		if([[[each service] name] isEqualToString:[netService name]] &&
		   [[[each service] type] isEqualToString:[netService type]] &&
		   [[[each service] domain] isEqualToString:[netService domain]]) {
			[_rendezvousTracker removeChild:each];
			
			break;
		}
	}
	
	// --- show if last
	if(!moreComing) {
		[_trackersOutlineView reloadData];
		[self updateStatus];
	}
}



- (void)netServiceDidResolveAddress:(NSNetService *)sender {
	// --- stop resolving
	[sender stop];
}



#pragma mark -

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldExpandItem:(WCTracker *)item {
	WCConnection		*connection;
	
	// --- let through non-trackers
	if([item type] != WCTrackerTypeTracker)
		return YES;
	
	// --- we've gotten everything and can finally open up
	if([item state] == WCTrackerStateDone)
		return YES;
	
	// --- this is to prevent us from forking two threads for the same item
	if([item state] != WCTrackerStateIdle)
		return NO;

	// --- clear set
	[[item children] removeAllObjects];
	
	// --- prevent us from forking two threads for the same item
	[item setState:WCTrackerStateExpanding];

	// --- initiate a socket and a controller for it
	connection = [[WCConnection alloc] initTrackerConnectionWithURL:[item URL] tracker:item];
	
	// --- start spinning
	[_progressIndicator startAnimation:self];
	
	return NO;
}



- (BOOL)outlineView:(NSOutlineView *)outlineView shouldSelectTableColumn:(NSTableColumn *)tableColumn {
	NSEnumerator	*enumerator;
	NSImage			*sortImage;
	NSString		*identifier;
	WCTracker		*tracker;
	
	if(_lastTableColumn == tableColumn) {
		// --- invert sorting
		_sortDescending = !_sortDescending;
	} else {
		_sortDescending = NO;
		
		if(_lastTableColumn)
			[_trackersOutlineView setIndicatorImage:NULL inTableColumn:_lastTableColumn];
		
		// --- get identifier
		identifier = [tableColumn identifier];
		
		// --- loop over trackers
		enumerator = [_trackers objectEnumerator];
		
		if([identifier isEqualToString:@"Name"]) {
			while((tracker = [enumerator nextObject]))
				[[tracker children] sortUsingSelector:@selector(nameSort:)];
		}
		else if([identifier isEqualToString:@"Users"]) {
			while((tracker = [enumerator nextObject]))
				[[tracker children] sortUsingSelector:@selector(usersSort:)];
		}
		else if([identifier isEqualToString:@"Speed"]) {
			while((tracker = [enumerator nextObject]))
				[[tracker children] sortUsingSelector:@selector(speedSort:)];
		}
		else if([identifier isEqualToString:@"Guest"]) {
			while((tracker = [enumerator nextObject]))
				[[tracker children] sortUsingSelector:@selector(guestSort:)];
		}
		else if([identifier isEqualToString:@"Download"]) {
			while((tracker = [enumerator nextObject]))
				[[tracker children] sortUsingSelector:@selector(downloadSort:)];
		}
		else if([identifier isEqualToString:@"Files"]) {
			while((tracker = [enumerator nextObject]))
				[[tracker children] sortUsingSelector:@selector(filesSort:)];
		}
		else if([identifier isEqualToString:@"Size"]) {
			while((tracker = [enumerator nextObject]))
				[[tracker children] sortUsingSelector:@selector(sizeSort:)];
		}
		else if([identifier isEqualToString:@"Description"]) {
			while((tracker = [enumerator nextObject]))
				[[tracker children] sortUsingSelector:@selector(descriptionSort:)];
		}

		// --- set the new sorting selector
		_lastTableColumn = tableColumn;
		
		[_trackersOutlineView setHighlightedTableColumn:tableColumn];
	}
	
	// --- set the image for the new column header
	sortImage = _sortDescending ? _sortDownImage : _sortUpImage;
	[_trackersOutlineView setIndicatorImage:sortImage inTableColumn:tableColumn];
	[_trackersOutlineView reloadData];

	return NO;
}



- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(WCTracker *)item {	
	if([item type] != WCTrackerTypeServer)
		return YES;

	return NO;
}



- (int)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(WCTracker *)item {
	if(!item)
		return [_trackers count];

	return [[item filteredChildren:_filter] count];
}



- (id)outlineView:(NSOutlineView *)outlineView child:(int)index ofItem:(WCTracker *)item {
	NSArray		*children;

	if(!item)
		return [_trackers objectAtIndex:index];
	
	children = [item filteredChildren:_filter];
	index = _sortDescending ? (int) [children count] - index - 1 : index;
	
	return [children objectAtIndex:index];
}



- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(WCTracker *)item {
	NSString	*identifier;
	
	// --- get identifier
	identifier = [tableColumn identifier];

	if([identifier isEqualToString:@"Name"]) {
		NSString	*string;
		NSImage		*image = NULL;
		
		if([item type] == WCTrackerTypeRendezvous)
			image = _rendezvousImage;

		if([item type] == WCTrackerTypeCategory)
			string = NSLocalizedStringFromTable([item name], @"Categories", @"Categories");
		else 
			string = [item name];
		
		if([item type] == WCTrackerTypeCategory || [item type] == WCTrackerTypeTracker)
			string = [NSString stringWithFormat:@"%@ (%u)", string, [item servers]];

		return [NSArray arrayWithObjects:string, image, NULL];
	}
	else if([item type] == WCTrackerTypeServer) {
		if([identifier isEqualToString:@"Description"]) {
			return [item description];
		}
		else if([item URL]) {
			if([identifier isEqualToString:@"Users"]) {
				return [NSNumber numberWithInt:[item users]];
			}
			else if([identifier isEqualToString:@"Speed"]) {
				return [[NSNumber numberWithUnsignedInt:[item speed]] bandwidth];
			}
			else if([identifier isEqualToString:@"Guest"]) {
				return [item guest]
					? NSLocalizedString(@"Yes", @"Yes")
					: NSLocalizedString(@"No", @"No");
			}
			else if([identifier isEqualToString:@"Download"]) {
				return [item download]
					? NSLocalizedString(@"Yes", @"Yes")
					: NSLocalizedString(@"No", @"No");
			}
			else if([identifier isEqualToString:@"Files"]) {
				return [NSNumber numberWithInt:[item files]];
			}
			else if([identifier isEqualToString:@"Size"]) {
				return [[NSNumber numberWithUnsignedLongLong:[item size]] humanReadableSize];
			}
		}
	}
	
	return NULL;
}



- (void)outlineViewSelectionDidChange:(NSNotification *)notification {
	// --- update status field
	[self updateStatus];
}



- (void)outlineViewShouldCopyInfo:(NSTableView *)tableView {
	NSPasteboard	*pasteboard;
	NSString		*string;
	WCTracker		*server;
	int				row;
	
	// --- get row
	row = [_trackersOutlineView selectedRow];
	
	if(row < 0)
		return;
	
	// --- get server
	server = [_trackersOutlineView itemAtRow:row];
	
	// --- create status string
	if([server URL])
		string = [[server URL] absoluteString];
	else
		string = [server name];
	
	// --- put it on the pasteboard
	pasteboard = [NSPasteboard generalPasteboard];
	[pasteboard declareTypes:[NSArray arrayWithObjects:NSStringPboardType, NULL] owner:NULL];
	[pasteboard setString:string forType:NSStringPboardType];
}

@end
