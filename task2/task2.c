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

char debugMode = 0;
char fileName[100];
char fileInMemory[100000];
int currFD = -1;
Elf32_Ehdr *hdr = NULL;

void cleanUp()
{
    if (currFD > 0)
        close(currFD);
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
    sscanf(str, "%x", &output);
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
        if (debugMode)
            printf("(DEBUG: set file name to %s)\n", fileName);
}

void examineElfFile()
{
    int closeRes = currFD == -1 ? -2 : close(currFD);
    if (debugMode)
        printf("(DEBUG: closing file %s returned %d)\n", fileName, closeRes);
    setFileName();
    currFD = open(fileName, O_RDONLY);
    if (debugMode)
        printf("(DEBUG: opening file %s returned %d)\n", fileName, currFD);
    struct stat s;
    if ((fstat(currFD, &s) == -1) | (currFD == -1))
    {
        printf("error opening or mapping file! aborting action\n");
        currFD = -1;
        return;
    }
    if (debugMode)
        printf("(DEBUG: file size = 0x%lX)\n", s.st_size);
    memcpy(fileInMemory, mmap(NULL, s.st_size, PROT_READ, MAP_PRIVATE, currFD, 0), 10000);
    hdr = fileInMemory;

    printf("MAGIC Nunmbers: %02X %02X %02X\n", hdr->e_ident[0], hdr->e_ident[1], hdr->e_ident[2]);
    printf("Entry Point:               0x%08X\t(deci: %d)\n", hdr->e_entry, hdr->e_entry);
    printf("Start  of Section Headers: 0x%08X\t(deci: %d)\n", hdr->e_shoff, hdr->e_shoff);
    printf("Number of Section Headers: 0x%08X\t(deci: %d)\n", hdr->e_shnum, hdr->e_shnum);
    printf("Section Header Entry Size: 0x%08X\t(deci: %d)\n", hdr->e_shentsize, hdr->e_shentsize);
    printf("Start  of Program Headers: 0x%08X\t(deci: %d)\n", hdr->e_phoff, hdr->e_phoff);
    printf("Number of Program Headers: 0x%08X\t(deci: %d)\n", hdr->e_phnum, hdr->e_phnum);
    printf("Program Header Entry Size: 0x%08X\t(deci: %d)\n", hdr->e_phentsize, hdr->e_phentsize);
}
//print its index, address, offset, size in bytes, type number, name
//referenced from stack overflow
void printSectionNames()
{
    printf("Sections:\n");
    printf("idx\tName\t\t   addr\t\toffset\tsize\ttype\n");
    Elf32_Shdr *sectionHeaders = (Elf32_Shdr *)(fileInMemory + hdr->e_shoff);
    Elf32_Shdr *sectionHeaderStringTable = &sectionHeaders[hdr->e_shstrndx];
    char *names = fileInMemory + sectionHeaderStringTable->sh_offset;

    for (int i = 0; i < hdr->e_shnum; ++i)
    {
        Elf32_Shdr shdr = sectionHeaders[i];
        printf("%2d) %-15s\t %8X\t %X\t %X\t %d\n",
               i, names + shdr.sh_name, shdr.sh_addr, shdr.sh_offset, shdr.sh_size, shdr.sh_type);
    }
}

void printSymbols()
{
    printf("Symbols:\n");
    printf("\tName\t\t\t\t\tSection\n");
    Elf32_Shdr *section = (Elf32_Shdr*)(fileInMemory+hdr->e_shoff);
    char *sectionNames = (char *)(fileInMemory + section[hdr->e_shstrndx].sh_offset);

    for(int i=0; i<hdr->e_shnum; i++) {
        if((section[i].sh_type==SHT_SYMTAB)|(section[i].sh_type==SHT_DYNSYM)) {
            Elf32_Sym *symtab = (Elf32_Sym *)(fileInMemory+section[i].sh_offset);
            int symbolNum = section[i].sh_size/section[i].sh_entsize;
            char *symbolNames = (char *)(fileInMemory + section[section[i].sh_link].sh_offset);
            for (int j=0; j<symbolNum; j++) {
                char* name = symbolNames + symtab[j].st_name;
                if(name[0]!='\0') printf("%d)\t%-25s\t\t%s\n", i,name,sectionNames + section[i].sh_name);
            }
        }
    }
}

void printSymbols2()
{
    printf("Symbols:\n");
    printf("idx\tName\t\t   addr\t\toffset\tsize\ttype\n");
    Elf32_Shdr *sectionHeaders = (Elf32_Shdr *)(fileInMemory + hdr->e_shoff);
    Elf32_Shdr *sectionHeaderStringTable = &sectionHeaders[hdr->e_shstrndx];
    Elf32_Shdr *sections = (Elf32_Shdr *)((char *)fileInMemory + hdr->e_shoff);
    char *names = fileInMemory + sectionHeaderStringTable->sh_offset;
    Elf32_Sym* symbolTable = NULL;   
    int i;
    for (i = 0; i < hdr->e_shnum; i++)
        if (sections[i].sh_type == SHT_SYMTAB) {
            symbolTable = (Elf32_Sym *)((char *)fileInMemory + sections[i].sh_offset);
            break; 
        }
    for(i=0;i<100;i++){
        Elf32_Sym symbol=symbolTable[i];
        printf("%s\n",names+symbol.st_name);
    }   
}

struct MenuItem
{
    char *name;
    void (*fun)();
};
const struct MenuItem menu[] = {
    {"Toggle Debug", toggleDebug},
    {"examine ELF File", examineElfFile},
    {"Print Section Names", printSectionNames},
    {"Print Symbols", printSymbols},
    {"Quit", quit},
    {NULL, NULL}};

void printFileBytes(char *fileName)
{
    FILE *file = fopen(fileName, "rb");
    int i = 0;
    while ((i = fgetc(file)) != EOF)
    {
        printf("%02x ", i);
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