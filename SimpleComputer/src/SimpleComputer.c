#include "headers/SimpleComputer.h"

int memory[100];
int memorySize = sizeof(memory) / sizeof(memory[0]);
int instructionCounter = 0;
int accum = 0;
int flag;
int commands[] = {0x10, 0x11, 0x20, 0x21, 0x30, 0x31, 0x32, 0x33, 0x40, 0x41,
                  0x42, 0x43, 0x75};

int sc_memoryInint()
{
  for (int i = 0; i < memorySize; i++)
    memory[i] = 0;

  return 0;
}

int sc_memorySet(int address, int value)
{
  if (address > memorySize || address < 0)
  {
    sc_regSet(MEMORY_CORRUPT, 1);
    return MEMORY_CORRUPT;
  }

  memory[address] = value;
  return 0;
}

int sc_memoryGet(int address, int *value)
{
  if (address > memorySize || address < 0)
  {
    sc_regSet(MEMORY_CORRUPT, 1);
    return MEMORY_CORRUPT;
  }

  *value = memory[address];
  return 0;
}

int sc_memorySave(char *filename)
{
  FILE *file = fopen(filename, "wb");
  fwrite(&memory, sizeof(int), memorySize, file);
  fclose(file);

  return 0;
}

int sc_memoryLoad(char *filename)
{
  FILE *file = fopen(filename, "rb");

  if (file == NULL)
    return 1;

  fread(&memory, sizeof(int), memorySize, file);

  fclose(file);

  return 0;
}

int sc_regInit(void)
{
  flag = 0;
  return 0;
}

int sc_regSet(int registerr, int value)
{
  if (registerr < 1 || registerr > 5)
  {
    return WRONG_COMMAND;
  }
  else if (value == 0)
  {
    flag = flag & (~(1 << (registerr - 1)));
  }
  else if (value == 1)
  {
    flag = flag | (1 << (registerr - 1));
  }
  else
  {
    return WRONG_COMMAND;
  }

  return 0;
}

int sc_regGet(int registerr, int *value)
{
  if (registerr < 1 || registerr > 5)
  {
    return WRONG_COMMAND;
  }
  *value = (flag >> (registerr - 1)) & 0x1;
  return 0;
}

int sc_commandEncode(int command, int operand, int *value)
{
  if (!isCommandExist(command))
    return WRONG_COMMAND;

  *value = (command << 7) | operand;

  return 0;
}

int isCommandExist(int command)
{
  //13 - количество возможных команд
  int *searchResult = (int *)bsearch(&command, &commands, 13,
                                     sizeof(int), compare);

  return !(searchResult == NULL);
}

int sc_commandDecode(int value, int *command, int *operand)
{
  if (value > MAX_MEM_VALUE)
  {
    sc_regSet(WRONG_COMMAND, 1);
    return WRONG_COMMAND;
  }

  *operand = value & 0x7F;
  *command = (value >> 7) & 0x7F;

  return 0;
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

int get_instructionCounter(int *value)
{
  *value = instructionCounter;

  return 0;
}

int set_instructionCounter(int value)
{
  instructionCounter = value;

  return 0;
}

int get_accum(int *value)
{
  *value = accum;

  return 0;
}

int set_accum(int value)
{
  accum = value;

  return 0;
}

int resetSimpleComputer()
{
  sc_memoryInint();
  set_instructionCounter(0);
  set_accum(0);
  flag = 0;
  sc_regSet(IGNORE_CLOCK_PULSES, 1);

  return 0;
}

void resetSIGhandler(__attribute__((unused)) int signal)
{
  resetSimpleComputer();
}

int startTimer()
{
  struct itimerval nval, oval;

  nval.it_interval.tv_sec = 0;
  nval.it_interval.tv_usec = 100000;
  nval.it_value.tv_sec = 0;
  nval.it_value.tv_usec = 100000;

  setitimer(ITIMER_REAL, &nval, &oval);
  sc_regSet(IGNORE_CLOCK_PULSES, 0);

  return 0;
}

int stopTimer()
{
  struct itimerval nval, oval;

  nval.it_interval.tv_usec = 0;
  nval.it_value.tv_usec = 0;

  setitimer(ITIMER_REAL, &nval, &oval);
  sc_regSet(IGNORE_CLOCK_PULSES, 1);

  return 0;
}

int increment_instrcntr()
{
  if (instructionCounter != 99)
    instructionCounter++;

  return 0;
}