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

#import <WiredAdditions/NSNumber-WIAdditions.h>
#import <WiredAdditions/WISettings.h>

static id			_WISettingsSharedSettings;


@interface WISettings(Private)

+ (id)_settings;
+ (id)_settingsWithIdentifier:(NSString *)identifier;
- (id)_initWithIdentifier:(NSString *)identifier;

- (void)_setObject:(id)object forKey:(id)key;
- (id)_objectForKey:(id)key;

- (void)_synchronize;

@end


@implementation WISettings(Private)

+ (id)_settings {
	return [self _settingsWithIdentifier:NULL];
}



+ (id)_settingsWithIdentifier:(NSString *)identifier {
	if(!_WISettingsSharedSettings)
		[self loadWithIdentifier:identifier];
	
	return _WISettingsSharedSettings;
}



- (id)_initWithIdentifier:(NSString *)identifier {
	NSDictionary	*defaultValues;
	NSString		*key;
	NSEnumerator	*enumerator;
	BOOL			synchronize = NO;
	id				object;

	self = [super init];

	if(identifier) {
		_identifier = [identifier retain];
		_defaults = [[[NSUserDefaults standardUserDefaults] persistentDomainForName:_identifier] mutableCopy];

		if(!_defaults)
			_defaults = [[NSMutableDictionary alloc] init];
	} else {
		_defaults	= [NSUserDefaults standardUserDefaults];
	}
	
	defaultValues	= [[self class] defaults];
	enumerator		= [[defaultValues allKeys] objectEnumerator];

	while((key = [enumerator nextObject])) {
		object = [_defaults objectForKey:key];

		if(!object) {
			object = [defaultValues objectForKey:key];
			
			[_defaults setObject:object forKey:key];
			
			synchronize = YES;
		}
	}
	
	if(synchronize)
		[self _synchronize];

	return self;
}



#pragma mark -

- (void)_setObject:(id)object forKey:(id)key {
	[_defaults setObject:object forKey:key];

	[self performSelectorOnce:@selector(_synchronize) withObject:NULL afterDelay:1.0];
}



- (id)_objectForKey:(id)key {
	return [_defaults objectForKey:key];
}



#pragma mark -

- (void)_synchronize {
	NSUserDefaults		*defaults;
	
	defaults = [NSUserDefaults standardUserDefaults];
	
	if(_identifier) {
		[defaults removePersistentDomainForName:_identifier];
		[defaults setPersistentDomain:_defaults forName:_identifier];
	}
	
	[defaults synchronize];
}

@end



@implementation WISettings

+ (void)loadWithIdentifier:(NSString *)identifier {
	if(!_WISettingsSharedSettings)
		_WISettingsSharedSettings = [[self alloc] _initWithIdentifier:identifier];
}



+ (NSDictionary *)defaults {
	return [NSDictionary dictionary];
}



#pragma mark -

+ (void)setObject:(id)object forKey:(id)key {
	[[self _settings] _setObject:object forKey:key];
}



+ (id)objectForKey:(id)key {
	return [[self _settings] _objectForKey:key];
}



+ (void)setString:(NSString *)object forKey:(id)key {
	[[self _settings] _setObject:object forKey:key];
}



+ (NSString *)stringForKey:(id)key {
	return [[self _settings] _objectForKey:key];
}



+ (void)setBool:(BOOL)value forKey:(id)key {
	[[self _settings] _setObject:[NSNumber numberWithBool:value] forKey:key];
}



+ (BOOL)boolForKey:(id)key {
	return [[[self _settings] _objectForKey:key] boolValue];
}



+ (void)setInt:(int)value forKey:(id)key {
	[[self _settings] _setObject:[NSNumber numberWithInt:value] forKey:key];
}



+ (int)intForKey:(id)key {
	return [[[self _settings] _objectForKey:key] intValue];
}



+ (void)setInteger:(NSInteger)value forKey:(id)key {
	[[self _settings] _setObject:[NSNumber numberWithInteger:value] forKey:key];
}



+ (NSInteger)integerForKey:(id)key {
	return [[[self _settings] _objectForKey:key] integerValue];
}



+ (void)setFloat:(float)value forKey:(id)key {
	[[self _settings] _setObject:[NSNumber numberWithFloat:value] forKey:key];
}



+ (float)floatForKey:(id)key {
	return [[[self _settings] _objectForKey:key] floatValue];
}



+ (void)setDouble:(double)value forKey:(id)key {
	[[self _settings] _setObject:[NSNumber numberWithDouble:value] forKey:key];
}



+ (double)doubleForKey:(id)key {
	return [[[self _settings] _objectForKey:key] doubleValue];
}

@end
