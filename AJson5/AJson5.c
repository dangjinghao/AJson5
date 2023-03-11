#include <stdlib.h>
#include <stdio.h>
#include "AJson5.h"
#include <string.h>
// just for learning
// maybe I should create a hook to suport custom alloc function like cJSON

// declaration
static AJson5 *new_item();
static FuncStat FreeItem(AJson5 *target);
static parse_buffer *buffer_skip_whitespace(parse_buffer *buffer);

static FuncStat parse_value(AJson5 *item, parse_buffer *input_buffer);
static FuncStat parse_string(AJson5 *item, parse_buffer *input_buffer);
static FuncStat parse_array(AJson5 *item, parse_buffer *input_buffer);
static FuncStat parse_object(AJson5 *item, parse_buffer *input_buffer);
static FuncStat parse_json5_key_string(AJson5 *item, parse_buffer *input_buffer);
static FuncStat parse_value(AJson5 *item, parse_buffer *input_buffer);
// done

static AJson5 *new_item()
{
    AJson5 *item = (AJson5 *)calloc(1, sizeof(AJson5));
    item->type = AJson5_EMPTY;
    item->next = NULL;
    return item;
}

AJson5 *CreateNull()
{
    AJson5 *item = new_item();

    item->type = AJson5_NULL;

    return item;
}

AJson5 *CreateBool(ValueType t)
{
    AJson5 *item = new_item();

    item->type = (t == AJson5_TRUE ? AJson5_TRUE : AJson5_FALSE);
    return item;
}

/*
 * only support UINT64 or INT64
 * default UINT64
 */

AJson5 *CreateInt(ValueType t, int64_t num)
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

AJson5 *CreateDouble(double num)
{
    AJson5 *item = new_item();
    item->type = AJson5_DOUBLE;
    item->value.Double = num;

    return item;
}

AJson5 *CreateString(char *s)
{
    AJson5 *item = new_item();
    item->type = AJson5_STRING;
    item->value.Str = s;

    return item;
}
AJson5 *CreateObject()
{
    AJson5 *item = new_item();
    item->type = AJson5_OBJECT;

    // empty item to make sure the same opertate

    item->value.Child = new_item();
    return item;
}
AJson5 *CreateArray()
{
    AJson5 *item = new_item();
    item->type = AJson5_ARRAY;
    item->value.Child = new_item();
    return item;
}
FuncStat AddItemToObject(
    AJson5 *target,
    char *key,
    AJson5 *item)
{
    item->key = key;

    AJson5 *child = target->value.Child;
    while (child->next != NULL)
        child = child->next;

    child->next = item;

    return STATUS_OK;
}

FuncStat AddItemToArray(
    AJson5 *target,
    AJson5 *item)
{
    AJson5 *child = target->value.Child;
    while (child->next != NULL)
        child = child->next;
    child->next = item;

    return STATUS_OK;
}
AJson5 *CreateNumber(double num)
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

FuncStat AddNullToObject(AJson5 *target, char *key)
{
    return AddItemToObject(target, key, CreateNull());
}

FuncStat AddTrueToObject(AJson5 *target, char *key)
{
    return AddItemToObject(target, key, CreateBool(AJson5_TRUE));
}

FuncStat AddFalseToObject(AJson5 *target, char *key)
{
    return AddItemToObject(target, key, CreateBool(AJson5_FALSE));
}

FuncStat AddStringToObject(AJson5 *target, char *key, char *val)
{
    return AddItemToObject(target, key, CreateString(val));
}

FuncStat AddNumberToObject(AJson5 *target, char *key, double val)
{
    return AddItemToObject(target, key, CreateNumber(val));
}

AJson5 *CreateNumberArray(size_t len, double nums[])
{
    AJson5 *array = CreateArray();
    AJson5 *tmpObj = array->value.Child;

    for (size_t i = 0; i < len; i++)
    {
        tmpObj->next = CreateNumber(nums[i]);
        tmpObj = tmpObj->next;
    }
    return array;
}
FuncStat AddNumberArrayToObject(AJson5 *target, char *key, size_t n, double nums[])
{
    return AddItemToObject(target, key, CreateNumberArray(n, nums));
}

AJson5 *CreateStringArray(size_t len, char *vals[])
{
    AJson5 *array = CreateArray();
    AJson5 *tmpObj = array->value.Child;

    for (size_t i = 0; i < len; i++)
    {
        tmpObj->next = CreateString(vals[i]);
        tmpObj = tmpObj->next;
    }
    return array;
}

FuncStat AddStringArrayToObject(AJson5 *target, char *key, size_t n, char *vals[])
{
    return AddItemToObject(target, key, CreateStringArray(n, vals));
}

FuncStat InsertItemToArray(AJson5 *target, size_t n, AJson5 *item)
{

    AJson5 *childItem = target->value.Child;
    for (size_t i = 0; i < n && childItem->next != NULL; i++)
        childItem = childItem->next;

    item->next = childItem->next;
    item->next = item;
    return STATUS_OK;
}

FuncStat DeleteItemFromArray(AJson5 *src, size_t subscript)
{
    AJson5 *item = src->value.Child;
    for (size_t i = 0; i < subscript && item->next != NULL; i++)
        item = item->next;
    AJson5 *targetItem = item->next;
    item->next = targetItem->next;
    free(targetItem);
    return STATUS_OK;
}

// free the item's child's next
static FuncStat FreeItem(AJson5 *target)
{
    AJson5 *tmpObj = target->value.Child->next;
    // free the single link list or json object's child
    while (tmpObj != NULL)
    {
        switch (tmpObj->type)
        {
        case AJson5_OBJECT:
            FreeItem(tmpObj);
            break;
        case AJson5_ARRAY:
            // free the single link list in json array
            AJson5 *arrayItem = tmpObj->value.Child;
            while (arrayItem != NULL)
            {
                AJson5 *arrayNext = arrayItem->next;
                free(arrayItem);
                arrayItem = arrayNext;
            }
            break;
        default:
            // basic type
            break;
        }
        AJson5 *next = tmpObj->next;
        free(tmpObj);
        tmpObj = next;
    }
    target->value.Child->next = NULL;
    return STATUS_OK;
}
FuncStat DeleteItemFromObject(AJson5 *src, char *key)
{
    AJson5 *item = src->value.Child;
    while (item->next != NULL && strcmp(item->next->key, key) != 0)
    {
        item = item->next;
    }
    if (item->next != NULL)
    {
        AJson5 *next = item->next->next;
        FreeItem(item->next);
        item->next = next;
        return STATUS_OK;
    }
    return STATUS_NOT_FOUND_ERROR;
}

FuncStat Clear(AJson5 *target)
{
    // only leave the child item
    FreeItem(target);

    return STATUS_OK;
}

FuncStat ReplaceItemInArray(AJson5 *target, size_t subscript, AJson5 *new_item)
{

    AJson5 *child = target->value.Child;
    for (size_t i = 0; i < subscript; i++)
    {
        child = child->next;
    }

    new_item->next = child->next->next;
    free(child->next);
    child->next = new_item;
    return STATUS_OK;
}

FuncStat ReplaceItemInObject(AJson5 *target, char *key, AJson5 *new_object)
{
    AJson5 *child = target->value.Child;
    while (child->next != NULL && strcmp(key, child->next->key) != 0)
    {
        child = child->next;
    }
    new_object->key = key;
    new_object->next = child->next->next;
    free(child->next);
    child->next = new_object;
    return STATUS_OK;
}

AJson5 *GetItem(AJson5 *target, char *key)
{
    AJson5 *child = target->value.Child;
    while (child->next != NULL && strcmp(key, child->next->key) != 0)
    {
        child = child->next;
    }
    return child->next;
}

char *GetStringValue(AJson5 *item)
{
    if (item->type == AJson5_STRING)
    {
        return item->value.Str;
    }
    return "";
}

ValueType GetBoolValue(AJson5 *item)
{

    return item->type == AJson5_TRUE || item->type == AJson5_FALSE ? item->type : AJson5_EMPTY;
}

uint64_t GetUIntValue(AJson5 *item)
{
    if (item->type == AJson5_UINT64)
        return item->value.Uint;
    return 0;
}

int64_t GetIntValue(AJson5 *item)
{
    if (item->type == AJson5_INT64)
        return item->value.Int;
    return 0;
}

double GetDoubleValue(AJson5 *item)
{
    if (item->type == AJson5_DOUBLE)
        return item->value.Double;
    return 0;
}

AJson5 *GetItemFromArray(AJson5 *item, size_t n)
{
    if (item->type == AJson5_ARRAY)
    {
        AJson5 *tmpItem = item->value.Child;
        for (size_t i = 0; i < n; i++)
        {
            tmpItem = tmpItem->next;
        }
        return tmpItem->next;
    }
    return NULL;
}

char *GetItemStringValue(AJson5 *target, char *key)
{
    AJson5 *targetItem = GetItem(target, key);
    if (targetItem == NULL)
        return "";
    return GetStringValue(targetItem);
}

ValueType GetItemBoolValue(AJson5 *target, char *key)
{
    AJson5 *targetItem = GetItem(target, key);
    if (targetItem == NULL)
        return AJson5_EMPTY;
    return GetBoolValue(targetItem);
}

uint64_t GetItemUIntValue(AJson5 *target, char *key)
{
    AJson5 *targetItem = GetItem(target, key);
    if (targetItem == NULL)
        return 0;
    return GetUIntValue(targetItem);
}

int64_t GetItemIntValue(AJson5 *target, char *key)
{
    AJson5 *targetItem = GetItem(target, key);
    if (targetItem == NULL)
        return 0;
    return GetIntValue(targetItem);
}

double GetItemDoubleValue(AJson5 *target, char *key)
{
    AJson5 *targetItem = GetItem(target, key);
    if (targetItem == NULL)
        return 0;
    return GetDoubleValue(targetItem);
}

double GetItemNumberValue(AJson5 *target, char *key)
{
    AJson5 *targetItem = GetItem(target, key);
    switch (targetItem->type)
    {
    case AJson5_UINT64:
        return (double)GetUIntValue(targetItem);
        break;
    case AJson5_DOUBLE:
        return GetDoubleValue(targetItem);
        break;
    case AJson5_INT64:
        return (double)GetIntValue(targetItem);
    default:
        return 0;
        break;
    }
}

static parse_buffer *buffer_skip_whitespace(parse_buffer *buffer)
{
    if ((buffer == NULL) || (buffer->content == NULL))
        return NULL;
    if (cannot_access_at_index(buffer, 0))
        return buffer;

    // skip the whitespace(ascii code <=32)
    while (can_access_at_index(buffer, 0) && (buffer_at_offset(buffer)[0] <= 32))
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
    else if (
        can_access_at_index(buffer, 1) && (!strncmp(buffer_at_offset(buffer), "/*", 2)))
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
    while (can_access_at_index(buffer, 0) && (buffer_at_offset(buffer)[0] <= 32))
        ++buffer->offset;

    // UDRSTD: I dont know when will it be used
    if (buffer->offset == buffer->length)
    {
        --buffer->offset;
    }
    return buffer;
}

static FuncStat parse_number(AJson5 *item, parse_buffer *input_buffer)
{
    double num = 0;
    char number_str[64] = {0};
    char *str_end = NULL;
    size_t i = 0;
    char withPoint = 0; // a number with point will forcely translate to double
    for (i = 0; i < ((sizeof(number_str) / sizeof(char)) - 1) && can_access_at_index(input_buffer, i); i++)
    {
        switch (buffer_at_offset(input_buffer)[i])
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
            number_str[i] = buffer_at_offset(input_buffer)[i];
            break;
        default:
            goto loop_end;
            break;
        }
    }

loop_end:
    number_str[i] = '\0';
    num = strtod(number_str, &str_end);
    if (num == (uint64_t)num && !withPoint)
    {
        // number is uint or > 0
        item->type = AJson5_UINT64;
        item->value.Uint = (uint64_t)num;
    }
    else if (num == (int64_t)num && !withPoint)
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
    input_buffer->offset += str_end - number_str;
    return STATUS_OK;
}

static FuncStat parse_object(AJson5 *item, parse_buffer *input_buffer)
{
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
        AJson5 *tmp_item = new_item();
        if (tmp_item == NULL)
        {
            goto fail;
        }

        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);

        // for the status of {{},} in json5
        if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '}'))
        {
            goto success;
        }

        if (parse_string(tmp_item, input_buffer) != STATUS_OK)
        {
            if (parse_json5_key_string(tmp_item, input_buffer))
            {
                goto fail;
            }
        }
        buffer_skip_whitespace(input_buffer);

        // parse string changed the value and we move it to key
        tmp_item->key = tmp_item->value.Str;
        tmp_item->value.Str = NULL;

        if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != ':'))
        {
            goto fail;
        }
        input_buffer->offset++; // skip `:`
        buffer_skip_whitespace(input_buffer);

        if (parse_value(tmp_item, input_buffer) != STATUS_OK)
        {
            goto fail;
        }
        AddItemToObject(item, tmp_item->key, tmp_item);
        buffer_skip_whitespace(input_buffer);
    } while (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ','));

    if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != '}'))
    {
        goto fail; /* expected end of object */
    }
success:
    item->type = AJson5_OBJECT;
    input_buffer->offset++;
    return STATUS_OK;

fail:
    return STATUS_ERROR;
}

static FuncStat parse_string(AJson5 *item, parse_buffer *input_buffer)
{

    char *output = NULL;
    char *input_pointer = NULL;
    if (buffer_at_offset(input_buffer)[0] != '\"' && buffer_at_offset(input_buffer)[0] != '\'')
    {
        goto fail;
    }

    char is_double_quote = buffer_at_offset(input_buffer)[0] == '\"' ? 1 : 0;

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

    size_t alloc_length = (size_t)(input_end - buffer_at_offset(input_buffer) - skipped_chars);
    output = (char *)malloc(alloc_length + sizeof(""));
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
static FuncStat parse_json5_key_string(AJson5 *item, parse_buffer *input_buffer)
{
    char *input_end = buffer_at_offset(input_buffer);
    char *input_pointer = buffer_at_offset(input_buffer);

    while (input_end[0] != ':' && input_end[0] > 32)
    {
        input_end++;
    }
    size_t alloc_length = (size_t)(input_end - buffer_at_offset(input_buffer));
    char *output = (char *)malloc(alloc_length + sizeof(""));
    char *output_pointer = output;
    while (input_pointer < input_end)
    {
        *output_pointer++ = *input_pointer++;
    }
    *output_pointer = '\0';
    item->type = AJson5_STRING;
    item->value.Str = (char *)output;
    input_buffer->offset = input_end - input_buffer->content;
    // input_buffer->offset++;
    return STATUS_OK;
}

static FuncStat parse_array(AJson5 *item, parse_buffer *input_buffer)
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
        if (parse_value(tmpItem, input_buffer) != STATUS_OK)
        {
            goto fail;
        }
        buffer_skip_whitespace(input_buffer);
        AddItemToArray(item, tmpItem);

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
static FuncStat parse_value(AJson5 *item, parse_buffer *input_buffer)
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
        input_buffer += 5;
        return STATUS_OK;
    }
    else if (can_read(input_buffer, 4) && (strncasecmp(buffer_at_offset(input_buffer), "true", 4) == 0))
    {
        item->type = AJson5_TRUE;
        input_buffer += 4;
        return STATUS_OK;
    }
    else if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '\"' || buffer_at_offset(input_buffer)[0] == '\''))
    {
        return parse_string(item, input_buffer);
    }
    else if ((can_access_at_index(input_buffer, 0) && ((buffer_at_offset(input_buffer)[0] == '-') || (buffer_at_offset(input_buffer)[0] == '+') || (buffer_at_offset(input_buffer)[0] == '.') || ((buffer_at_offset(input_buffer)[0] >= '0') && (buffer_at_offset(input_buffer)[0] <= '9'))))

             || (can_access_at_index(input_buffer, 3) &&
                 (!strncasecmp(buffer_at_offset(input_buffer), "inf", 3) || !strncasecmp(buffer_at_offset(input_buffer), "nan", 3))))
    {
        return parse_number(item, input_buffer);
    }
    else if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '['))
    {
        return parse_array(item, input_buffer);
    }
    else if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '{'))
    {
        return parse_object(item, input_buffer);
    }
    return STATUS_ERROR;
}
AJson5 *ParseWithLength(char *str, size_t length)
{
    parse_buffer buffer = {0};
    AJson5 *item = CreateObject();
    buffer.content = str;
    buffer.length = length;
    buffer.offset = 0;
    parse_object(item, &buffer);
    return item;
}

FuncStat FormatValue(char *buf, AJson5 *target)
{

    switch (target->type)
    {
    case AJson5_FALSE:
        strncpy(buf, "false", 6);
        break;
    case AJson5_TRUE:
        strncpy(buf, "true", 5);
        break;
    case AJson5_NULL:
        strncpy(buf, "null", 5);
        break;
    case AJson5_ARRAY:
        return FormatArray(buf, target);
        break;
    case AJson5_OBJECT:
        return FormatObject(buf, target);
        break;
    case AJson5_DOUBLE:
    case AJson5_INT64:
    case AJson5_UINT64:
        return FormatNumbers(buf, target);
        break;
    case AJson5_STRING:
        return FormatString(buf, target);
        break;
    default:
        *buf = '\0';
        break;
    }
    return STATUS_OK;
}

FuncStat FormatObject(char *buf, AJson5 *target)
{
    char s[1024] = {0};
    char child_buf[2048] = {0};
    size_t i = 0;
    s[i++] = '{';

    AJson5 *child = target->value.Child;
    while (child->next != NULL)
    {
        child = child->next;

        // key format
        s[i++] = '\"';
        strcpy(s + i, child->key);
        i += strlen(child->key);
        s[i++] = '\"';
        strncpy(s + i, ": ", 2);
        i += 2;

        // value format
        FormatValue(child_buf, child);
        strcpy(s + i, child_buf);
        i += strlen(child_buf);
        strncpy(s + i, ", ", 2);
        i += 2;
        memset(child_buf, 0, sizeof(child_buf));
    }
    s[i++] = '}';

    strncpy(buf, s, i);
}

FuncStat FormatArray(char *buf, AJson5 *target)
{
    char s[1024] = {0};
    char child_buf[512] = {0};
    size_t i = 0;
    s[i++] = '[';

    AJson5 *child = target->value.Child;
    while (child->next != NULL)
    {
        child = child->next;
        FormatValue(child_buf, child);
        strcpy(s + i, child_buf);
        i += strlen(child_buf);
        strncpy(s + i, ", ", 2);
        i += 2;
    }
    s[i++] = ']';
    strncpy(buf, s, i);
    return STATUS_OK;
}
FuncStat FormatString(char *buf, AJson5 *target)
{
    if (target->type == AJson5_STRING)
    {
        buf[0] = '\"';
        strcpy(buf + 1, target->value.Str);
        buf[strlen(target->value.Str) + 1] = '\"';
        return STATUS_OK;
    }
    return STATUS_ERROR;
}
FuncStat FormatNumbers(char *buf, AJson5 *target)
{
    switch (target->type)
    {
    case AJson5_DOUBLE:
        sprintf(buf, "%.8lf", target->value.Double);
        break;
    case AJson5_INT64:
        sprintf(buf, "%ld", target->value.Int);
        break;
    case AJson5_UINT64:
        sprintf(buf, "%ld", target->value.Uint);
        break;
    }
    return STATUS_OK;
}

