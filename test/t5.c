#include "../AJson5/AJson5.h"
#include <stdio.h>
#include <stdlib.h>
int main(){
    FILE*fp;
    fp=fopen("t5.json","r");
    fseek( fp , 0 , SEEK_END );
    int file_size;
    file_size = ftell( fp );
    // printf( "%d" , file_size );
    char *tmp;
    fseek( fp , 0 , SEEK_SET);
    tmp =  (char *)malloc( file_size * sizeof( char ) );
    fread( tmp , file_size/sizeof( char) , sizeof(char) , fp);
    puts(tmp);
    AJson5*o=LoadFromString(tmp,file_size);
    if(o==NULL){
        puts("error occurred when parsing!");
        exit(0);
    }
    char buf[40960]={0};
    Dumplicate(buf,o);
    puts( buf);
    Clear(o);
    free(tmp);
    fclose(fp);
    return 0;
}