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
    size_t length;  //char array size, not string size
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

AJson5*CreateTrue();
AJson5*CreateNull();
AJson5*CreateFalse();
AJson5 *CreateString(char*s);

FuncStat InsertItemToArrayByIndex(AJson5 *target, size_t index, AJson5 *item);

/* create array includes something */

AJson5 *CreateNumberArray(size_t len, double nums[]);
AJson5 *CreateStringArray(size_t len, char *vals[]);

/*
add something  to obj directly
*/


FuncStat AddNullToObjectByKey(AJson5 *target, char *key);
FuncStat AddTrueToObjectByKey(AJson5 *target, char *key);
FuncStat AddFalseToObjectByKey(AJson5 *target, char *key);
FuncStat AddStringToObjectByKey(AJson5 *target, char *key, char *val);
FuncStat AddNumberToObjectByKey(AJson5 *target, char *key, double val);


FuncStat AddItemToTarget(AJson5 *target, char *key, AJson5 *item);
FuncStat AddNumberArrayToTargetByKey(AJson5 *target, char *key, size_t n, double nums[]);
FuncStat AddStringArrayToTargetByKey(AJson5 *target, char *key, size_t n, char *vals[]);

FuncStat AddNumberToArray(AJson5 *target,double val);
FuncStat AddNullToArray(AJson5 *target);
FuncStat AddFalseToArray(AJson5 *target);
FuncStat AddTrueToArray(AJson5 *target);
FuncStat AddStringToArray(AJson5 *target,char*item);

/*
delete something from item
must be support auto release/free all the child item
*/

FuncStat DeleteItemFromArrayByIndex(AJson5 *src, size_t index);
FuncStat DeleteItemFromObjectByKey(AJson5 *src, char *key);
FuncStat Clear(AJson5 *target);

/*
replace something
*/
FuncStat ReplaceItemInArrayByIndex(AJson5 *target, size_t index, AJson5 *new_value);
FuncStat ReplaceItemInObjectByKey(AJson5 *target, char *key, AJson5 *new_value);

// get

AJson5*GetItemByKey(AJson5 *target, char *key);
char *GetValueString(AJson5 *item);
ValueType GetValueBool(AJson5 *item);
uint64_t GetValueUInt(AJson5 *item);
int64_t GetValueInt(AJson5 *item);
double GetValueDouble(AJson5 *item);
AJson5 *GetArrayItemByindex(AJson5 *item, size_t index);

char *GetValueStringByKey(AJson5 *target, char *key);
ValueType GetBoolValueByKey(AJson5 *target, char *key);
uint64_t GetUIntValueByKey(AJson5 *target, char *key);
int64_t GetIntValueByKey(AJson5 *target, char *key);
double GetDoubleValueByKey(AJson5 *target, char *key);
double GetNumberValueByKey(AJson5 *target, char *key);



/* parse*/

// parse the full obejct

FuncStat Dumplicate(char *buf, AJson5 *target);

/*
    parse some partion of json
    e.g. array string or object string
*/
AJson5 *LoadFromString(char *s,size_t size);

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