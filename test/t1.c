#include "../AJson5/AJson5.h"
#include <stdio.h>
#include <stdlib.h>
int main(){
    FILE*fp;
    fp=fopen("t1.json5","r");
    fseek( fp , 0 , SEEK_END );
    int file_size;
    file_size = ftell( fp );
    // printf( "%d" , file_size );
    char *tmp;
    fseek( fp , 0 , SEEK_SET);
    tmp =  (char *)malloc( file_size * sizeof( char ) );
    fread( tmp , file_size/sizeof( char) , sizeof(char) , fp);
    puts(tmp);
    AJson5*o=ParseWithLength(tmp,file_size);
    char buf[2048]={0};
    FormatObject(buf,o);
    puts( buf);
    return 0;
}