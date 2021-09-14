#include "headers/interface.h"

cursorpos cursor;

int activate_interface()
{
    keys pressed_key;
    int ignoreSystemTimerSignals;

    while (1)
    {
        sc_regGet(IGNORE_CLOCK_PULSES, &ignoreSystemTimerSignals);

        if (ignoreSystemTimerSignals == 1)
        {
            draw_interface();
            rk_readkey(&pressed_key);
            serve_pressed_key(pressed_key);
        }
    }

    return 0;
}

int draw_interface()
{
    mt_clrscr();
    draw_memory_box();
    draw_accum_box();
    draw_instrcntr();
    draw_operation();
    draw_flags();
    draw_big_num();
    draw_keys();
    draw_cursor();
    mt_gotoXY(1, 23);
    printf("\n");

    return 0;
}

int draw_memory_box()
{
    bc_box(1, 1, 61, 12);
    mt_gotoXY(27, 1);
    printf("Memory");
    mt_gotoXY(2, 2);

    int mem_value;

    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            sc_memoryGet(10 * i + j, &mem_value);

            if (j != 9)
                printf("+%.4X ", mem_value);
            else
                printf("+%.4X", mem_value);
        }

        mt_gotoXY(2, 3 + i);
    }

    return 0;
}

int draw_accum_box()
{
    int accum;

    get_accum(&accum);

    bc_box(63, 1, 82, 3);
    mt_gotoXY(67, 1);
    printf("accumulator");
    mt_gotoXY(69, 2);
    char *format = accum >= 0 ? "+%.4X" : "-%.4X";
    printf(format, accum);

    return 0;
}

int draw_instrcntr()
{
    int instrcntr;

    get_instructionCounter(&instrcntr);

    bc_box(63, 4, 82, 6);
    mt_gotoXY(64, 4);
    printf("instructionCounter");
    mt_gotoXY(69, 5);
    printf("+%.4X", instrcntr);

    return 0;
}

int draw_operation()
{
    int memvalue, command, operand;

    sc_memoryGet(cursor.y * 10 + cursor.x, &memvalue);
    sc_commandDecode(memvalue, &command, &operand);

    bc_box(63, 7, 82, 9);
    mt_gotoXY(69, 7);
    printf("Operation");
    mt_gotoXY(69, 8);
    printf("+%.2X : %.2X", command, operand);

    return 0;
}

int draw_flags()
{
    char strflag[9];

    get_strflag(strflag);

    bc_box(63, 10, 82, 12);
    mt_gotoXY(69, 10);
    printf("Flags");
    mt_gotoXY(68, 11);
    printf("%s", strflag);

    return 0;
}

int draw_big_num()
{
    int memvalue;

    bc_box(1, 13, 46, 22);
    sc_memoryGet(cursor.y * 10 + cursor.x, &memvalue);
    bc_printbigchar(bcPlus, 2, 14, Cyan, Black);

    for (int i = 4; i > 0; i--)
    {
        bc_printbigchar(bcdictionary[memvalue % 16],
                        2 + 9 * i, 14, Cyan, Black);
        memvalue /= 16;
    }

    return 0;
}

int draw_keys()
{
    bc_box(47, 13, 82, 22);
    mt_gotoXY(48, 13);
    printf("Keys:");
    mt_gotoXY(48, 14);
    printf("l   - load");
    mt_gotoXY(48, 15);
    printf("s   - save");
    mt_gotoXY(48, 16);
    printf("r   - run");
    mt_gotoXY(48, 17);
    printf("t   - step");
    mt_gotoXY(48, 18);
    printf("i   - reset");
    mt_gotoXY(48, 19);
    printf("F5  - accumulator");
    mt_gotoXY(48, 20);
    printf("F6  - instructionCounter");

    return 0;
}

int get_input()
{

    return 0;
}

int draw_cursor()
{
    int mem_value;

    sc_memoryGet(cursor.y * 10 + cursor.x, &mem_value);
    mt_setbgcolor(Blue);
    mt_gotoXY(2 + cursor.x * 6, cursor.y + 2);
    printf("+%.4X", mem_value);
    mt_setbgcolor(Default);

    return 0;
}

int scan_mem_value(int *input)
{
    scan_hex_num(input);

    return !isMEMvaluecorrect(*input);
}

int scan_accum_value()
{
    int input;

    scan_hex_num(&input);

    if (input < MIN_ACCUM_VALUE || input > MAX_ACCUM_VALUE)
        return 1;

    set_accum(input);

    return 0;
}

int scan_instrcntr_value()
{
    int input;

    scan_hex_num(&input);

    if (input < 0 || input > 99)
        return 1;

    set_instructionCounter(input);

    return 0;
}

int scan_hex_num(int *input)
{
    mt_gotoXY(1, 23);
    printf("\nInput: ");
    if (scanf("%x", input) == 0)
    {
        //Если при поптыке сканирования были введены символы не подходящие
        //для 16-ричной цифры то нужно очистить буффер stdin
        while (getchar() != '\n')
        {
        };
        *input = 0;
    }

    return 0;
}