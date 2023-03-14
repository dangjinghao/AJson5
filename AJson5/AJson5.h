#ifndef AJson5_h
#define AJson5_h
#include <stdint.h>
#include <string.h>
/*
    please try to implement a method
    looks like list calling in other language
    or oo
    NO YOU DON'T!

*/

typedef enum
{
    STATUS_OK,
    STATUS_ERROR,
    STATUS_AJSON5_NOT_FOUND_ERROR,
    STATUS_AJSON5_SUBSCRIPT_ERROR,
    STATUS_AJSON5_Allocate_ERROR,
    STATUS_AGB_INIT_ERROR,
    STATUS_AGB_ARGUMENT_ERROR,
    STATUS_AGB_REALLOC_ERROR,
    
} FuncStat;
typedef enum
{
    AJson5_FALSE,
    AJson5_TRUE,
    AJson5_EMPTY,
    AJson5_NULL,
    AJson5_UINT64,
    AJson5_INT64,
    AJson5_STRING,
    AJson5_DOUBLE,
    AJson5_ARRAY,
    AJson5_OBJECT
} ValueType;

typedef struct AJson5
{
    // just a single link list
    // the double link list that I don't know how to use is unnecessary
    struct AJson5 *next;
    ValueType type;
    char *key; // if the item's father is obj,it will be exist
    union
    {
        struct AJson5 *Child; // if the type is  array or obj, the child will be exist
        char *Str;
        uint64_t Uint; // identify the unsigned and signed by enum type and use it by compulsory type conversion
        int64_t Int;
        double Double;
    } value;
} AJson5;

typedef struct
{
    char *content;
    size_t length;
    size_t offset;
} parse_buffer;

typedef struct
{
    char *array;
    size_t length;
    size_t capacity;
} AutoGrowthBuffer, AGB;

typedef char flag_t;

// auto check double, int, uint.That is great
AJson5 *CreateNumber(double num);
// create an empty object
AJson5 *CreateObject();
// create an empty array
AJson5 *CreateArray();

FuncStat InsertItemToArray(AJson5 *target, size_t n, AJson5 *item);

/* create array includes something */

AJson5 *CreateNumberArray(size_t len, double nums[]);
AJson5 *CreateStringArray(size_t len, char *vals[]);

/*
add something  to obj directly
*/

FuncStat AddNullToObject(AJson5 *target, char *key);
FuncStat AddTrueToObject(AJson5 *target, char *key);
FuncStat AddFalseToObject(AJson5 *target, char *key);
FuncStat AddStringToObject(AJson5 *target, char *key, char *val);
FuncStat AddNumberToObject(AJson5 *target, char *key, double val);
FuncStat AddObjectToObject(AJson5 *target, char *key, AJson5 *item);
FuncStat AddArrayToObject(AJson5 *target, char *key, AJson5 *item);
FuncStat AddNumberArrayToObject(AJson5 *target, char *key, size_t n, double nums[]);
FuncStat AddStringArrayToObject(AJson5 *target, char *key, size_t n, char *vals[]);
/*
delete something from item
must be support auto release/free all the child item
*/

FuncStat DeleteItemFromArray(AJson5 *src, size_t subscript);
FuncStat DeleteItemFromObject(AJson5 *src, char *key);
FuncStat Clear(AJson5 *target);
/*
replace something
*/

FuncStat ReplaceItemInArray(AJson5 *target, size_t subscript, AJson5 *new_value);
FuncStat ReplaceItemInObject(AJson5 *target, char *key, AJson5 *new_value);

// get

AJson5 *get_item(AJson5 *target, char *key);
char *GetStringValue(AJson5 *item);
ValueType GetBoolValue(AJson5 *item);
uint64_t GetUIntValue(AJson5 *item);
int64_t GetIntValue(AJson5 *item);
double GetDoubleValue(AJson5 *item);
AJson5 *GetItemFromArray(AJson5 *item, size_t n);
char *GetItemStringValue(AJson5 *target, char *key);
ValueType GetItemBoolValue(AJson5 *target, char *key);
uint64_t GetItemUIntValue(AJson5 *target, char *key);
int64_t GetItemIntValue(AJson5 *target, char *key);
double GetItemDoubleValue(AJson5 *target, char *key);
double GetItemNumberValue(AJson5 *target, char *key);

/* parse*/

// parse the full obejct

FuncStat Dumplicate(char *buf, AJson5 *target);

/*
    parse some partion of json
    e.g. array string or object string
*/
AJson5 *LoadFromString(char *s);

/* marco */

//for parse_number function,the most length of number 
#define PARSE_NUMBER_LENGTH 64

// those macro from cJson

#define can_access_at_index(buffer, index) ((buffer != NULL) && (((buffer)->offset + index) < (buffer)->length))
#define cannot_access_at_index(buffer, index) (!can_access_at_index(buffer, index))

#define can_read(buffer, size) ((buffer != NULL) && (((buffer)->offset + size) <= (buffer)->length))
// get the pointer point to current buffer char
#define buffer_at_offset(buffer) ((buffer)->content + (buffer)->offset)
#endif