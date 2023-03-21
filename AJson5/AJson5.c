#include <stdlib.h>
#include <stdio.h>
#include "AJson5.h"
#include <string.h>
#include <strings.h>
// just for learning
// maybe I should create a hook to suport custom alloc function like cJSON

// declaration


static AJson5 *new_item();
static FuncStat AJson5_free_item(AJson5 *target);
static parse_buffer *buffer_skip_whitespace(parse_buffer *buffer);
/*
    create all basic type
*/
static AJson5 *Ajson5_create_null();
static AJson5 *AJson5_create_bool(ValueType t);
static AJson5 *AJson5_create_int(ValueType t, int64_t num);
static AJson5 *AJson5_create_double(double num);
static AJson5 *AJson5_create_string(char *s);
static AJson5 *AJson5_create_object();
static AJson5 *AJson5_create_array();
static FuncStat AJson5_add_item_to_array(AJson5 *target, AJson5 *item);
static FuncStat AJson5_add_item_to_target(AJson5 *target, char *key, AJson5 *item);
static AJson5 *AJson5_get_item(AJson5 *target, char *key);

static FuncStat AJson5_format_value(AutoGrowthBuffer *buf, AJson5 *target);
static FuncStat AJson5_format_object(AutoGrowthBuffer *buf, AJson5 *target);
static FuncStat AJson5_format_array(AutoGrowthBuffer *buf, AJson5 *target);
static FuncStat AJson5_format_string(AutoGrowthBuffer *buf, AJson5 *target);
static FuncStat AJson5_format_number(AutoGrowthBuffer *buf, AJson5 *target);

static FuncStat AJson5_parse_value(AJson5 *item, parse_buffer *input_buffer);
static FuncStat AJson5_parse_string(AJson5 *item, parse_buffer *input_buffer);
static FuncStat AJson5_parse_array(AJson5 *item, parse_buffer *input_buffer);
static FuncStat AJson5_parse_object(AJson5 *item, parse_buffer *input_buffer);
static FuncStat AJson5_parse_special_key_string(AJson5 *item, parse_buffer *input_buffer);
// done

// implements of  auto growth buffer

/*
initialize a agb object
return STATUS_OK or STATUS_AGB_INIT_ERROR
*/
FuncStat AGB_Init(AutoGrowthBuffer *b)
{
    b->array = calloc(257, sizeof(char));
    if (b->array == NULL)
    {
        goto fail;
    }

success:
    b->length = 0;
    b->capacity = 256; // this must be enough
    return STATUS_OK;
fail:
    return STATUS_AGB_INIT_ERROR;
}

// realloc agb object's array 
// return STATUS, STATUS_ERROR when b->array is NULL 
// and STATUS_AGB_REALLOC_ERROR 
static FuncStat AGB_Realloc(AutoGrowthBuffer *b)
{
    if (b->array == NULL)
    {
        return STATUS_ERROR;
    }
    b->capacity += b->capacity / 2;
    b->array = (char *)realloc(b->array, b->capacity);
    if (b->array == NULL)
    {
        b->capacity = 0;
        return STATUS_AGB_REALLOC_ERROR;
    }

    return STATUS_OK;
}

//append item to agb object 
//return STATUS_OK,AGB_Realloc ERROR when it's error occurred
FuncStat AGB_Append(AutoGrowthBuffer *b, char *item)
{
    size_t needed_size = b->length + strlen(item);
    while (needed_size >= b->capacity)
    {
        FuncStat ret;
        if (ret = AGB_Realloc(b))
        {
            return ret;
        }
    }
    strcat(b->array, item);
    b->length = needed_size;
    return STATUS_OK;
}


// replace contents of b with src
// return STATUS_OK,STATUS_AGB_ARGUMENT_ERROR b->capacity is 0 (ralloc failed previously) or src is NULL/
// AGB_Realloc error return while error occurred
FuncStat AGB_Replace(AutoGrowthBuffer *b, char *src)
{
    if (b->capacity == 0 || src == NULL)
    {
        return STATUS_AGB_ARGUMENT_ERROR;
    }
    size_t needed_size = strlen(src);
    while (needed_size >= b->capacity)
    {
        FuncStat ret;
        if (ret = AGB_Realloc(b))
        {
            return ret;
        }
    }
    bzero(b->array, b->length);
    strcpy(b->array, src);
    b->length = needed_size;
    return STATUS_OK;
}

//clrear all of a agb content
FuncStat AGB_Clear(AutoGrowthBuffer *b)
{
    bzero(b->array, b->capacity);
    b->length = 0;
    return STATUS_OK;
}

//delete last n char from agb contents 
//return STATUS_OK, STATUS_AGB_ARGUMENT_ERROR when b->length < n
FuncStat AGB_DeleteNLast(AutoGrowthBuffer *b, size_t n)
{
    if (b->length < n){
        return STATUS_AGB_ARGUMENT_ERROR;
    }
    b->length -= n;
    bzero(b->array + b->length, n);
    return STATUS_OK;
}

FuncStat AGB_Release(AutoGrowthBuffer *b){
    free(b->array);
    return STATUS_OK;
}

static AJson5 *new_item()
{
    AJson5 *item=NULL;
    do{
        item = (AJson5 *)calloc(1, sizeof(AJson5));
    }while (item==NULL);//make sure allocate success forever

    item->type = AJson5_EMPTY;
    item->next = NULL;
    return item;
    
}

static AJson5 *Ajson5_create_null()
{
    AJson5 *item = new_item();
    item->type = AJson5_NULL;
    return item;
}

AJson5*CreateNull(){
    return Ajson5_create_null();
}

AJson5*CreateTrue(){
    return AJson5_create_bool(AJson5_TRUE);
}


AJson5*CreateFalse(){
    return AJson5_create_bool(AJson5_FALSE);
}
static AJson5 *AJson5_create_bool(ValueType t)
{
    AJson5 *item = new_item();
    item->type = (t == AJson5_TRUE ? AJson5_TRUE : AJson5_FALSE);
    return item;
}

/*
 * t only support UINT64 or INT64
 * other UINT64
 */
static AJson5 *AJson5_create_int(ValueType t, int64_t num)
{
    AJson5 *item = new_item();
    if (t == AJson5_INT64)
    {
        item->type = t;
        item->value.Uint = num;
    }
    else
    {
        item->type = AJson5_UINT64;
        item->value.Int = num;
    }

    return item;
}

static AJson5 *AJson5_create_double(double num)
{
    AJson5 *item = new_item();
    item->type = AJson5_DOUBLE;
    item->value.Double = num;

    return item;
}

static AJson5 *AJson5_create_string(char *s)
{
    AJson5 *item = new_item();
    item->type = AJson5_STRING;
        size_t string_length = strlen(s);
        item->value.Str=calloc(sizeof(char),string_length+1);
        strncpy(item->value.Str,s,string_length+1);


    return item;
}

AJson5*CreateString(char*s){
    return AJson5_create_string(s);
}
static AJson5 *AJson5_create_object()
{
    AJson5 *item = new_item();
    item->type = AJson5_OBJECT;
    // empty child item to make sure the same opertate
    item->value.Child = new_item();
    return item;
}
static AJson5 *AJson5_create_array()
{
    AJson5 *item = new_item();
    item->type = AJson5_ARRAY;
    item->value.Child = new_item();
    return item;
}
static FuncStat AJson5_add_item_to_target(
    AJson5 *target,
    char *key,
    AJson5 *item)
{
        // don't add the key
        if(key!=NULL){
            size_t key_string_length = strlen(key);
            item->key=calloc(sizeof(char),key_string_length+1);
            strncpy(item->key,key,key_string_length+1);
        }

    

    AJson5 *child = target->value.Child;
    while (child->next != NULL)
        child = child->next;

    child->next = item;

    return STATUS_OK;
}

static FuncStat AJson5_add_item_to_array(
    AJson5 *target,
    AJson5 *item)
{
    AJson5 *child = target->value.Child;
    while (child->next != NULL)
        child = child->next;
    child->next = item;

    return STATUS_OK;
}
static AJson5 *AJson5_create_number(double num)
{
    AJson5 *item = new_item();

    if (num == (uint64_t)num)
    {
        // number is uint or > 0
        item->type = AJson5_UINT64;
        item->value.Uint = (uint64_t)num;
    }
    else if (num == (int64_t)num)
    {
        // number is int or < 0
        item->type = AJson5_INT64;
        item->value.Uint = (int64_t)num;
    }
    else
    {
        // the number is double float number
        item->type = AJson5_DOUBLE;
        item->value.Double = num;
    }
    return item;
}

AJson5 *CreateNumber(double num)
{
    return AJson5_create_number(num);
}

FuncStat AddNullToObjectByKey(AJson5 *target, char *key)
{
    return AJson5_add_item_to_target(target, key, Ajson5_create_null());
}


FuncStat AddNullToArray(AJson5 *target)
{
    return AJson5_add_item_to_target(target, NULL, Ajson5_create_null());
}


FuncStat AddTrueToObjectByKey(AJson5 *target, char *key)
{
    return AJson5_add_item_to_target(target, key, AJson5_create_bool(AJson5_TRUE));
}

FuncStat AddTrueToArray(AJson5 *target)
{
    return AJson5_add_item_to_target(target, NULL, AJson5_create_bool(AJson5_TRUE));
}

FuncStat AddFalseToObjectByKey(AJson5 *target, char *key)
{
    return AJson5_add_item_to_target(target, key, AJson5_create_bool(AJson5_FALSE));
}

FuncStat AddFalseToArray(AJson5 *target)
{
    return AJson5_add_item_to_target(target, NULL, AJson5_create_bool(AJson5_FALSE));
}

FuncStat AddStringToObjectByKey(AJson5 *target, char *key, char *val)
{
    return AJson5_add_item_to_target(target, key, AJson5_create_string(val));
}

FuncStat AddStringToArray(AJson5 *target,char*str){
    return AJson5_add_item_to_target(target, NULL, AJson5_create_string(str));

}
//if there are not keys exists,key should be NULL
FuncStat AddSomethingToTarget(AJson5 *target, char *key, AJson5 *item){
    return AJson5_add_item_to_target(target, key,item);

}
FuncStat AddNumberToObjectByKey(AJson5 *target, char *key, double val)
{
    return AJson5_add_item_to_target(target, key, AJson5_create_number(val));
}
FuncStat AddNumberToArray(AJson5 *target,double val){
    return AJson5_add_item_to_target(target, NULL, AJson5_create_number(val));

}
AJson5 *CreateNumberArray(size_t n, double nums[])
{
    AJson5 *array = AJson5_create_array();
    AJson5 *the_child = array->value.Child;

    for (size_t i = 0; i < n; i++)
    {
        the_child->next = AJson5_create_number(nums[i]);
        the_child = the_child->next;
    }
    return array;
}
//when target is array,the key should be NULL
FuncStat AddNumberArrayToTargetByKey(AJson5 *target, char *key, size_t n, double nums[])
{
    return AJson5_add_item_to_target(target, key, CreateNumberArray(n, nums));
}

AJson5 *CreateStringArray(size_t len, char *vals[])
{
    AJson5 *array = AJson5_create_array();
    AJson5 *the_child = array->value.Child;

    for (size_t i = 0; i < len; i++)
    {
        the_child->next = AJson5_create_string(vals[i]);
        the_child = the_child->next;
    }
    return array;
}

//when target is array,the key should be NULL
FuncStat AddStringArrayToTargetByKey(AJson5 *target, char *key, size_t n, char *vals[])
{
    return AJson5_add_item_to_target(target, key, CreateStringArray(n, vals));
}

//You can't inset to the end of array,please use add*
//start at 0
FuncStat InsertItemToArrayByIndex(AJson5 *target, size_t index, AJson5 *item)
{

    if(target==NULL||item==NULL){
        return STATUS_ERROR;
    }
    AJson5 *the_child = target->value.Child;
    for (size_t i = 0; i < index && the_child->next != NULL; i++)
        the_child = the_child->next;

    if(the_child->next==NULL){
        return STATUS_AJSON5_SUBSCRIPT_ERROR;
    }
    item->next = the_child->next;
    the_child->next = item;
    return STATUS_OK;
}

//start at 0
FuncStat DeleteItemFromArrayByIndex(AJson5 *src, size_t index)
{
    if(src==NULL||src->type!=AJson5_ARRAY){
        return STATUS_ERROR;
    }

    AJson5 *the_child = src->value.Child;
    for (size_t i = 0; i < index && the_child->next != NULL; i++)
        the_child = the_child->next;
    
    if(the_child->next==NULL){
        return STATUS_AJSON5_SUBSCRIPT_ERROR;
    }
    AJson5 *targetItem = the_child->next;
    the_child->next = targetItem->next;
    AJson5_free_item(targetItem);
    return STATUS_OK;
}


//free the item and it's all child
static FuncStat AJson5_free_item(AJson5 *target)
{
    
    switch (target->type)
    {
    case AJson5_OBJECT:
    case AJson5_ARRAY:
        AJson5 *childItem = target->value.Child;
        //child next handler
        while (childItem != NULL)
        {
            AJson5 *childNext = childItem->next;
            AJson5_free_item(childItem);
            childItem = childNext;
        }
        // free(target->value.Child);//free the empty child
        target->value.Child=NULL;
        break; 
    case AJson5_EMPTY://maybe empty or the 1st node in object or array
        if(target->next!=NULL){

            // AJson5_free_item(target->next); //conflict with the process of free array 
        }
        break;
    case AJson5_STRING:
        free(target->value.Str);  
        target->value.Str=NULL;

    }
    if(target->key!=NULL){
        free(target->key);
        target->key=NULL;
    }
    

    free(target);
    return STATUS_OK;
}
FuncStat DeleteItemFromObjectByKey(AJson5 *src, char *key)
{
    if(src==NULL||src->type!=AJson5_OBJECT) {
        return STATUS_ERROR;
    }

    AJson5 *item = src->value.Child;
    while (item->next != NULL
     && strcmp(item->next->key, key) != 0)
    {
        item = item->next;
    }
    if (item->next != NULL)
    {
        AJson5 *next = item->next->next;
        AJson5_free_item(item->next);
        item->next = next;
        return STATUS_OK;
    }
    return STATUS_AJSON5_NOT_FOUND_ERROR;
}

FuncStat Clear(AJson5 *target)
{
    // only leave the child item
    AJson5_free_item(target);

    return STATUS_OK;
}

FuncStat ReplaceItemInArrayByIndex(AJson5 *target, size_t index, AJson5 *new_item)
{

    if(target==NULL||new_item==NULL){
        return STATUS_ERROR;
    }
    AJson5 *child = target->value.Child;
    for (size_t i = 0; i < index&&child->next!=NULL; i++)
    {
        child = child->next;
    }

    if(child->next==NULL){
        return STATUS_AJSON5_SUBSCRIPT_ERROR;
    }
    new_item->next = child->next->next;
    AJson5_free_item(child->next);
    child->next = new_item;
    return STATUS_OK;
}

FuncStat ReplaceItemInObjectByKey(AJson5 *target, char *key, AJson5 *new_object)
{
    AJson5 *child = target->value.Child;
    while (child->next != NULL && child->next->key!=NULL&&strcmp(key, child->next->key) != 0)
    {
        child = child->next;
    }
    
    if(child->next==NULL){
        return STATUS_AJSON5_NOT_FOUND_ERROR;
    }else if(child->next->key==NULL){
        return STATUS_ERROR;
    }
    new_object->key = key;
    new_object->next = child->next->next;
    AJson5_free_item(child->next);
    child->next = new_object;
    return STATUS_OK;
}

static AJson5 *AJson5_get_item(AJson5 *target, char *key)
{
    if(target==NULL||key==NULL||strlen(key)==0){
        return NULL;
    }
    AJson5 *child = target->value.Child;
    if(child->next->key==NULL)return NULL;

    while (child->next != NULL && strcmp(key, child->next->key) != 0)
    {
        child = child->next;
    }
    return child->next;
}

AJson5*GetItemByKey(AJson5 *target, char *key){
    return AJson5_get_item(target,key);
}
char *GetValueString(AJson5 *item)
{
    if (item->type == AJson5_STRING)
    {
        return item->value.Str;
    }
    return "";
}

ValueType GetValueBool(AJson5 *item)
{

    return item->type == AJson5_TRUE || item->type == AJson5_FALSE ? item->type : AJson5_EMPTY;
}

//default return value is 0
uint64_t GetValueUInt(AJson5 *item)
{
    if (item->type == AJson5_UINT64)
        return item->value.Uint;
    return 0;
}

//default return value is 0
int64_t GetValueInt(AJson5 *item)
{
    if (item->type == AJson5_INT64)
        return item->value.Int;
    return 0;
}

//default return value is 0
double GetValueDouble(AJson5 *item)
{
    if (item->type == AJson5_DOUBLE)
        return item->value.Double;
    return 0;
}

AJson5 *GetArrayItemByindex(AJson5 *item, size_t index)
{
    if(item==NULL){
        return NULL;
    }
    if (item->type == AJson5_ARRAY)
    {
        AJson5 *tmpItem = item->value.Child;
        for (size_t i = 0; i < index; i++)
        {
            if(tmpItem->next!=NULL){
                tmpItem = tmpItem->next;
            }else{
                break;//next is NULL
            }
        }
        return tmpItem->next;
    }
    return NULL;
}

char *GetValueStringByKey(AJson5 *target, char *key)
{
    AJson5 *targetItem = AJson5_get_item(target, key);
    if (targetItem == NULL)
        return "";
    return GetValueString(targetItem);
}

//Due to special dedclaration , the return value True is 1,False is 0 and no target item will return AJson5_EMPTY=2
ValueType GetBoolValueByKey(AJson5 *target, char *key)
{
    AJson5 *targetItem = AJson5_get_item(target, key);
    if (targetItem == NULL)
        return AJson5_EMPTY;
    return GetValueBool(targetItem);
}

//no target found will return value is 0
uint64_t GetUIntValueByKey(AJson5 *target, char *key)
{
    AJson5 *targetItem = AJson5_get_item(target, key);
    if (targetItem == NULL)
        return 0;
    return GetValueUInt(targetItem);
}

//no target found will return value is 0
int64_t GetIntValueByKey(AJson5 *target, char *key)
{
    AJson5 *targetItem = AJson5_get_item(target, key);
    if (targetItem == NULL)
        return 0;
    return GetValueInt(targetItem);
}

//no target found will return value is 0
double GetDoubleValueByKey(AJson5 *target, char *key)
{
    AJson5 *targetItem = AJson5_get_item(target, key);
    if (targetItem == NULL)
        return 0;
    return GetValueDouble(targetItem);
}

//no target found will return value is 0,what ever the number type ,it will be transformed to double type
double GetNumberValueByKey(AJson5 *target, char *key)
{
    AJson5 *targetItem = AJson5_get_item(target, key);
    if (targetItem == NULL)
        return 0;

    switch (targetItem->type)
    {
    case AJson5_UINT64:
        return (double)GetValueUInt(targetItem);
        break;
    case AJson5_DOUBLE:
        return GetValueDouble(targetItem);
        break;
    case AJson5_INT64:
        return (double)GetValueInt(targetItem);
    default:
        break;
    }
    return 0;

}


static parse_buffer *buffer_skip_whitespace(parse_buffer *buffer)
{
    if ((buffer == NULL) || (buffer->content == NULL))
        return NULL;
    if (cannot_access_at_index(buffer, 0))
        return buffer;

    do
    {
        // skip the whitespace(ascii code <=32)
        while (buffer_at_offset(buffer)[0] <= 32)
            ++buffer->offset;

        // skip the // comments
        if (can_access_at_index(buffer, 1) && (!strncmp(buffer_at_offset(buffer), "//", 2)))
        {
            while (can_access_at_index(buffer, 0) && (buffer_at_offset(buffer)[0] != '\n'))
            {
                ++buffer->offset;
            }

            ++buffer->offset; // skip \n
        }
        /*
            skip  multiline comments
        */
        else if (can_access_at_index(buffer, 1) && (!strncmp(buffer_at_offset(buffer), "/*", 2)))
        {
            // skip "/*   */"
            buffer->offset += 2;
            while (can_access_at_index(buffer, 1) && strncmp(buffer_at_offset(buffer), "*/", 2))
            {
                ++buffer->offset;
            }
            buffer->offset += 2; // skip */
        }

        /*
            e.g."    \n    xxx"
        */
        while (buffer_at_offset(buffer)[0] <= 32)
            ++buffer->offset;

    } while (buffer_at_offset(buffer)[0] <= 32 || !strncmp(buffer_at_offset(buffer), "/*", 2) || !strncmp(buffer_at_offset(buffer), "//", 2)); // no empty no /* no //

    // FIXME: I dont know when will it be used
    if (buffer->offset == buffer->length)
    {
        --buffer->offset;
    }
    return buffer;
}

static FuncStat parse_number(AJson5 *item, parse_buffer *input_buffer)
{

    char number_str[PARSE_NUMBER_LENGTH] = {0};
    size_t end_index = 0;
    flag_t withPoint = 0; // a number with point will forcely translate to double
    for (end_index = 0; end_index < ((sizeof(number_str) / sizeof(char)) - 1) && can_access_at_index(input_buffer, end_index); end_index++)
    {
        switch (buffer_at_offset(input_buffer)[end_index])
        {
        case '.':
            withPoint = 1;
        case 'i': // i I n N for inf
        case 'I':
        case 'n': // and NaN
        case 'N':
        /*  normal    */
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '+':
        case '-':
        case 'a':
        case 'A':
        case 'b':
        case 'B':
        case 'c':
        case 'C':
        case 'd':
        case 'D':
        case 'e':
        case 'E': // for 1e10 or 0xe  1E10
        case 'f':
        case 'F':
        case 'x': // 0x
        case 'X': // 0X
            number_str[end_index] = buffer_at_offset(input_buffer)[end_index];
            break;
        default:
            goto loop_end;
            break;
        }
    }

loop_end:
    char *number_str_end = NULL;
    number_str[end_index] = '\0';
    double result_number = strtod(number_str, &number_str_end);
    if (result_number == (uint64_t)result_number && !withPoint)
    {
        // number is uint or > 0
        item->type = AJson5_UINT64;
        item->value.Uint = (uint64_t)result_number;
    }
    else if (result_number == (int64_t)result_number && !withPoint)
    {
        // number is int or < 0
        item->type = AJson5_INT64;
        item->value.Uint = (int64_t)result_number;
    }
    else
    {
        // the number is double float number
        item->type = AJson5_DOUBLE;
        item->value.Double = result_number;
    }
    input_buffer->offset += number_str_end - number_str;
    return STATUS_OK;
}

static FuncStat AJson5_parse_object(AJson5 *item, parse_buffer *input_buffer)
{
    item->type = AJson5_OBJECT;
    item->value.Child = new_item();



    AJson5 *value_item = NULL;
    // this function may need a string key without quotes parse function
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '}'))
    {
        goto success; // empty
    }
    if (cannot_access_at_index(input_buffer, 0))
    {
        input_buffer->offset--;
        goto fail;
    }
    input_buffer->offset++;
    buffer_skip_whitespace(input_buffer);

    // empty object
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '}'))
    {
        goto success;
    }

    if (cannot_access_at_index(input_buffer, 0))
    {
        input_buffer->offset--;
        goto fail;
    }
    input_buffer->offset--;
    do
    {

        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);

        // for the status of {{},} in json5
        if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '}'))
        {
            goto success;
        }

        //move it to here for the json string end and goto success but it has been created a new item;
        //fix memory leak
        value_item = new_item();
        if (AJson5_parse_string(value_item, input_buffer) != STATUS_OK)
        {
            if (AJson5_parse_special_key_string(value_item, input_buffer))
            {
                goto fail;
            }
        }
        buffer_skip_whitespace(input_buffer);

        // parse string changed the value and we move it to key
        value_item->key = value_item->value.Str;
        value_item->value.Str = NULL;

        if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != ':'))
        {
            goto fail;
        }
        input_buffer->offset++; // skip `:`
        buffer_skip_whitespace(input_buffer);

        if (AJson5_parse_value(value_item, input_buffer) != STATUS_OK)
        {
            goto fail;
        }
        //object key has been added
        AJson5_add_item_to_target(item, NULL, value_item);
        buffer_skip_whitespace(input_buffer);
    } while (can_access_at_index(input_buffer, 0) 
    && (buffer_at_offset(input_buffer)[0] == ','));
    if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != '}'))
    {
        goto fail; /* expected end of object */
    }
success:

    input_buffer->offset++;
    return STATUS_OK;

fail:
    // if (value_item != NULL)
    // {
    //     AJson5_free_item(item->value.Child);
    //     item->value.Child=NULL;
    //     value_item=NULL;
    // }
        AJson5_free_item(item->value.Child);
        item->value.Child=NULL;
    return STATUS_ERROR;
}

static FuncStat AJson5_parse_string(AJson5 *item, parse_buffer *input_buffer)
{

    char *output = NULL;
    char *input_pointer = NULL;
    if (buffer_at_offset(input_buffer)[0] != '\"' && buffer_at_offset(input_buffer)[0] != '\'')
    {
        goto fail;
    }

    flag_t is_double_quote = buffer_at_offset(input_buffer)[0] == '\"' ? 1 : 0;

    size_t skipped_chars = 0;
    input_pointer = buffer_at_offset(input_buffer) + 1;
    char *input_end = buffer_at_offset(input_buffer) + 1;

    while (((input_end - input_buffer->content) < input_buffer->length))
    {

        // support other quote in ont type quote
        if (is_double_quote && input_end[0] == '\"')
        {
            break;
        }
        else if (!is_double_quote && input_end[0] == '\'')
        {
            break;
        }

        // jump over the escape char
        if (input_end[0] == '\\' && input_end[1] != '\n') // !='\n' means \ char and \n , at that time dont escape and forgive the escape char
        {
            if ((input_end + 1 - input_buffer->content) >= input_buffer->length)
            {
                goto fail;
            }
            skipped_chars++;
            input_end++;
        }
        input_end++;
    }

    // string ended unexceptedly(not ' or ")
    if (((input_end - input_buffer->content) >= input_buffer->length) || (input_end[0] != '\"' && input_end[0] != '\''))
    {
        goto fail;
    }

    size_t allocate_length = (size_t)(input_end - buffer_at_offset(input_buffer) - skipped_chars);
    output = (char *)calloc(sizeof(char), allocate_length + sizeof("") / sizeof(char));
    char *output_pointer = output;
    while (input_pointer < input_end)
    {
        if (*input_pointer != '\\')
        {
            *output_pointer++ = *input_pointer++;
        }
        else
        {
            if ((input_end - input_pointer) < 1) // the escape char is in the front of string end char
            {
                goto fail;
            }
            switch (input_pointer[1])
            {
            case 'b':
                *output_pointer++ = '\b';
                break;
            case 'f':
                *output_pointer++ = '\f';
                break;
            case 'n':
                *output_pointer++ = '\n';
                break;
            case 'r':
                *output_pointer++ = '\r';
                break;
            case 't':
                *output_pointer++ = '\t';
                break;
            case '\"':
            case '\\':
            case '/':
            case '\'':
            case '\n': // for `\` to continue line
                *output_pointer++ = input_pointer[1];
                break;
            default:
                goto fail;
            }
            input_pointer += 2;
        }
    }
    *output_pointer = '\0';
    item->type = AJson5_STRING;
    item->value.Str = (char *)output;
    input_buffer->offset = input_end - input_buffer->content;
    input_buffer->offset++;
    return STATUS_OK;

fail:
    if (output != NULL)
    {
        free(output);
    }
    if (input_pointer != NULL)
    {
        input_buffer->offset = input_pointer - input_buffer->content; // 2 means quotes of string
    }
    return STATUS_ERROR;
}

// this method support json5 object key string that not includes quotes
static FuncStat AJson5_parse_special_key_string(AJson5 *item, parse_buffer *input_buffer)
{
    char *input_end = buffer_at_offset(input_buffer);
    char *input_pointer = buffer_at_offset(input_buffer);

    while (input_end[0] != ':' && input_end[0] > 32)
    {
        input_end++;
    }
    size_t allocate_length = (size_t)(input_end - buffer_at_offset(input_buffer));
    char *output = (char *)calloc(sizeof(char), allocate_length + sizeof("") / sizeof(char));
    char *output_pointer = output;
    while (input_pointer < input_end)
    {
        *output_pointer++ = *input_pointer++;
    }
    *output_pointer = '\0';
    item->type = AJson5_STRING;
    item->value.Str = output;
    input_buffer->offset = input_end - input_buffer->content;
    // input_buffer->offset++;
    return STATUS_OK;
}

static FuncStat AJson5_parse_array(AJson5 *item, parse_buffer *input_buffer)
{

    item->type = AJson5_ARRAY;
    item->value.Child = new_item();
    AJson5 *tmpItem = NULL;
    if (buffer_at_offset(input_buffer)[0] != '[')
    {
        goto fail;
    }
    input_buffer->offset++;
    buffer_skip_whitespace(input_buffer);
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ']'))
    { // if "[]""
        goto success;
    }
    if (cannot_access_at_index(input_buffer, 0))
    {
        input_buffer->offset--;
        goto fail;
    }

    input_buffer->offset--; // back to the front char of the array
    do
    {

        input_buffer->offset++; // ignore comma
        buffer_skip_whitespace(input_buffer);
        if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ']'))
        { // the status of `[,   ]`
            goto success;
        }

        tmpItem = new_item();
        if (AJson5_parse_value(tmpItem, input_buffer) != STATUS_OK)
        {
            goto fail;
        }
        buffer_skip_whitespace(input_buffer);
        AJson5_add_item_to_array(item, tmpItem);

    } while (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ','));

    if (cannot_access_at_index(input_buffer, 0) &&
        (buffer_at_offset(input_buffer)[0] != ']'))
    {
        goto fail;
    }

success:
    input_buffer->offset++;

    return STATUS_OK;
fail:
    if (tmpItem != NULL)
    {
        free(tmpItem);
    }
    return STATUS_ERROR;
}

// source from cjson parse_value
static FuncStat AJson5_parse_value(AJson5 *item, parse_buffer *input_buffer)
{
    if ((input_buffer == NULL) || (input_buffer->content == NULL))
        return STATUS_ERROR;

    if (can_read(input_buffer, 4) && (strncasecmp(buffer_at_offset(input_buffer), "null", 4) == 0))
    {
        item->type = AJson5_NULL;
        input_buffer->offset += 4;
        return STATUS_OK;
    }
    else if (can_read(input_buffer, 5) && (strncasecmp(buffer_at_offset(input_buffer), "false", 5) == 0))
    {
        item->type = AJson5_FALSE;
        input_buffer->offset += 5;
        return STATUS_OK;
    }
    else if (can_read(input_buffer, 4) && (strncasecmp(buffer_at_offset(input_buffer), "true", 4) == 0))
    {
        item->type = AJson5_TRUE;
        input_buffer->offset += 4;
        return STATUS_OK;
    }
    else if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '\"' || buffer_at_offset(input_buffer)[0] == '\''))
    {
        return AJson5_parse_string(item, input_buffer);
    }
    else if ((can_access_at_index(input_buffer, 0) && ((buffer_at_offset(input_buffer)[0] == '-') || (buffer_at_offset(input_buffer)[0] == '+') || (buffer_at_offset(input_buffer)[0] == '.') || ((buffer_at_offset(input_buffer)[0] >= '0') && (buffer_at_offset(input_buffer)[0] <= '9'))))

             || (can_access_at_index(input_buffer, 3) &&
                 (!strncasecmp(buffer_at_offset(input_buffer), "inf", 3) || !strncasecmp(buffer_at_offset(input_buffer), "nan", 3))))
    {
        return parse_number(item, input_buffer);
    }
    else if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '['))
    {
        return AJson5_parse_array(item, input_buffer);
    }
    else if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '{'))
    {

        return AJson5_parse_object(item, input_buffer);
    }
    return STATUS_ERROR;
}
AJson5 *LoadFromString(char *str,size_t str_length)
{
    parse_buffer buffer = {0};
    AJson5 *item = new_item();
    buffer.content = str;
    buffer.length = str_length+1;
    buffer.offset = 0;
    if(AJson5_parse_value(item, &buffer)!=STATUS_OK){
        AJson5_free_item(item);
        item=NULL;
    };
    return item;
}

static FuncStat AJson5_format_value(AutoGrowthBuffer *buf, AJson5 *target)
{

    switch (target->type)
    {
    case AJson5_FALSE:
        AGB_Replace(buf, "false");
        break;
    case AJson5_TRUE:
        AGB_Replace(buf, "true");
        break;
    case AJson5_NULL:
        AGB_Replace(buf, "null");
        break;
    case AJson5_ARRAY:
        return AJson5_format_array(buf, target);
        break;
    case AJson5_OBJECT:
        return AJson5_format_object(buf, target);
        break;
    case AJson5_DOUBLE:
    case AJson5_INT64:
    case AJson5_UINT64:
        return AJson5_format_number(buf, target);
        break;
    case AJson5_STRING:
        return AJson5_format_string(buf, target);
        break;
    default:
        AGB_Clear(buf);
        return STATUS_ERROR;
        break;
    }
    return STATUS_OK;
}

static FuncStat AJson5_format_object(AutoGrowthBuffer *buf, AJson5 *target)
{
    // char child_buf[40960] = {0};
    AutoGrowthBuffer child_buf;
    AGB_Init(&child_buf);
    // size_t i = 0;
    AGB_Append(buf, "{");

    AJson5 *child = target->value.Child;
    while (child->next != NULL)
    {
        child = child->next;

        AGB_Append(buf, "\"");
        AGB_Append(buf, child->key);
        // i += strlen(child->key);
        AGB_Append(buf, "\"");
        AGB_Append(buf, ": ");

        // value format
        AJson5_format_value(&child_buf, child);
        AGB_Append(buf, child_buf.array);
        AGB_Append(buf, ", ");
        AGB_Clear(&child_buf);
    }
    // delete redundant `, `
    AGB_DeleteNLast(buf, 2);
    AGB_Append(buf, "}");

    AGB_Release(&child_buf);
    return STATUS_OK;
}

static FuncStat AJson5_format_array(AutoGrowthBuffer *buf, AJson5 *target)
{
    // char s[40960] = {0};
    AutoGrowthBuffer child_buf;
    AGB_Init(&child_buf);
    // size_t i = 0;
    AGB_Append(buf, "[");
    AJson5 *child = target->value.Child;
    while (child->next != NULL)
    {
        child = child->next;
        AJson5_format_value(&child_buf, child);
        AGB_Append(buf, child_buf.array);
        AGB_Append(buf, ", ");
        AGB_Clear(&child_buf);
    }
    // delete redundant `, `
    AGB_DeleteNLast(buf, 2);
    AGB_Append(buf, "]");
    
    AGB_Release(&child_buf);

    return STATUS_OK;
}
static FuncStat AJson5_format_string(AutoGrowthBuffer *buf, AJson5 *target)
{
    if (target->type == AJson5_STRING)
    {

        // AGB_Clear(buf);
        AGB_Append(buf, "\"");

        ////for escape the "
        char *value_buffer = (char *)calloc(2 * strlen(target->value.Str) / sizeof(char),sizeof(char));
        char *the_value = target->value.Str;
        size_t value_end = 0, value_buffer_end = 0;
        while (the_value[value_end] != '\0')
        {
            switch(the_value[value_end])
            {
                case '\"':
                case '\\':
                    value_buffer[value_buffer_end++] = '\\';
                    break;
                //should I esacpe \n?
                // case '\n':  //escape '\n' to "\\n" 
                //     value_buffer[value_buffer_end++] = '\\';
                //     value_buffer[value_buffer_end++] = 'n';
                //     value_end++;//skip the \n
                //     break;
                default:
                    break;
            }
            value_buffer[value_buffer_end++] = the_value[value_end++];

        }

        AGB_Append(buf, value_buffer);
        AGB_Append(buf, "\"");
        free(value_buffer);
        return STATUS_OK;
    }
    return STATUS_ERROR;
}
static FuncStat AJson5_format_number(AutoGrowthBuffer *buf, AJson5 *target)
{

    char tmpBuf[64] = {0};
    switch (target->type)
    {
    case AJson5_DOUBLE:
        sprintf(tmpBuf, "%.8lf", target->value.Double);
        break;
    case AJson5_INT64:
        sprintf(tmpBuf, "%ld", target->value.Int);
        break;
    case AJson5_UINT64:
        sprintf(tmpBuf, "%ld", target->value.Uint);
        break;
    }
    AGB_Replace(buf, tmpBuf);
    return STATUS_OK;
}

AJson5 *CreateObject()
{
    return AJson5_create_object();
}




AJson5 *CreateArray()
{
    return AJson5_create_array();
}

//when target is array,the key should be NULL
FuncStat AddItemToTarget(AJson5 *target, char *key, AJson5 *item)
{
    return AJson5_add_item_to_target(target, key, item);
}

//dumplicate object or array
FuncStat Dumplicate(char *str, AJson5 *target)
{
    AutoGrowthBuffer tmpbuffer;
    AGB_Init(&tmpbuffer);

    switch (target->type)
    {
    case AJson5_OBJECT:
        AJson5_format_object(&tmpbuffer, target);
        break;
    case AJson5_ARRAY:
        AJson5_format_array(&tmpbuffer, target);
        break;
    default:
        return STATUS_ERROR;
        break;
    }
    strcpy(str, tmpbuffer.array);
    AGB_Release(&tmpbuffer);
    return STATUS_OK;
}
