#include "headers/myBigChars.h"
#include "headers/myTerm.h"
#include "headers/SimpleComputer.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int bc0[2] = {1717992960, 8283750};
int bc1[2] = {471341056, 3938328};
int bc2[2] = {538983424, 3935292};
int bc3[2] = {2120252928, 8282238};
int bc4[2] = {2120640000, 6316158};
int bc5[2] = {2114092544, 8273984};
int bc6[2] = {33701376, 4071998};
int bc7[2] = {811630080, 396312};
int bc8[2] = {2120646144, 8283750};
int bc9[2] = {2087074816, 3956832};
int bcA[2] = {2118269952, 4342338};
int bcB[2] = {1044528640, 4080194};
int bcC[2] = {37895168, 3949058};
int bcD[2] = {1111637504, 4080194};
int bcE[2] = {2114092544, 8258050};
int bcF[2] = {33717760, 131646};

int bcPlus[2] = {2115508224, 1579134};
int bcMinus[2] = {2113929216, 126};

//Works like dictionary for big chars in range 0-16
int *bcdictionary[16] = {bc0, bc1, bc2, bc3, bc4, bc5, bc6, bc7,
                         bc8, bc9, bcA, bcB, bcC, bcD, bcE, bcF};

int bc_printA(char *str)
{
    printf("\E(0%s\E(B", str);
    return 0;
}

int bc_box(int x1, int y1, int x2, int y2)
{
    int rows, cols; //количество строк и столбцов в терминале
    mt_getscreensize(&rows, &cols);

    if (x1 >= x2 || y1 >= y2 || x1 < 0 || y1 < 0 || x2 > cols || y2 > rows)
        return WRONG_COMMAND;

    mt_gotoXY(x1, y1);
    bc_printA(TOP_L_CORNER);
    //Минус 1 так как еще рисуются углы
    for (int i = 0; i < x2 - x1 - 1; i++)
        bc_printA(HORIZONTAL);
    bc_printA(TOP_R_CORNER);
    // +1 так как еще рисуются углы
    for (int i = y1 + 1; i < y2; i++)
    {
        mt_gotoXY(x1, i);
        bc_printA(VERTICAL);
        mt_gotoXY(x2, i);
        bc_printA(VERTICAL);
    }

    mt_gotoXY(x1, y2);
    bc_printA(BOTTOM_L_CORNER);
    //Минус 1 так как еще рисуются углы
    for (int i = 0; i < x2 - x1 - 1; i++)
        bc_printA(HORIZONTAL);
    bc_printA(BOTTOM_R_CORNER);

    return 0;
}

int bc_printbigchar(int big[2], int x, int y, enum color fgcolor, enum color bgcolor)
{
    int rows, cols; //количество строк и столбцов в терминале
    mt_getscreensize(&rows, &cols);

    if (x < 0 || y < 0 || x + 8 > cols || y + 8 > rows)
        return WRONG_COMMAND;

    mt_setfgcolor(fgcolor);
    mt_setbgcolor(bgcolor);

    int bit = 0;        //номер бита который нужна напечатать
    int digit = big[0]; //число, которое нужно вывести

    for (int i = 0; i < 8; i++)
    {
        mt_gotoXY(x, y + i);
        for (int j = 0; j < 8; j++)
        {
            if (bit == 32)
            {
                bit = 0;
                digit = big[1];
            }

            if (((digit >> bit) & 1) == 0)
                printf(" ");
            else
                bc_printA(CAGE);
            bit++;
        }
    }

    mt_setfgcolor(Default);
    mt_setbgcolor(Default);

    return 0;
}

int bc_setbigcharpos(int *big, int x, int y, int value)
{
    if (x < 0 || y < 0 || x > 8 || y > 8 || (value != 0 && value != 1))
        return WRONG_COMMAND;

    int index;

    if (y <= 4)
        index = 0;
    else
        index = 1;

    if (y > 4)
        y -= 4;

    if (value == 0)
        big[index] &= ~(1 << (((y - 1) * 8) + (x - 1)));
    else
        big[index] |= 1 << (((y - 1) * 8) + (x - 1));

    return 0;
}

int bc_getbigcharpos(int *big, int x, int y, int *value)
{
    if (x < 0 || y < 0 || x > 8 || y > 8)
        return WRONG_COMMAND;

    int index;

    if (y <= 4)
        index = 0;
    else
        index = 1;

    if (y > 4)
        y -= 4;

    *value = (big[index] >> (((y - 1) * 8) + (x - 1))) & 1;

    return 0;
}

int bc_bigcharwrite(int fd, int *big, int count)
{
    if (write(fd, &count, sizeof(count)) == -1)
        return -1;
    if (write(fd, big, count * sizeof(int) * 2) == -1)
        return -1;

    return 0;
}

int bc_bigcharread(int fd, int *big, int need_count, int *count)
{
    int n, readen, err;

    err = read(fd, &n, sizeof(n));
    if (err == -1 || (err != sizeof(n)))
        return -1;
    readen = read(fd, big, need_count * sizeof(int) * 2);
    if (readen == -1)
        return -1;
    *count = readen / (sizeof(int) * 2);

    return 0;
}