#include <stdio.h>
#include <fcntl.h>
#include <signal.h>

#include "headers/SimpleComputer.h"
#include "headers/myTerm.h"
#include "headers/myBigChars.h"
#include "headers/myReadkey.h"
#include "headers/interface.h"
#include "headers/Controller.h"


int main()
{

  signal(SIGUSR1, resetSIGhandler);
  signal(SIGALRM, timerhandler);
  sc_regSet(IGNORE_CLOCK_PULSES, 1);

  activate_interface();

  return 0;
}