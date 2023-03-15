#include "../AJson5/AJson5.h"
#include <stdio.h>
#include <stdlib.h>

int main(){
    AJson5* mainObj=CreateObject();
    AJson5 *widgetObj=CreateObject();
    AddStringToObjectByKey(widgetObj,"debug","on");
    AJson5*windowObj=CreateObject();
    AddStringToObjectByKey(windowObj,"title","sample Konfabulator Widget");
    AddNumberToObjectByKey(windowObj,"width",500);
    AddNumberToObjectByKey(windowObj,"height",500);
    AddItemToTarget(widgetObj,"window",windowObj);
    AJson5*menuObj = CreateObject();
    AddStringToObjectByKey(menuObj,"header","SVG Viewer");
    AJson5*itemsArray =CreateArray();
    AJson5*tmpObj = CreateObject();
    AddStringToObjectByKey(tmpObj,"id","Open");
    AddItemToTarget(itemsArray,NULL,tmpObj);
    AddItemToTarget(menuObj,"items",itemsArray);
    tmpObj = CreateObject();
    AddStringToObjectByKey(tmpObj,"id","Open");
    AddStringToObjectByKey(tmpObj,"label","Open New");

    


    char b[10240]={0};
    Dumplicate(b,menuObj);
    puts(b);

}