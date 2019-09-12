// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#if !defined(PROPERTIES)
#define PROPERTIES

enum {
	PROPERTY_TYP_INTEGER,
	PROPERTY_TYP_STRING,
	PROPERTY_TYP_DOUBLE,
	PROPERTY_TYP_CHOICE,
	PROPERTY_TYP_USERDATA
};

enum {
	PROPERTY_HINT_EDIT,
	PROPERTY_HINT_LIST
};

typedef struct {
	char* key;	
	union {
		char* s;
		int i;
		double d;
		void* ud;
	} value;
	int min;
	int max;
	int typ;
	int hint;	
} Property;

struct PropertiesStruct {
	Property item;	
	struct PropertiesStruct* next;
	struct PropertiesStruct* children;
	void (*dispose)(Property*);
};

typedef struct PropertiesStruct Properties;


void properties_init(Properties* self);
Properties* properties_create(void);
void properties_free(Properties* self);
Properties* properties_append_string(Properties*, const char* key, const char* value);
Properties* properties_append_choice(Properties*, const char* key, int value);
Properties* properties_append_userdata(Properties*, const char* key,
	void* value, void (*dispose)(Property*));
Properties* properties_append_int(Properties*, const char* key, int value, int min, int max);
Properties* properties_append_double(Properties*, const char* key, double value, double min, double max);
Properties* properties_read(Properties*, const char* key);
void properties_readint(Properties*, const char* key, int* value, int defaultvalue);
void properties_readdouble(Properties*, const char* key, double* value, double defaultvalue);
void properties_readstring(Properties*, const char* key, char** text, char* defaulttext);
void properties_write_string(Properties*, const char* key, const char* value);
void properties_write_int(Properties*, const char* key, int value);
void properties_write_double(Properties*, const char* key, double value);
void properties_enumerate(Properties*, void* target, int (*enumerate)(void* self, struct PropertiesStruct* properties, int level));
Properties* properties_find(Properties*, const char* key);
const char* properties_key(Properties*);
int properties_value(Properties*);
const char* properties_valuestring(Properties*);

#endif
