#include "headers/sbt.h"

int freeMemCellIndex = 0;
int bufferMemCell;
char freeVarName = 'Z';

int translateSBtoSA(char *filenameSB, char *filenameOUT)
{
    char *result = (char *)(malloc(2048 * sizeof(char)));
    FILE *sourceFile = fopen(filenameSB, "r");

    if (sourceFile == NULL)
        return 1;

    makeGOTOqueue(sourceFile);
    int lineNumber;
    char *command = (char *)(malloc(256 * sizeof(char)));
    nodeCL *searchCL;

    while (fscanf(sourceFile, "%d %[=A-Z 0-9<>=+/*-]",
                  &lineNumber, command) != EOF)
    {
        TrimRight(command);

        if (strcmp(command, "") == 0)
            continue;

        searchCL = findCL(lineNumber);

        if (searchCL != NULL)
            searchCL->memCellIndex = freeMemCellIndex;

        strcat(result, SBCmndToSA(command));
    }
    fclose(sourceFile);

    searchCL = clqueue.head;

    //Проверяем корректность адрессов goto
    while (searchCL != NULL)
    {
        //lineNumber - номер последней строчки в файле
        if (searchCL->CodeLineNumber < 0 ||
            searchCL->CodeLineNumber > lineNumber)
            return 1;

        searchCL = searchCL->next;
    }

    if (clqueue.tail->CodeLineNumber > lineNumber)
        return 1;

    pasteGOTOAdrresses(result);
    pasteVaribalesAddresses(result);

    filenameSB[strlen(filenameSB) - 1] = 'a';

    FILE *foutput = fopen(filenameSB, "w");

    if (foutput == NULL)
        return 1;

    fwrite(result, sizeof(char), strlen(result), foutput);
    fclose(foutput);

    translateSAtoBIN(filenameSB, filenameOUT);

    return freeMemCellIndex < 0 || freeMemCellIndex > 99;
}

char *SBCmndToSA(char *command)
{
    if (IsStrStartsWith(command, "REM"))
        return cmndREM(command);
    else if (IsStrStartsWith(command, "INPUT"))
        return cmndINPUT(command);
    else if (IsStrStartsWith(command, "OUTPUT"))
        return cmndOUTPUT(command);
    else if (IsStrStartsWith(command, "GOTO"))
        return cmndGOTO(command);
    else if (IsStrStartsWith(command, "IF"))
        return cmndIF(command);
    else if (IsStrStartsWith(command, "LET"))
        return cmndLET(command);
    else if (IsStrStartsWith(command, "END"))
        return cmndEND(command);
    else
        return NULL;
}

int IsStrStartsWith(char *a, char *b)
{
    return (strncmp(a, b, strlen(b)) == 0);
}

char *cmndREM(char *cmnd)
{
    //пропускаем строку REM
    while (*cmnd != ' ')
    {
        cmnd++;
    }

    char *result = (char *)(malloc((strlen(cmnd) + 1 + 3) * sizeof(char)));
    sprintf(result, "%.2d ;", freeMemCellIndex);
    strcat(result, cmnd);

    freeMemCellIndex++;
    return result;
}

char *cmndINPUT(char *cmnd)
{
    while (*cmnd != ' ')
    {
        cmnd++;
    }
    cmnd++; //Это будет имя переменной

    char *result =
        (char *)(malloc((strlen("READ vc\n") + strlen(cmnd) + 3) * sizeof(char)));
    sprintf(result, "%.2d READ v%c\n", freeMemCellIndex, *cmnd);

    if (!IsVarQueueContains(*cmnd))
    {
        nodeVar *newnode = (nodeVar *)(malloc(sizeof(nodeVar)));
        newnode->name = *cmnd;
        addToVarQueue(newnode);
    }

    freeMemCellIndex++;
    return result;
}

char *cmndOUTPUT(char *cmnd)
{
    while (*cmnd != ' ')
    {
        cmnd++;
    }
    cmnd++; //Это будет имя переменной

    char *result =
        (char *)(malloc((strlen("WRITE vc\n") + strlen(cmnd) + 3) * sizeof(char)));
    sprintf(result, "%.2d WRITE v%c\n", freeMemCellIndex, *cmnd);

    if (!IsVarQueueContains(*cmnd))
    {
        nodeVar *newnode = (nodeVar *)(malloc(sizeof(nodeVar)));
        newnode->name = *cmnd;
        addToVarQueue(newnode);
    }

    freeMemCellIndex++;
    return result;
}

char *cmndGOTO(char *cmnd)
{
    while (*cmnd != ' ')
    {
        cmnd++;
    }
    cmnd++; //Это будет имя переменной

    char *result =
        (char *)(malloc(((strlen("JUMP ") + strlen(cmnd) + 3)) * sizeof(char)));
    sprintf(result, "%.2d JUMP %d\n", freeMemCellIndex, atoi(cmnd));

    freeMemCellIndex++;
    return result;
}

// if a = b
// if a > b
// if a < b
char *cmndIF(char *cmnd)
{
    char *result = (char *)(malloc(512 * sizeof(char)));
    char buffer[256];

    while (*cmnd != ' ')
    {
        cmnd++;
    }
    cmnd++; //Попадаем на 'a'
    char a, b, mathsymbol;
    int ai, bi, aMemIndex, bMemIndex; //a integer, b integer
    aMemIndex = bMemIndex = -1;
    a = *cmnd;
    ai = atoi(cmnd);

    cmnd += 2; //Попадаем на знак < или = или >

    mathsymbol = *cmnd;

    cmnd += 2; //Попадаем на 'b'

    b = *cmnd;
    bi = atoi(cmnd);

    //Значит a - const
    if (a == '0' || ai != 0)
    {
        sprintf(buffer, "%.2d = %d\n", freeMemCellIndex, ai);
        aMemIndex = freeMemCellIndex;
        freeMemCellIndex++;
        strcat(result, buffer);
    }

    //Значит b - const
    if (b == '0' || bi != 0)
    {
        sprintf(buffer, "%.2d = %d\n", freeMemCellIndex, bi);
        bMemIndex = freeMemCellIndex;
        freeMemCellIndex++;
        strcat(result, buffer);
    }

    char *nextCommands = (char *)(malloc(512 * sizeof(char)));
    cmnd += 2; //указывает на начало следующей команды

    if (mathsymbol == '<')
    {
        if (bMemIndex != -1)
            sprintf(buffer, "%.2d LOAD %.2d\n", freeMemCellIndex, bMemIndex);
        else
            sprintf(buffer, "%.2d LOAD v%c\n", freeMemCellIndex, b);

        strcat(result, buffer);
        freeMemCellIndex++;

        if (aMemIndex != -1)
            sprintf(buffer, "%.2d SUB %.2d\n", freeMemCellIndex, aMemIndex);
        else
            sprintf(buffer, "%.2d SUB v%c\n", freeMemCellIndex, a);

        strcat(result, buffer);
        freeMemCellIndex++;

        //LOAD B(или конкретный адресс)
        //SUB A(или конкретный адресс)

        freeMemCellIndex++;
        sprintf(nextCommands, "%s\n", SBCmndToSA(cmnd));
        sprintf(buffer, "%.2d JNEG %d\n", freeMemCellIndex - 2, freeMemCellIndex);
        strcat(result, buffer);
        strcat(result, nextCommands);
    }
    else
    {
        if (aMemIndex != -1)
            sprintf(buffer, "%.2d LOAD %.2d\n", freeMemCellIndex, aMemIndex);
        else
            sprintf(buffer, "%.2d LOAD v%c\n", freeMemCellIndex, a);

        strcat(result, buffer);
        freeMemCellIndex++;

        if (bMemIndex != -1)
            sprintf(buffer, "%.2d SUB %.2d\n", freeMemCellIndex, bMemIndex);
        else
            sprintf(buffer, "%.2d SUB %c\n", freeMemCellIndex, b);

        strcat(result, buffer);
        freeMemCellIndex++;

        //LOAD B(или конкретный адресс)
        //SUB A(или конкретный адресс)

        if (mathsymbol == '>')
        {
            freeMemCellIndex++;
            sprintf(nextCommands, "%s", SBCmndToSA(cmnd));
            sprintf(buffer, "%.2d JNEG %d\n", freeMemCellIndex - 1, freeMemCellIndex);
            strcat(result, buffer);
            strcat(result, nextCommands);
        }
        else if (mathsymbol == '=')
        {
            sprintf(buffer, "%.2d JZ %.2d\n", freeMemCellIndex, freeMemCellIndex + 2);
            strcat(result, buffer);
            freeMemCellIndex++;

            freeMemCellIndex++;
            sprintf(nextCommands, "%s", SBCmndToSA(cmnd));
            sprintf(buffer, "%.2d JUMP %d\n", freeMemCellIndex - 2, freeMemCellIndex);
            strcat(result, buffer);
            strcat(result, nextCommands);
        }
    }

    return result;
}

char *cmndLET(char *cmnd)
{
    while (*cmnd != ' ')
        cmnd++;

    cmnd++; //Это будет имя переменной которой присваивается значение
    char variableName = *cmnd;

    while (*cmnd != '=')
        cmnd++;
    cmnd += 2; //Начало выражения

    char buffer[256];
    char *result = (char *)(malloc(1024 * sizeof(char)));
    char *operationSymbol;
    char rpn[strlen(cmnd)]; //reverse polish notation
    if (strlen(cmnd) == 1)
        sprintf(rpn, "%d", atoi(cmnd));
    else
        translate_to_rpn(rpn, cmnd);
    int AddressBuffer[strlen(rpn) / 2];
    int bufferIndex = -1;
    size_t rpnindex = 0;
    int iter = countComma(rpn) + 1;

    for (int i = 0; i < iter && iter != 1; i++)
    {
        if (isMoreThan2InARow(&rpn[rpnindex]))
        {
            storeCache(result, rpn);
            bufferIndex++;
            AddressBuffer[bufferIndex] = freeMemCellIndex;
            freeMemCellIndex++;

            while (rpn[rpnindex] != ',' && rpn[rpnindex] != '\000' && rpn[rpnindex] != 'n')
                rpnindex++;
            if (rpnindex < strlen(rpn))
                rpnindex++; //Указывает на следующую переменную
        }
        else
        {

            if (is2InARow(&rpn[rpnindex]))
            {
                //Имеем дело с константой
                if (rpn[rpnindex] == '0' || atoi(&rpn[rpnindex]) != 0)
                {
                    sprintf(buffer, "%.2d = %d\n", freeMemCellIndex,
                            atoi(&rpn[rpnindex]));
                    bufferIndex++;
                    AddressBuffer[bufferIndex] = freeMemCellIndex;
                    freeMemCellIndex++;

                    sprintf(buffer, "%.2d LOAD %.2d\n", freeMemCellIndex,
                            freeMemCellIndex - 1);
                    strcat(result, buffer);
                    freeMemCellIndex++;
                }
                else
                {
                    sprintf(buffer, "%.2d LOAD v%c\n", freeMemCellIndex,
                            rpn[rpnindex]);
                    strcat(result, buffer);
                    freeMemCellIndex++;
                }

                while (rpn[rpnindex] != ',' && rpn[rpnindex] != '\000' && rpn[rpnindex] != 'n')
                    rpnindex++;
                if (rpnindex < strlen(rpn))
                    rpnindex++; //Указывает на следующую переменную
                i++;

                operationSymbol = getOperationSymbol(&rpn[rpnindex]);
                //Имеем дело с константой
                if (rpn[rpnindex] == '0' || atoi(&rpn[rpnindex]) != 0)
                {
                    sprintf(buffer, "%.2d = %d\n", freeMemCellIndex,
                            atoi(&rpn[rpnindex]));
                    bufferIndex++;
                    AddressBuffer[bufferIndex] = freeMemCellIndex;
                    freeMemCellIndex++;

                    strcat(result, buffer);

                    sprintf(buffer, "%.2d %s %.2d\n", freeMemCellIndex,
                            getStrOperation(*operationSymbol),
                            freeMemCellIndex - 1);
                    strcat(result, buffer);
                    freeMemCellIndex++;
                }
                else
                {
                    sprintf(buffer, "%.2d %s v%c\n", freeMemCellIndex,
                            getStrOperation(*operationSymbol), rpn[rpnindex]);
                    strcat(result, buffer);
                    freeMemCellIndex++;
                }

                while (rpn[rpnindex] != ',' && rpn[rpnindex] != '\000' && rpn[rpnindex] != 'n')
                    rpnindex++;
                if (rpnindex < strlen(rpn))
                    rpnindex++;
                while (rpn[rpnindex] != ',' && rpn[rpnindex] != '\000' && rpn[rpnindex] != 'n')
                    rpnindex++;
                if (rpnindex < strlen(rpn))
                    rpnindex++; //Указывает на следующую переменную
                i += 2;
            }

            if (!isalnum(rpn[rpnindex]) && rpnindex < strlen(rpn))
            {
                operationSymbol = getOperationSymbol(&rpn[rpnindex]);

                sprintf(buffer, "%.2d %s %.2d\n",
                        freeMemCellIndex, getStrOperation(*operationSymbol),
                        AddressBuffer[bufferIndex]);

                freeMemCellIndex++;

                if (bufferIndex > 0)
                    bufferIndex--;
                strcat(result, buffer);

                while (rpn[rpnindex] != ',' && rpn[rpnindex] != '\000' && rpn[rpnindex] != 'n')
                    rpnindex++;
                if (rpnindex < strlen(rpn))
                    rpnindex++; //Указывает на следующую переменную
            }
            else if (!isalnum(rpn[rpnindex + 2]) && rpnindex + 2 < strlen(rpn))
            {
                operationSymbol = getOperationSymbol(&rpn[rpnindex]);

                if (rpn[rpnindex] == '0' || atoi(&rpn[rpnindex]) != 0)
                {
                    sprintf(buffer, "%.2d = %d\n", freeMemCellIndex,
                            atoi(&rpn[rpnindex]));
                    freeMemCellIndex++;
                    strcat(result, buffer);
                    sprintf(buffer, "%.2d %s %.2d\n", freeMemCellIndex,
                            getStrOperation(*operationSymbol),
                            freeMemCellIndex - 1);
                }
                else
                    sprintf(buffer, "%.2d %s v%c\n", freeMemCellIndex,
                            getStrOperation(*operationSymbol), rpn[rpnindex]);

                freeMemCellIndex++;
                strcat(result, buffer);

                while (rpn[rpnindex] != ',' && rpn[rpnindex] != '\000' && rpn[rpnindex] != 'n')
                    rpnindex++;
                if (rpnindex < strlen(rpn))
                    rpnindex += 3; //Указывает на следующий символ после обслуженных переменную
                i++;
            }
            else if (rpnindex < strlen(rpn))
            {
                sprintf(buffer, "%.2d STORE %.2d\n\n", freeMemCellIndex,
                        freeMemCellIndex + 1);
                bufferIndex++;
                AddressBuffer[bufferIndex] = freeMemCellIndex + 1;
                freeMemCellIndex += 2;
                strcat(result, buffer);
            }
        }
    }

    if (iter == 1)
    {
        sprintf(buffer, "%.2d = %d\n", freeMemCellIndex, atoi(&rpn[rpnindex]));
        freeMemCellIndex++;
        strcat(result, buffer);
        sprintf(buffer, "%.2d LOAD %.2d\n", freeMemCellIndex, freeMemCellIndex - 1);
        strcat(result, buffer);
        freeMemCellIndex++;
    }

    sprintf(buffer, "%.2d STORE v%c\n", freeMemCellIndex, variableName);
    freeMemCellIndex++;
    strcat(result, buffer);

    if (!IsVarQueueContains(*cmnd) && isalpha(*cmnd) != 0)
    {
        nodeVar *newnode = (nodeVar *)(malloc(sizeof(nodeVar)));
        newnode->name = *cmnd;
        addToVarQueue(newnode);
    }

    return result;
}

char *cmndEND(char *cmnd)
{
    char *result =
        (char *)(malloc((strlen("dd HALT 00\n") + strlen(cmnd)) * sizeof(char)));
    sprintf(result, "%.2d HALT 00\n", freeMemCellIndex);

    freeMemCellIndex++;
    return result;
}

int storeCache(char *result, char *rpn)
{
    char buffer[14];
    //Имеем дело с const
    if (*rpn == '0' || atoi(rpn) != 0)
        sprintf(buffer, "%.2d  = %d\n", freeMemCellIndex, atoi(rpn));
    else
        sprintf(buffer, "%.2d STORE v%c\n", freeMemCellIndex, *rpn);

    strcat(result, buffer);

    return 0;
}

char *getStrOperation(char ch)
{
    if (ch == '-')
        return "SUB";
    else if (ch == '+')
        return "ADD";
    else if (ch == '/')
        return "DIVIDE";
    else if (ch == '*')
        return "MUL";
    else
        return NULL;
}

char *getOperationSymbol(char *polstr)
{
    char *ptr = polstr;

    while (*ptr == ',' || isalnum(*ptr))
        ptr++;

    return ptr;
}

int countComma(char *str)
{
    char *ptr = str;
    int counter = 0;

    while (*ptr != '\0' && *ptr != '\n')
    {
        if (*ptr == ',')
            counter++;
        ptr++;
    }

    return counter;
}

int is2InARow(char *polstr)
{
    int counter = 0;
    int rowCounter = 0;
    char *ptr = polstr;

    while (counter != 3)
    {
        if (isalnum(*ptr) != 0)
            rowCounter++;

        while (*ptr != ',' && *ptr != '\0' &&
               *ptr != '\n' && *ptr != '\000' && rowCounter != 3)
            ptr++;
        if (*ptr == ',')
            ptr++;
        else
            break;

        counter++;
    }

    return rowCounter == 2;
}

int isMoreThan2InARow(char *polstr)
{
    int counter = 0;
    int rowCounter = 0;
    char *ptr = polstr;

    while (counter != 3)
    {
        if (isalnum(*ptr) != 0)
            rowCounter++;

        while (*ptr != ',' && *ptr != '\0' &&
               *ptr != '\n' && *ptr != '\000' && rowCounter != 3)
            ptr++;
        if (*ptr == ',')
            ptr++;
        else
            break;

        counter++;
    }

    return rowCounter > 2;
}

int countConst(char *polstr)
{
    char *ptr = polstr;
    int counter = 0;

    for (size_t i = 0; i < strlen(polstr); i++, ptr++)
    {
        if (*ptr == '0' || atoi(ptr) > 0)
        {
            counter++;
            while (*ptr != ',' && *ptr != '\0' && *ptr != '\n' && *ptr != '\000')
            {
                ptr++;
                i++;
            }
        }
    }

    return counter;
}

stack_t *stack_push(stack_t *head, char a)
{
    stack_t *ptr;

    ptr = malloc(sizeof(stack_t));

    ptr->c = a;
    ptr->next = head;

    return ptr;
}

char stack_pop(stack_t **head)
{
    stack_t *ptr;
    char a;

    if (*head == NULL)
        return '\0';

    ptr = *head;
    a = ptr->c;
    *head = ptr->next;

    return a;
}

int get_prior(char c)
{
    if (c == '*')
        return 3;
    else if (c == '/')
        return 3;
    else if (c == '-')
        return 2;
    else if (c == '+')
        return 2;
    else if (c == '(')
        return 1;
    else
        return 0;
}

void translate_to_rpn(char *outstr, char *a)
{
    stack_t *opers = NULL;
    int k, point;

    k = 0;
    point = 0;
    while (a[k] != '\0' && a[k] != '\n')
    {
        if (a[k] == ')')
        {
            while ((opers->c) != '(')
            {
                outstr[point++] = stack_pop(&opers);
                outstr[point++] = ',';
            }
            stack_pop(&opers);
        }
        if ((a[k] >= 'A' && a[k] <= 'Z') || (a[k] >= '0' && a[k] <= '9'))
        {
            outstr[point++] = a[k];
            outstr[point++] = ',';
        }
        else if (a[k] == '(')
            opers = stack_push(opers, '(');
        else if (a[k] == '+' || a[k] == '-' || a[k] == '/' || a[k] == '*')
        {
            if (opers == NULL)
                opers = stack_push(opers, a[k]);
            else if (get_prior(opers->c) < get_prior(a[k]))
                opers = stack_push(opers, a[k]);
            else
            {
                while ((opers != NULL) && (get_prior(opers->c) >= get_prior(a[k])))
                {
                    outstr[point++] = stack_pop(&opers);
                    outstr[point++] = ',';
                }
                opers = stack_push(opers, a[k]);
            }
        }
        k++;
    }

    while (opers != NULL)
        outstr[point++] = stack_pop(&opers);
    outstr[point] = '\0';
}

//Var Queue Var Queue Var Queue Var Queue Var Queue Var Queue Var Queue Var Queu
int IsVarQueueContains(char varName)
{
    nodeVar *parser = varQueue.head;

    while (parser != NULL)
    {
        if (parser->name == varName)
            return 1;

        parser = parser->next;
    }

    return 0;
}

void addToVarQueue(nodeVar *var)
{
    if (varQueue.head == NULL)
    {
        varQueue.head = var;
        varQueue.tail = var;
    }
    else
    {
        varQueue.tail->next = var;
        varQueue.tail = varQueue.tail->next;
    }
}

//CL QUEUE CL QUEUE CL QUEUE CL QUEUE CL QUEUE CL QUEUE CL QUEUE CL QUEUE CL QUE
int IsCLQueueContains(int lineNumber)
{
    nodeCL *parser = clqueue.head;

    while (parser != NULL)
    {
        if (parser->CodeLineNumber == lineNumber)
            return 1;

        parser = parser->next;
    }

    return 0;
}

nodeCL *findCL(int lineNumber)
{
    nodeCL *parser = clqueue.head;

    while (parser != NULL)
    {
        if (parser->CodeLineNumber == lineNumber)
            return parser;

        parser = parser->next;
    }

    return NULL;
}

void addToCLQueue(nodeCL *var)
{
    if (clqueue.head == NULL)
    {
        clqueue.head = var;
        clqueue.tail = var;
    }
    else
    {
        clqueue.tail->next = var;
        clqueue.tail = clqueue.tail->next;
    }
}

int makeGOTOqueue(FILE *file)
{
    char str[128];
    int gotoline;
    char *GOTOstr;
    while (fgets(str, 128, file) != NULL)
    {

        GOTOstr = strstr(str, "GOTO ");

        if (GOTOstr == NULL)
            continue;

        for (size_t i = 0; i < strlen("GOTO "); i++)
            GOTOstr++;

        gotoline = atoi(GOTOstr);

        if (!IsCLQueueContains(gotoline))
        {
            nodeCL *newnode = (nodeCL *)(malloc(sizeof(nodeCL)));
            newnode->CodeLineNumber = gotoline;
            addToCLQueue(newnode);
        }
    }

    rewind(file);

    return 0;
}

void TrimRight(char *s)
{
    if (!s || !*s)
        return;       // Пустая строка
    char *nonspc = s; // Тут будет указатель на последний НЕ пробел
    for (; *s; s++)
        if (*s != ' ')
            nonspc = s; // Обновляем позицию НЕ пробела

    if (*nonspc != ' ') // Вся строка могла быть пробелами, тогда nonspc->' '
        nonspc++;       // Позиция за последним НЕ пробелом - там или пробел или конец строки
    *nonspc = 0;        // Теперь там точно конец строки
    return;
}

int pasteGOTOAdrresses(char *result)
{
    //Вставляем занчения для goto
    char *ptr, buffer[3], strsearch[10];

    nodeCL *searchCL = clqueue.head;

    while (searchCL != NULL)
    {

        sprintf(strsearch, "JUMP %d", searchCL->CodeLineNumber);

        ptr = strstr(result, strsearch);

        if (ptr == NULL)
        {
            searchCL = searchCL->next;
            continue;
        }

        sprintf(buffer, "%.2d", searchCL->memCellIndex);
        ptr += strlen("JUMP ");

        for (size_t i = 0; i < strlen(buffer); i++, ptr++)
            *ptr = buffer[i];

        char *cut = ptr;
        int counter = 0;
        while (*cut != '\n')
        {
            counter++;
            cut++;
        }
        memmove(ptr, cut, strlen(cut));
    }

    return 0;
}

int pasteVaribalesAddresses(char *result)
{
    char *ptr, buffer[3], strsearch[10];

    nodeVar *varNode = varQueue.head;

    while (varNode != NULL)
    {
        varNode->memAddress = freeMemCellIndex;
        freeMemCellIndex++;
        varNode = varNode->next;
    }

    varNode = varQueue.head;

    while (varNode != NULL)
    {
        sprintf(strsearch, "v%c", varNode->name);

        ptr = strstr(result, strsearch);

        if (ptr == NULL)
        {
            varNode = varNode->next;
            continue;
        }

        sprintf(buffer, "%.2d", varNode->memAddress);

        for (size_t i = 0; i < strlen(buffer); i++, ptr++)
            *ptr = buffer[i];
    }

    return 0;
}