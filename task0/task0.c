#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h> // Prototype for basename() function
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <elf.h>

char debugMode =0;
char fileName[100];
char* fileInMemory;
int currFD = -1;
Elf32_Ehdr* hdr = NULL;

void cleanUp(){
    if(currFD>0)close(currFD);
    //if(fileInMemory != NULL)free(fileInMemory);
}
char *stripNewLine(char *str)
{
    int i = 0;
    while (str[i] != '\n')
        i++;
    str[i] = '\0';
    return str;
}
int getHexaInput(int maxStrLen)
{
    int output;
    char str[maxStrLen]; //8 for address, 1 for \n, 1 for \0
    fgets(str, maxStrLen, stdin);
    stripNewLine(str);
    sscanf(str,"%x",&output);
    return output;
}
int getDeciInput(int maxStrLen)
{
    char str[maxStrLen];
    fgets(str, maxStrLen, stdin);
    stripNewLine(str);
    return atoi(str);
}

void flushStdin()
{
    char temp;
    while ((temp != EOF) & (temp != '\n'))
        temp = fgetc(stdin);
}

void quit()
{
    cleanUp(); 
    exit(0);
}
void toggleDebug()
{
    debugMode = (-1 * debugMode) + 1;
    printf("Debug mode is now %s\n", debugMode ? "ON" : "OFF");
}
void setFileName()
{
    printf("file name: ");
    fgets(fileName, 100, stdin);
    stripNewLine(fileName);
    if (debugMode)
        if(debugMode)printf("(DEBUG: set file name to %s)\n", fileName);
}

void examineElfFile(){
    int closeRes = currFD==-1? -2 : close(currFD);
    if(debugMode)printf("(DEBUG: closing file %s returned %d)\n",fileName,closeRes);
    setFileName();
    currFD = open(fileName,O_RDONLY);
    if(debugMode)printf("(DEBUG: opening file %s returned %d)\n",fileName,currFD);
    struct stat s;a
    if(fstat(currFD, &s) == -1)printf("could not get file length!\n");
    if(debugMode)printf("(DEBUG: file size = 0x%lX)\n",s.st_size);
    fileInMemory = mmap(NULL,s.st_size,PROT_READ,MAP_PRIVATE,currFD,0);
    hdr = fileInMemory;

    printf("magic numbers: %02X %02X %02X\n",hdr->e_ident[0],hdr->e_ident[1],hdr->e_ident[2]);
    printf("Entry Point:   %X\n",hdr->e_entry);
}


struct MenuItem
{
    char *name;
    void (*fun)();
};
const struct MenuItem menu[] = {
    {"Toggle Debug", toggleDebug},
    {"examine ELF File", examineElfFile},
    {"Quit", quit},
    {NULL, NULL}};

void printFileBytes(char* fileName){
    FILE* file = fopen(fileName,"rb");
    int i=0;
    while((i = fgetc(file))!= EOF){
        printf("%02x ",i);
    }
    fclose(file);
}
//entry at byte 24
int main(int argc, char **argv)
{
    int lowOption = 0;
    int highOption = (sizeof(menu) / sizeof(struct MenuItem)) - 1;
    struct MenuItem menuItem;
    while (1)
    {
        printf("Please choose a function:\n");
        for (int i = 0;; i++)
        {
            menuItem = menu[i];
            if (menuItem.name == NULL)
                break;
            printf("%d) %s\n", i, menuItem.name);
        }
        int choice = fgetc(stdin) - '0';
        flushStdin();
        if (!(choice >= lowOption) | !(choice < highOption))
            printf("Not within bounds\n");
        else
            menu[choice].fun();
    }
}