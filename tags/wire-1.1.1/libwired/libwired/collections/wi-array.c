/* $Id$ */

/*
 *  Copyright (c) 2005-2006 Axel Andersson
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

#include "config.h"

#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include <wired/wi-array.h>
#include <wired/wi-assert.h>
#include <wired/wi-compat.h>
#include <wired/wi-list.h>
#include <wired/wi-lock.h>
#include <wired/wi-log.h>
#include <wired/wi-runtime.h>
#include <wired/wi-string.h>
#include <wired/wi-system.h>

#include "wi-private.h"

#define _WI_ARRAY_MIN_COUNT				10
#define _WI_ARRAY_MAX_COUNT				16777213

#define _WI_ARRAY_CHECK_OPTIMIZE(array)							\
	WI_STMT_START												\
		if((array->items_count >= 3 * array->data_count &&		\
			array->items_count >  array->min_count) ||			\
		   (array->data_count  >= 3 * array->items_count &&		\
			array->items_count <  _WI_ARRAY_MAX_COUNT))			\
			_wi_array_optimize(array);							\
    WI_STMT_END

#define _WI_ARRAY_RETAIN(array, data)							\
	((array)->callbacks.retain									\
		? (*(array)->callbacks.retain)((data))					\
		: (data))

#define _WI_ARRAY_RELEASE(array, data)							\
	WI_STMT_START												\
		if((array)->callbacks.release)							\
			(*(array)->callbacks.release)((data));				\
	WI_STMT_END

#define _WI_ARRAY_IS_EQUAL(array, data1, data2)					\
	(((array)->callbacks.is_equal &&							\
	  (*(array)->callbacks.is_equal)((data1), (data2))) ||		\
	 (!(array)->callbacks.is_equal &&							\
	  (data1) == (data2)))

#define _WI_ARRAY_INDEX_ASSERT(array, index)					\
	WI_ASSERT((index) < (array)->data_count,					\
		"index %d out of range (count %d) in %@",				\
		(index), (array)->data_count, (array))


struct _wi_array_item {
	void								*data;

	struct _wi_array_item				*link;
};
typedef struct _wi_array_item			_wi_array_item_t;

struct _wi_array {
	wi_runtime_base_t					base;
	
	wi_array_callbacks_t				callbacks;
	
	_wi_array_item_t					**items;
	uint32_t							items_count;
	uint32_t							min_count;
	uint32_t							data_count;

	wi_rwlock_t							*lock;

	_wi_array_item_t					**item_chunks;
	uint32_t							item_chunks_count;
	uint32_t							item_chunks_offset;

	_wi_array_item_t					*item_free_list;
};


static void								_wi_array_dealloc(wi_runtime_instance_t *);
static wi_runtime_instance_t *			_wi_array_copy(wi_runtime_instance_t *);
static wi_boolean_t						_wi_array_is_equal(wi_runtime_instance_t *, wi_runtime_instance_t *);
static wi_string_t *					_wi_array_description(wi_runtime_instance_t *);

static void								_wi_array_grow(wi_array_t *, uint32_t);
static void								_wi_array_optimize(wi_array_t *);

static _wi_array_item_t *				_wi_array_item_create(wi_array_t *);
static void								_wi_array_item_remove(wi_array_t *, _wi_array_item_t *);
static void								_wi_array_add_item(wi_array_t *, _wi_array_item_t *);
static void								_wi_array_add_item_sorted(wi_array_t *, _wi_array_item_t *, wi_compare_func_t *);
static void								_wi_array_insert_item_at_index(wi_array_t *, _wi_array_item_t *, uint32_t);
#ifdef HAVE_QSORT_R
static int								_wi_array_compare_data(void *, const void *, const void *);
#else
static int								_wi_array_compare_data(const void *, const void *);
#endif


const wi_array_callbacks_t				wi_array_default_callbacks = {
	wi_retain,
	wi_release,
	wi_is_equal,
	wi_description
};

static uint32_t							_wi_array_items_per_page;

#ifndef HAVE_QSORT_R
static wi_lock_t						*_wi_array_sort_lock;
static wi_compare_func_t				*_wi_array_sort_function;
#endif

static wi_runtime_id_t					_wi_array_runtime_id = WI_RUNTIME_ID_NULL;
static wi_runtime_class_t				_wi_array_runtime_class = {
	"wi_array_t",
	_wi_array_dealloc,
	_wi_array_copy,
	_wi_array_is_equal,
	_wi_array_description,
	NULL
};


void wi_array_register(void) {
	_wi_array_runtime_id = wi_runtime_register_class(&_wi_array_runtime_class);
}



void wi_array_initialize(void) {
	_wi_array_items_per_page = wi_page_size() / sizeof(_wi_array_item_t);
	
#ifndef HAVE_QSORT_R
	_wi_array_sort_lock = wi_lock_init(wi_lock_alloc());
#endif
}



#pragma mark -

wi_runtime_id_t wi_array_runtime_id(void) {
	return _wi_array_runtime_id;
}



#pragma makr -

wi_array_t * wi_array_alloc(void) {
	return wi_runtime_create_instance(_wi_array_runtime_id, sizeof(wi_array_t));
}



wi_array_t * wi_array_init(wi_array_t *array) {
	return wi_array_init_with_capacity(array, 0);
}



wi_array_t * wi_array_init_with_capacity(wi_array_t *array, uint32_t capacity) {
	return wi_array_init_with_capacity_and_callbacks(array, capacity, wi_array_default_callbacks);
}



wi_array_t * wi_array_init_with_capacity_and_callbacks(wi_array_t *array, uint32_t capacity, wi_array_callbacks_t callbacks) {
	array->callbacks			= callbacks;
	array->item_chunks_offset	= _wi_array_items_per_page;
	array->items_count			= WI_MAX(wi_exp2m1(wi_log2(capacity) + 1), _WI_ARRAY_MIN_COUNT);
	array->min_count			= array->items_count;
	array->items				= wi_malloc(array->items_count * sizeof(_wi_array_item_t *));
	array->lock					= wi_rwlock_init(wi_rwlock_alloc());
	
	return array;
}



wi_array_t * wi_array_init_with_data(wi_array_t *array, ...) {
	void		*data;
	va_list		ap;

	array = wi_array_init_with_capacity(array, 0);

	va_start(ap, array);
	while((data = va_arg(ap, void *)))
		wi_array_add_data(array, data);
	va_end(ap);

	return array;
}



wi_array_t * wi_array_init_with_list(wi_array_t *array, wi_list_t *list) {
	wi_list_node_t	*node;
	void			*data;
	
	array = wi_array_init_with_capacity(array, wi_list_count(list));
	
	WI_LIST_FOREACH(list, node, data)
		wi_array_add_data(array, data);
	
	return array;
}



wi_array_t * wi_array_init_with_data_and_count(wi_array_t *array, void **data, uint32_t count) {
	uint32_t		i;
	
	array = wi_array_init_with_capacity(array, count);

	for(i = 0; i < count; i++)
		wi_array_add_data(array, data[i]);
	
	return array;
}



wi_array_t * wi_array_init_with_argv(wi_array_t *array, int argc, const char **argv) {
	wi_string_t		*string;
	int				i;
	
	array = wi_array_init_with_capacity(array, argc);
	
	for(i = 0; i < argc; i++) {
		string = wi_string_init_with_cstring(wi_string_alloc(), argv[i]);
		wi_array_add_data(array, string);
		wi_release(string);
	}
	
	return array;
}



wi_array_t * wi_array_init_with_string(wi_array_t *array, wi_string_t *string, wi_string_t *separator) {
	if(wi_string_length(string) == 0) {
		array = wi_array_init_with_data(array, WI_STR(""), (void *) NULL);
	} else {
		wi_release(array);
		
		array = wi_retain(wi_string_components_separated_by_string(string, separator));
	}
	
	return array;
}



wi_array_t * wi_array_init_with_argument_string(wi_array_t *array, wi_string_t *string, int32_t index) {
	wi_string_t		*data;
	const char		*cstring;
	char			*buffer, *end;
	uint32_t		count;
	wi_boolean_t	squote, dquote, bsquote;
	
	array		= wi_array_init_with_capacity(array, 0);
	cstring		= wi_string_cstring(string);
	buffer		= wi_malloc(strlen(cstring) + 1);
	count		= 0;

	squote = dquote = bsquote = false;

	while(*cstring) {
		if(index < 0 || (index >= 0 && count != (uint32_t) index)) {
			while(isspace(*cstring))
				cstring++;
		}
		
		end = buffer;
		
		while(*cstring) {
			if(index >= 0 && count == (uint32_t) index) {
				*end++ = *cstring++;
				
				continue;
			}

			if(isspace(*cstring) && !squote && !dquote && !bsquote)
				break;
			
			if(bsquote) {
				bsquote = false;
				*end++ = *cstring;
			}
			else if(squote) {
				if(*cstring == '\'')
					squote = false;
				else
					*end++ = *cstring;
			}
			else if(dquote) {
				if(*cstring == '"')
					dquote = false;
				else
					*end++ = *cstring;
			}
			else {
				if(*cstring == '\'')
					squote = true;
				else if(*cstring == '"')
					dquote = true;
				else if(*cstring == '\\')
					bsquote = true;
				else
					*end++ = *cstring;
			}
			
			cstring++;
		}
		
		*end = '\0';
		
		data = wi_string_init_with_cstring(wi_string_alloc(), buffer);
		wi_array_add_data(array, data);
		wi_release(data);
		
		count++;
		
		while(isspace(*cstring))
			cstring++;
	}
	
	wi_free(buffer);
	
	return array;
}



static void _wi_array_dealloc(wi_runtime_instance_t *instance) {
	wi_array_t		*array = instance;
	uint32_t		i;

	wi_array_remove_all_data(array);

	if(array->item_chunks) {
		for(i = 0; i < array->item_chunks_count; i++)
			wi_free(array->item_chunks[i]);

		wi_free(array->item_chunks);
	}

	wi_release(array->lock);

	wi_free(array->items);
}



static wi_runtime_instance_t * _wi_array_copy(wi_runtime_instance_t *instance) {
	wi_array_t		*array = instance, *array_copy;
	uint32_t		i;

	array_copy = wi_array_init_with_capacity_and_callbacks(wi_array_alloc(), array->data_count, array->callbacks);

	for(i = 0; i < array->data_count; i++)
		wi_array_add_data(array_copy, array->items[i]->data);

	return array_copy;
}



static wi_boolean_t _wi_array_is_equal(wi_runtime_instance_t *instance1, wi_runtime_instance_t *instance2) {
	wi_array_t			*array1 = instance1;
	wi_array_t			*array2 = instance2;
	uint32_t			i;
	
	if(array1->data_count != array2->data_count)
		return false;
	
	if(array1->callbacks.is_equal != array2->callbacks.is_equal)
		return false;
	
	for(i = 0; i < array1->data_count; i++) {
		if(!_WI_ARRAY_IS_EQUAL(array1, WI_ARRAY(array1, i), WI_ARRAY(array2, i)))
			return false;
	}

	return true;
}



static wi_string_t * _wi_array_description(wi_runtime_instance_t *instance) {
	wi_array_t			*array = instance;
	wi_string_t			*string, *description;
	void				*data;
	uint32_t			i;
	
	string = wi_string_with_format(WI_STR("<%s %p>{count = %d, values = (\n"),
		wi_runtime_class_name(array),
		array,
		array->data_count);
	
	for(i = 0; i < array->data_count; i++) {
		data = WI_ARRAY(array, i);
		
		if(array->callbacks.description)
			description = (*array->callbacks.description)(data);
		else
			description = wi_string_with_format(WI_STR("%p"), data);
		
		wi_string_append_format(string, WI_STR("    %u: %@\n"), i, description);
	}
	
	wi_string_append_string(string, WI_STR(")}"));
	
	return string;
}



#pragma mark -

void wi_array_wrlock(wi_array_t *array) {
	wi_rwlock_wrlock(array->lock);
}



void wi_array_rdlock(wi_array_t *array) {
	wi_rwlock_rdlock(array->lock);
}



void wi_array_unlock(wi_array_t *array) {
	wi_rwlock_unlock(array->lock);
}



#pragma mark -

uint32_t wi_array_count(wi_array_t *array) {
	return array->data_count;
}



void * wi_array_first_data(wi_array_t *array) {
	return array->data_count > 0 ? array->items[0]->data : NULL;
}



void * wi_array_last_data(wi_array_t *array) {
	return array->data_count > 0 ? array->items[array->data_count - 1]->data : NULL;
}



wi_boolean_t wi_array_contains_data(wi_array_t *array, void *data) {
	uint32_t		i;

	for(i = 0; i < array->data_count; i++) {
		if(_WI_ARRAY_IS_EQUAL(array, array->items[i]->data, data))
			return true;
	}

	return false;
}



uint32_t wi_array_index_of_data(wi_array_t *array, void *data) {
	uint32_t		i;

	for(i = 0; i < array->data_count; i++) {
		if(_WI_ARRAY_IS_EQUAL(array, array->items[i]->data, data))
			return i;
	}

	return WI_NOT_FOUND;
}



void wi_array_get_data(wi_array_t *array, void **data) {
	uint32_t		i;
	
	for(i = 0; i < array->data_count; i++)
		data[i] = array->items[i]->data;
}



void wi_array_get_data_in_range(wi_array_t *array, void **data, wi_range_t range) {
	uint32_t		i;
	
	_WI_ARRAY_INDEX_ASSERT(array, range.location);
	_WI_ARRAY_INDEX_ASSERT(array, range.location + range.length - 1);
	
	for(i = range.location; i < range.location + range.length; i++)
		data[i] = array->items[i]->data;
}



wi_string_t * wi_array_components_joined_by_string(wi_array_t *array, wi_string_t *separator) {
	wi_string_t		*string, *description;
	void			*data;
	uint32_t		i;
	
	string = wi_string_init(wi_string_alloc());
	
	for(i = 0; i < array->data_count; i++) {
		data = WI_ARRAY(array, i);
		
		if(array->callbacks.description)
			description = (*array->callbacks.description)(data);
		else
			description = wi_string_with_format(WI_STR("%p"), data);
		
		wi_string_append_string(string, description);
	
		if(i < array->data_count - 1)
			wi_string_append_string(string, separator);
	}
	
	return wi_autorelease(string);
}



const char ** wi_array_argv(wi_array_t *array) {
	wi_string_t		*description;
	const char		**argv;
	void			*data;
	uint32_t		i;
	
	argv = wi_malloc(array->data_count * sizeof(void *));
	
	for(i = 0; i < array->data_count; i++) {
		data = WI_ARRAY(array, i);
		
		if(array->callbacks.description)
			description = (*array->callbacks.description)(data);
		else
			description = wi_string_with_format(WI_STR("%p"), data);
		
		argv[i]	= strdup(wi_string_cstring(description));
	}
	
	return argv;
}



wi_array_t * wi_array_subarray_with_range(wi_array_t *array, wi_range_t range) {
	wi_array_t		*newarray;
	uint32_t		i;
	
	_WI_ARRAY_INDEX_ASSERT(array, range.location);
	_WI_ARRAY_INDEX_ASSERT(array, range.location + range.length - 1);
	
	newarray = wi_array_init_with_capacity(wi_array_alloc(), range.length);
	
	for(i = range.location; i < range.location + range.length; i++)
		wi_array_add_data(newarray, WI_ARRAY(array, i));

	return wi_autorelease(newarray);
}



#pragma mark -

wi_enumerator_t * wi_array_data_enumerator(wi_array_t *array) {
	return wi_autorelease(wi_enumerator_init_with_array(wi_enumerator_alloc(), array, wi_enumerator_array_data_enumerator));
}



wi_enumerator_t * wi_array_reverse_data_enumerator(wi_array_t *array) {
	return wi_autorelease(wi_enumerator_init_with_array(wi_enumerator_alloc(), array, wi_enumerator_array_reverse_data_enumerator));
}



void * wi_enumerator_array_data_enumerator(wi_runtime_instance_t *instance, void *context) {
	wi_array_t		*array = instance;
	uint32_t		*index = context;
	void			*data;
	
	if(*index == array->data_count)
		return NULL;
	
	data = wi_array_data_at_index(array, *index);
	
	(*index)++;
	
	return data;
}



void * wi_enumerator_array_reverse_data_enumerator(wi_runtime_instance_t *instance, void *context) {
	wi_array_t		*array = instance;
	uint32_t		*index = context;
	void			*data;
	
	if(*index == array->data_count)
		return NULL;
	
	data = wi_array_data_at_index(array, array->data_count - *index - 1);
	
	(*index)++;
	
	return data;
}



#pragma mark -

static void _wi_array_grow(wi_array_t *array, uint32_t index) {
	uint32_t		items_count;
	
	items_count			= (uint32_t) ((double) ((index + 1) * 3) / 2.0);
	array->items		= wi_realloc(array->items, items_count * sizeof(_wi_array_item_t *));
	array->items_count	= items_count;
}



static void _wi_array_optimize(wi_array_t *array) {
	uint32_t		items_count;

	items_count			= WI_CLAMP(array->data_count, array->min_count, _WI_ARRAY_MAX_COUNT);
	array->items		= wi_realloc(array->items, items_count * sizeof(_wi_array_item_t *));
	array->items_count	= items_count;
}



#pragma mark -

static _wi_array_item_t * _wi_array_item_create(wi_array_t *array) {
	_wi_array_item_t	*item, *item_block;
	size_t				size;

	if(!array->item_free_list) {
		if(array->item_chunks_offset == _wi_array_items_per_page) {
			array->item_chunks_count++;

			size = array->item_chunks_count * sizeof(_wi_array_item_t *);
			array->item_chunks = wi_realloc(array->item_chunks, size);

			size = _wi_array_items_per_page * sizeof(_wi_array_item_t);
			array->item_chunks[array->item_chunks_count - 1] = wi_malloc(size);

			array->item_chunks_offset = 0;
		}

		item_block = array->item_chunks[array->item_chunks_count - 1];
		array->item_free_list = &item_block[array->item_chunks_offset++];
		array->item_free_list->link = NULL;
	}

	item = array->item_free_list;
	array->item_free_list = item->link;

	return item;
}



static void _wi_array_item_remove(wi_array_t *array, _wi_array_item_t *item) {
	_WI_ARRAY_RELEASE(array, item->data);
	item->data = NULL;
	
	item->link = array->item_free_list;
	array->item_free_list = item;
}



static void _wi_array_add_item(wi_array_t *array, _wi_array_item_t *item) {
	if(array->data_count >= array->items_count)
		_wi_array_grow(array, array->data_count);

	array->items[array->data_count] = item;
	array->data_count++;
}



static void _wi_array_add_item_sorted(wi_array_t *array, _wi_array_item_t *item, wi_compare_func_t *compare) {
	uint32_t		i;
	
	if(array->data_count == 0) {
		_wi_array_add_item(array, item);
	} else {
		for(i = 0; i < array->data_count; i++) {
			if((*compare)(item->data, array->items[i]->data) < 0) {
				_wi_array_insert_item_at_index(array, item, i);
				
				return;
			}
		}
		
		_wi_array_add_item(array, item);
	}
}



static void _wi_array_insert_item_at_index(wi_array_t *array, _wi_array_item_t *item, uint32_t index) {
	if(index >= array->items_count)
		_wi_array_grow(array, index);
	
	memcpy(array->items + index + 1,
		   array->items + index,
		   (array->data_count - index) * sizeof(_wi_array_item_t *));
	
	array->items[index] = item;
	array->data_count++;
}



#ifdef HAVE_QSORT_R

static int _wi_array_compare_data(void *context, const void *p1, const void *p2) {
	return (*(wi_compare_func_t *) context)(*(void **) p1, *(void **)p2);
}

#else

static int _wi_array_compare_data(const void *p1, const void *p2) {
	return (*_wi_array_sort_function)(*(void **) p1, *(void **)p2);
}

#endif



#pragma mark -

void wi_array_sort(wi_array_t *array, wi_compare_func_t *compare) {
	void		**data;
	uint32_t	i;
	
	if(array->data_count == 0)
		return;
	
	data = wi_malloc(sizeof(void *) * array->data_count);
	wi_array_get_data(array, data);

#ifdef HAVE_QSORT_R
	qsort_r(data, array->data_count, sizeof(void *), compare, _wi_array_compare_data);
#else
	wi_lock_lock(_wi_array_sort_lock);
	_wi_array_sort_function = compare;
	qsort(data, array->data_count, sizeof(void *), _wi_array_compare_data);
	wi_lock_unlock(_wi_array_sort_lock);
#endif
	
	for(i = 0; i < array->data_count; i++)
		array->items[i]->data = data[i];
	
	wi_free(data);
}



void wi_array_reverse(wi_array_t *array) {
	_wi_array_item_t	*item;
	uint32_t			i, max, count;

	count = array->data_count;
	max = count / 2;

	for(i = 0; i < max; i++) {
		item = array->items[i];
		array->items[i] = array->items[count - i - 1];
		array->items[count - i - 1] = item;
	}
}



#pragma mark -

void wi_array_add_data(wi_array_t *array, void *data) {
	_wi_array_item_t	*item;

	item = _wi_array_item_create(array);
	item->data = _WI_ARRAY_RETAIN(array, data);

	_wi_array_add_item(array, item);
}



void wi_array_add_data_sorted(wi_array_t *array, void *data, wi_compare_func_t *compare) {
	_wi_array_item_t	*item;

	item = _wi_array_item_create(array);
	item->data = _WI_ARRAY_RETAIN(array, data);

	_wi_array_add_item_sorted(array, item, compare);
}



void wi_array_add_data_from_array(wi_array_t *array, wi_array_t *otherarray) {
	uint32_t		i, count;
	
	count = wi_array_count(otherarray);
	
	for(i = 0; i < count; i++)
		wi_array_add_data(array, wi_array_data_at_index(otherarray, i));
}



void wi_array_insert_data_at_index(wi_array_t *array, void *data, uint32_t index) {
	_wi_array_item_t	*item;
	
	_WI_ARRAY_INDEX_ASSERT(array, index);

	item = _wi_array_item_create(array);
	item->data = _WI_ARRAY_RETAIN(array, data);

	_wi_array_insert_item_at_index(array, item, index);
}



void wi_array_replace_data_at_index(wi_array_t *array, void *data, uint32_t index) {
	_wi_array_item_t	*item;
	
	_WI_ARRAY_INDEX_ASSERT(array, index);

	item = _wi_array_item_create(array);
	item->data = _WI_ARRAY_RETAIN(array, data);
	
	_wi_array_item_remove(array, array->items[index]);
	array->items[index] = item;
}



void * wi_array_data_at_index(wi_array_t *array, uint32_t index) {
	_WI_ARRAY_INDEX_ASSERT(array, index);

	return array->items[index]->data;
}



void wi_array_set_array(wi_array_t *array, wi_array_t *otherarray) {
	wi_array_remove_all_data(array);
	wi_array_add_data_from_array(array, otherarray);
}



#pragma mark -

void wi_array_remove_data_at_index(wi_array_t *array, uint32_t index) {
	_WI_ARRAY_INDEX_ASSERT(array, index);

	_wi_array_item_remove(array, array->items[index]);

	memcpy(array->items + index,
		   array->items + index + 1,
		   (array->data_count - index) * sizeof(_wi_array_item_t *));

	array->items[array->data_count] = NULL;
	array->data_count--;

	_WI_ARRAY_CHECK_OPTIMIZE(array);
}



void wi_array_remove_data_in_range(wi_array_t *array, wi_range_t range) {
	uint32_t		count;
	
	count = range.length;
	
	while(count > 0) {
		wi_array_remove_data_at_index(array, range.location);
		
		count--;
	}
}



void wi_array_remove_all_data(wi_array_t *array) {
	uint32_t		i, count;
	
	count = array->data_count;
	
	if(count > 0) {
		for(i = 0; i < count; i++)
			_wi_array_item_remove(array, array->items[i]);
		
		array->data_count = 0;
	}

	_WI_ARRAY_CHECK_OPTIMIZE(array);
}
