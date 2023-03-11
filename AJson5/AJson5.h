#ifndef AJson5_h
#define AJson5_h
#include <stdint.h>
#include <stddef.h>
#include<string.h>
/*
    create  done
    delete  declear only
    update/replace  declear only
    search/get
    print/format
    duplicate

    please try to implement a method
    looks like list calling in other language
    or oo

*/

typedef enum
{
    STATUS_OK,
    STATUS_ERROR,
    STATUS_NOT_FOUND_ERROR,
    STATUS_SUBSCRIPT_ERROR,
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
    char* content;
    size_t length;
    size_t offset;
}parse_buffer;



// if I can use generic...
/*
    create all basic type
*/

AJson5 *CreateNull();
AJson5 *CreateBool(ValueType t);
AJson5 *CreateInt(ValueType t, int64_t num);
AJson5 *CreateDouble(double num);
AJson5 *CreateString(char *s);
AJson5 *CreateObject();
AJson5 *CreateArray();
// auto check double, int, uint.That is great
AJson5 *CreateNumber(double num);

// the base addTo function

FuncStat AddItemToObject(AJson5 *target, char *keyName, AJson5 *item);
FuncStat AddItemToArray(AJson5 *target, AJson5 *item);
// I think we need the way to add item to array with subscript
// But maybe add to json object is unnecessary ?
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

/*
    format something
    maybe I should implement them  early
*/

FuncStat FormatValue(char*buf,AJson5 *target);
FuncStat FormatObject(char*buf,AJson5 *target);
FuncStat FormatArray(char*buf,AJson5 *target);
FuncStat FormatString(char*buf,AJson5 *target);
FuncStat FormatNumbers(char*buf,AJson5 *target);


// get


AJson5 *GetItem(AJson5 *target, char *key);
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
AJson5*ParseWithLength(char*value,size_t length);
/* marco */

//those macro from cJson

#define can_access_at_index(buffer, index) ((buffer != NULL) && (((buffer)->offset + index) < (buffer)->length))
#define cannot_access_at_index(buffer, index) (!can_access_at_index(buffer, index))


#define can_read(buffer, size) ((buffer != NULL) && (((buffer)->offset + size) <= (buffer)->length))
//get the pointer point to current buffer char
#define buffer_at_offset(buffer) ((buffer)->content + (buffer)->offset)
#endif