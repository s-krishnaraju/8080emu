#include "emu.h"

int handleOpcode(unsigned char* codebuffer, int pc){
    unsigned char* code = *codebuffer[pc];
    int opbytes = 1;
    switch(code[0]){
        
    }

}


int main(int argc, char *argv[]){
    FILE* f = fopen(argv[1], "rb");

    if(f == NULL){
        printf("error: Couldn't open file %s\n", argv[1]);
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    int fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    unsigned char *buffer = malloc(fsize);
    fread(buffer,fsize, 1, f);
    fclose(f);

    int pc = 0;

    while(pc < fsize){
       pc += handleOpcode(buffer, pc);
    }
    return 0;
}
