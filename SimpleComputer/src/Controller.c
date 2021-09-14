#include "headers/Controller.h"

int serve_pressed_key(keys pressed_key)
{

    if (pressed_key == KEY_l)
        loadMemory();
    else if (pressed_key == KEY_s)
        sc_memorySave("sc_memory.dat");
    else if (pressed_key == KEY_r)
        startTimer();
    else if (pressed_key == KEY_t)
        timerhandler(0);
    else if (pressed_key == KEY_i)
        resetSC();
    else if (pressed_key == KEY_f5)
        scan_accum_value();
    else if (pressed_key == KEY_f6)
        scan_instrcntr_value();
    else if (pressed_key == KEY_up)
        cursor_UP();
    else if (pressed_key == KEY_down)
        cursor_DOWN();
    else if (pressed_key == KEY_left)
        cursor_LEFT();
    else if (pressed_key == KEY_right)
        cursor_RIGHT();
    else if (pressed_key == KEY_enter)
        set_choosen_mem_cell();

    return 0;
}

int cursor_UP()
{
    if (cursor.y != 0)
        cursor.y--;
    return 0;
}

int cursor_DOWN()
{
    if (cursor.y != 9)
        cursor.y++;
    return 0;
}

int cursor_LEFT()
{
    if (cursor.x != 0)
        cursor.x--;
    return 0;
}

int cursor_RIGHT()
{
    if (cursor.x != 9)
        cursor.x++;
    return 0;
}

int set_choosen_mem_cell()
{
    int hexnum;
    if (scan_mem_value(&hexnum) != 0)
    {
        sc_regSet(OVERFLOW, 1);
        return 1;
    }

    sc_memorySet(cursor.y * 10 + cursor.x, hexnum);

    return 0;
}

int get_strflag(char *strflag)
{
    int flag;

    sc_regGet(OVERFLOW, &flag);
    strflag[0] = flag == 0 ? ' ' : 'O';
    sc_regGet(DIVISION_BY_ZERO, &flag);
    strflag[2] = flag == 0 ? ' ' : 'D';
    sc_regGet(MEMORY_CORRUPT, &flag);
    strflag[4] = flag == 0 ? ' ' : 'M';
    sc_regGet(IGNORE_CLOCK_PULSES, &flag);
    strflag[6] = flag == 0 ? ' ' : 'I';
    sc_regGet(WRONG_COMMAND, &flag);
    strflag[8] = flag == 0 ? ' ' : 'W';

    for (int i = 1; i < 9; i += 2)
        strflag[i] = ' ';

    return 0;
}

int resetSC()
{
    cursor.y = cursor.x = 0;
    raise(SIGUSR1);

    return 0;
}

int makeStep()
{

    return 0;
}

void timerhandler(__attribute__((unused)) int signal)
{
    int ignore_flag;
    sc_regGet(IGNORE_CLOCK_PULSES, &ignore_flag);

    if (ignore_flag == 1)
        stopTimer();

    int instrcntr;

    get_instructionCounter(&instrcntr);

    cursor.x = instrcntr % 10;
    cursor.y = instrcntr / 10;

    draw_interface();

    CU();
}

int loadMemory()
{
    mt_gotoXY(1, 23);
    printf("\nВведите имя файла с памятью: ");
    char input[32];
    scanf("%s", input);

    if (access(input, F_OK) != -1)
    {
        sc_memoryLoad(input);
    }
    else
    {
        printf("Такой файл не существует\nНажмите Enter");
        sleep(1);
    }

    return 0;
}