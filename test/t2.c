#include "../AJson5/AJson5.h"
#include <stdio.h>
#include <stdlib.h>

int main(){
    AJson5* mainObj=CreateObject();
    AddStringToObjectByKey(mainObj,"firstName","John");
    AddStringToObjectByKey(mainObj,"lastName","Smith");
    AddTrueToObjectByKey(mainObj,"isAlive");
    AddNumberToObjectByKey(mainObj,"age",27);

    AJson5* addressObj=CreateObject();
    AddStringToObjectByKey(addressObj,"streetAddress","21 2nd Street");
    AddStringToObjectByKey(addressObj,"city","New York");
    AddStringToObjectByKey(addressObj,"state","NY");
    AddStringToObjectByKey(addressObj,"postalCode","10021-3100");
    AddItemToTarget(mainObj,"address",addressObj);
    
    char*parse_string ="[\
      {\
        \"type\": \"home\",\
        \"number\": \"212 555-1234\"\
      },\
      {\
        \"type\": \"office\",\
        \"number\": \"646 555-4567\"\
      }\
    ]";

    AJson5*phoneNumbersArr=LoadFromString(parse_string,strlen(parse_string));
    AddItemToTarget(mainObj,"phoneNumbers",phoneNumbersArr);
    char*childrenArr[16]={"Catherine","Thomas","Trevor"};
    AddStringArrayToTargetByKey(mainObj,"children",3,childrenArr);
    AddNullToObjectByKey(mainObj,"spouse");
    char op[2048]={0};
    Dumplicate(op,mainObj);
    Clear(mainObj);
    puts(op);

}