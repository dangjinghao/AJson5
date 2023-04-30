#include "../AJson5/AJson5.h"
#include <stdio.h>
#include <stdlib.h>
int main(){
    FILE*fp;
    fp=fopen("t4.json","r");
    fseek( fp , 0 , SEEK_END );
    int file_size;
    file_size = ftell( fp );
    // printf( "%d" , file_size );
    char *tmp;
    fseek( fp , 0 , SEEK_SET);
    tmp =  (char *)malloc( file_size * sizeof( char ) );
    fread( tmp , file_size/sizeof( char) , sizeof(char) , fp);
    // puts(tmp);
    AJson5*o=LoadFromString(tmp,file_size);
    if(o==NULL){
        puts("error occurred when parsing!");
        exit(0);
    }
    //not test
    AJson5*web_app_val=GetItemByKey(o,"web-app");
    AJson5*servlet_val=GetItemByKey(web_app_val,"servlet");
    AJson5*item5=GetArrayItemByindex(servlet_val,4000);
    AJson5*init_param_val=GetItemByKey(item5,"init-param");

    char*fileTransferFolder=GetValueStringByKey(init_param_val,"fileTransferFolder");
    int b=GetBoolValueByKey(init_param_val,"betaServer");
    DeleteItemFromObjectByKey(item5,"servlet-name");
    DeleteItemFromObjectByKey(item5,"init-param");
    DeleteItemFromArrayByIndex(servlet_val,1);
    InsertItemToArrayByIndex(servlet_val,1,CreateNumber(1));
    // DeleteItemFromArrayByIndex(servlet_val,5);
    ReplaceItemInArrayByIndex(servlet_val,1,CreateTrue());
    // ReplaceItemInObjectByKey(o,"3",CreateNull());

    char buf[40960]={0};
    Dumplicate(buf,o);
    puts(buf);
    // puts(fileTransferFolder);
    // printf("%d\n",b);
    //get in here
    Clear(o);
    fclose(fp);
    free(tmp);
    return 0;
}