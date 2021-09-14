#include "headers/sat.h"

int commands[] = {0, 0x10, 0x11, 0x20, 0x21, 0x30, 0x31, 0x32, 0x33, 0x40,
                  0x41, 0x42, 0x43, 0x75};

int translateSAtoBIN(char *filenameSA, char *filenameOUT)
{
    FILE *sourceFile = fopen(filenameSA, "r");

    if (sourceFile == NULL)
        return 1;

    int memory[COMPUTER_MEM_SIZE];

    for (int i = 0; i < COMPUTER_MEM_SIZE; i++)
        memory[i] = 0;

    int memAddress, command, operand, encodedCommand;

    //5 - максимальная длина комманды из рассматриваемых в курсе
    char strCommand[5];

    while (fscanf(sourceFile, "%d %[=A-Z]", &memAddress, strCommand) != EOF)
    {
        TrimRight(strCommand);

        if (strcmp(strCommand, "") == 0)
            continue;

        if (memAddress < 0 || memAddress > COMPUTER_MEM_SIZE)
            return 1;

        if (strcmp(strCommand, "=") == 0)
        {
            if (fscanf(sourceFile, "%x", &encodedCommand) != 1)
                return 1;
        }
        else
        {
            if (fscanf(sourceFile, "%d", &operand) != 1)
                return 1;

            command = str_cmnd_to_int(strCommand);

            if (command == -1)
                return 1;

            if (commandEncode(command, operand, &encodedCommand) != 0)
                return 1;
        }

        if (encodedCommand < 0 || encodedCommand > 0x3FFF)
            printf("Mem error");
        memory[memAddress] = encodedCommand;

        skipComment(sourceFile);
    }

    fclose(sourceFile);

    makeBINfile(filenameOUT, memory);

    return 0;
}

int CheckFilesExtensions(char *filenameSA, char *filenameOUT)
{
    return IsStrEndsWith(filenameSA, ".sa") && IsStrEndsWith(filenameOUT, ".o");
}

int IsStrEndsWith(char *base, char *str)
{
    int blen = strlen(base);
    int slen = strlen(str);

    return (blen >= slen) && (0 == strcmp(base + blen - slen, str));
}

int str_cmnd_to_int(const char *command)
{
    if (strcmp(command, "READ") == 0)
        return READ;
    else if (strcmp(command, "WRITE") == 0)
        return WRITE;
    else if (strcmp(command, "LOAD") == 0)
        return LOAD;
    else if (strcmp(command, "STORE") == 0)
        return STORE;
    else if (strcmp(command, "ADD") == 0)
        return ADD;
    else if (strcmp(command, "SUB") == 0)
        return SUB;
    else if (strcmp(command, "DIVIDE") == 0)
        return DIVIDE;
    else if (strcmp(command, "MUL") == 0)
        return MUL;
    else if (strcmp(command, "JUMP") == 0)
        return JUMP;
    else if (strcmp(command, "JNEG") == 0)
        return JNEG;
    else if (strcmp(command, "JZ") == 0)
        return JZ;
    else if (strcmp(command, "HALT") == 0)
        return HALT;
    else if (strcmp(command, "ADDC") == 0)
        return ADDC;
    else
        return -1;
}

int commandEncode(int command, int operand, int *value)
{
    if (!isCommandExist(command))
        return 1;

    *value = (command << 7) | operand;

    return 0;
}

int isCommandExist(int command)
{
    //13 - количество возможных команд
    int *searchResult = (int *)bsearch(&command, &commands, 14,
                                       sizeof(int), compare);

    return !(searchResult == NULL);
}

int compare(const void *a, const void *b)
{
    if (*(int *)a < *(int *)b)
        return -1;
    else if (*(int *)a == *(int *)b)
        return 0;
    else
        return 1;
}

int skipComment(FILE *sourceFile)
{
    int readResult;
    do
    {
        readResult = fgetc(sourceFile);
    } while (readResult != '\n' && readResult != EOF);

    return 0;
}

int makeBINfile(char *filename, int memory[])
{
    FILE *file = fopen(filename, "wb");
    fwrite(memory, sizeof(int), COMPUTER_MEM_SIZE, file);
    fclose(file);

    return 0;
}