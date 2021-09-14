#include "headers/CPU.h"

int CU(void)
{
    int instrcntr;
    get_instructionCounter(&instrcntr);

    int memvalue;
    sc_memoryGet(instrcntr, &memvalue);

    int command, operand;
    if (sc_commandDecode(memvalue, &command, &operand) != 0)
    {
        sc_regSet(WRONG_COMMAND, 1);
        sc_regSet(IGNORE_CLOCK_PULSES, 1);
        return -1;
    }

    if (!isOperandAddressCorrect(operand))
    {
        sc_regSet(MEMORY_CORRUPT, 1);
        sc_regSet(IGNORE_CLOCK_PULSES, 1);
        return -1;
    }

    if (command != 0 && !isCommandExist(command))
    {
        sc_regSet(WRONG_COMMAND, 1);
        sc_regSet(IGNORE_CLOCK_PULSES, 1);
        return -1;
    }

    if (isALUcommand(command))
        ALU(command, operand);
    else if (command != 0)
        serveNotALUcommand(command, operand);

    if (instrcntr + 1 <= 99)
        increment_instrcntr();
    else
        sc_regSet(MEMORY_CORRUPT, 1);

    return 0;
}

int isOperandAddressCorrect(int operand)
{
    return operand >= 0 && operand <= 100;
}

int isALUcommand(int command)
{
    return command >= 0x30 && command <= 0x33;
}

int isMEMvaluecorrect(int memvalue)
{
    return memvalue >= MIN_MEM_VALUE && memvalue <= MAX_MEM_VALUE;
}

int serveNotALUcommand(int command, int operand)
{
    if (command == READ)
        com_READ(operand);
    else if (command == WRITE)
        com_WRITE(operand);
    else if (command == LOAD)
        com_LOAD(operand);
    else if (command == STORE)
        com_STORE(operand);
    else if (command == JUMP)
        com_JUMP(operand);
    else if (command == JNEG)
        com_JNEG(operand);
    else if (command == JZ)
        com_JZ(operand);
    else if (command == HALT)
        com_HALT();
    else if (command == ADDC)
        com_ADDC(operand);
    else
    {
        sc_regSet(WRONG_COMMAND, 1);
        return 1;
    }

    return 0;
}

int ALU(int command, int operand)
{
    if (command == ADD)
        com_ADD(operand);
    else if (command == SUB)
        com_SUB(operand);
    else if (command == DIVIDE)
        com_DIVIDE(operand);
    else if (command == MUL)
        com_MUL(operand);
    else
    {
        sc_regSet(WRONG_COMMAND, 1);
        return 1;
    }

    return 0;
}

//Ввод с терминала в указанную ячейку памяти с контролем переполнения
int com_READ(int operand)
{
    int hexnum;
    if (scan_mem_value(&hexnum) != 0)
    {
        sc_regSet(OVERFLOW, 1);
        return 1;
    }

    sc_memorySet(operand, hexnum);

    return 0;
}

//Вывод на терминал значение указанной ячейки памяти
int com_WRITE(int operand)
{
    int memvalue, command, _operand;
    sc_memoryGet(operand, &memvalue);
    sc_commandDecode(memvalue, &command, &_operand);
    mt_gotoXY(1, 23);
    printf("\nMemory Cell №%d: %.2X : %.2X\n", operand, command, _operand);
    getchar();
    getchar();

    return 0;
}

//Загрузка в аккумулятор значения из указанного адреса памяти
int com_LOAD(int operand)
{
    int value;
    sc_memoryGet(operand, &value);

    set_accum(value);

    return 0;
}

//Выгружает значение из аккумулятора по указанному адресу памяти
int com_STORE(int operand)
{
    int value;
    get_accum(&value);

    sc_memorySet(operand, value);

    return 0;
}

//Выполняет сложение слова в аккумуляторе и слова из указанной ячейки памяти
//(результат в аккумуляторе)
int com_ADD(int operand)
{
    int accumulator;
    get_accum(&accumulator);

    int memvalue;
    sc_memoryGet(operand, &memvalue);

    int summ = accumulator + memvalue;

    if (summ > MAX_ACCUM_VALUE)
    {
        sc_regSet(OVERFLOW, 1);
        return 1;
    }

    set_accum(summ);

    return 0;
}

//Вычитает из слова в аккумуляторе слово из указанной ячейки памяти
//(результат в аккумуляторе)
int com_SUB(int operand)
{
    int accumulator;
    get_accum(&accumulator);

    int memvalue;
    sc_memoryGet(operand, &memvalue);

    int sub = accumulator - memvalue;

    if (sub < MIN_ACCUM_VALUE)
    {
        sc_regSet(OVERFLOW, 1);
        return 1;
    }

    set_accum(sub);

    return 0;
}

//Выполняет деление слова в аккумуляторе на слово из указанной ячейки памяти
//(результат в аккумуляторе)
int com_DIVIDE(int operand)
{
    int accumulator;
    get_accum(&accumulator);

    int memvalue;
    sc_memoryGet(operand, &memvalue);

    if (memvalue == 0)
    {
        sc_regSet(DIVISION_BY_ZERO, 1);
        return 1;
    }

    set_accum(accumulator / memvalue);

    return 0;
}

//Вычисляет произведение слова в аккумуляторе на слово из указанной ячейки памяти
//(результат в аккумуляторе)
int com_MUL(int operand)
{
    int accumulator;
    get_accum(&accumulator);

    int memvalue;
    sc_memoryGet(operand, &memvalue);

    int mult = accumulator * memvalue;

    if (mult < MIN_ACCUM_VALUE || mult > MAX_ACCUM_VALUE)
    {
        sc_regSet(OVERFLOW, 1);
        return 1;
    }

    set_accum(mult);

    return 0;
}

//Переход к указанному адресу памяти
int com_JUMP(int operand)
{
    set_instructionCounter(--operand);

    return 0;
}

//Переход к указанному адресу памяти, если в аккумуляторе находится
//отрицательное число
int com_JNEG(int operand)
{
    int value;
    get_accum(&value);

    if (value < 0)
        set_instructionCounter(--operand);

    return 0;
}

//Переход к указанному адресу памяти, если в аккумуляторе находится ноль
int com_JZ(int operand)
{
    int value;
    get_accum(&value);

    if (value == 0)
        set_instructionCounter(--operand);

    return 0;
}

//Останов, выполняется при завершении работы программы
int com_HALT()
{
    sc_regSet(IGNORE_CLOCK_PULSES, 1);
    stopTimer();

    return 0;
}

//Сложение содержимого указанной ячейки памяти с ячейкой памяти,
//адрес которой находится в ячейке памяти, указанной в аккумуляторе
//(результат в аккумуляторе)
int com_ADDC(int operand)
{
    int memvalue;
    sc_memoryGet(operand, &memvalue);

    int command1, operand1;
    sc_commandDecode(memvalue, &command1, &operand1);

    int accum;
    get_accum(&accum);

    int command2, operand2;
    sc_commandDecode(accum, &command2, &operand2);

    int command, _operand;

    command = command1 + command2;
    _operand = operand1 + operand2;

    int result;
    sc_commandEncode(command, _operand, &result);

    set_accum(result);

    return 0;
}