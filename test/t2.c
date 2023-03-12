#include "../AJson5/AJson5.h"
#include <stdio.h>
#include <stdlib.h>

int main(){
    AJson5* mainObj=CreateObject();
    AddStringToObject(mainObj,"firstName","John");
    AddStringToObject(mainObj,"lastName","Smith");
    AddTrueToObject(mainObj,"isAlive");
    AddNumberToObject(mainObj,"age",27);

    AJson5* addressObj=CreateObject();
    AddStringToObject(addressObj,"streetAddress","21 2nd Street");
    AddStringToObject(addressObj,"city","New York");
    AddStringToObject(addressObj,"state","NY");
    AddStringToObject(addressObj,"postalCode","10021-3100");
    AddObjectToObject(mainObj,"address",addressObj);
    
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

    AJson5*phoneNumbersArr=LoadFromString(parse_string);
    AddArrayToObject(mainObj,"phoneNumbers",phoneNumbersArr);
    char*childrenArr[16]={"Catherine","Thomas","Trevor"};
    AddStringArrayToObject(mainObj,"children",3,childrenArr);
    AddNullToObject(mainObj,"spouse");
    char op[2048]={0};
    Dumplicate(op,mainObj);
    puts(op);

}