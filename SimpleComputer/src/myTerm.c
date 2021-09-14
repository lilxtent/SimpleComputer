#include "headers/myTerm.h"

#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

int mt_clrscr(void)
{
  printf("\E[H\E[2J");
  return 0;
}

int mt_gotoXY(int x, int y)
{

  int rows, columns;

  mt_getscreensize(&rows, &columns);

  if (x > columns || x < 1 || y > rows || y < 1)
  {
    return -1;
  }

  printf("\E[%d;%dH", y, x);

  return 0;
}

int mt_getscreensize(int *rows, int *cols)
{
  struct winsize w;

  if (!ioctl(STDOUT_FILENO, TIOCGWINSZ, &w))
  {
    *rows = w.ws_row;
    *cols = w.ws_col;
    return 0;
  }
  else
  {
    return -1;
  }
}

int mt_setfgcolor(enum color color)
{
  //коды цветов символов находятся в диапазоне 30-37
  if ((color != 39) && (color < 30 || color > 37))
  {
    return -1;
  }

  printf("\E[%dm", color);
  return 0;
}

int mt_setbgcolor(enum color color)
{
  //коды цветов символов находятся в диапазоне 30-37
  if ((color != 39) && (color < 30 || color > 37))
  {
    return -1;
  }

  printf("\E[%dm", color + 10); //+10 т.к. у фона код цвета на 10 больше цвета тескста

  return 0;
}