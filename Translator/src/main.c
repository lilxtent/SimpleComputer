#include "headers/sat.h"
#include "headers/sbt.h"

int main(int argc, char **argv)
{
  if (argc != 3)
  {
    printf("Передано неверное количество аргументов\n");
    return 1;
  }

  if (IsStrEndsWith(argv[1], ".sa"))
    translateSAtoBIN(argv[1], argv[2]);
  else if (IsStrEndsWith(argv[1], ".sb"))
    translateSBtoSA(argv[1], argv[2]);
  else
  {
    printf("WRONG COMMAND\n");
    return 1;
  }

  //int temp;

  //printf("Scanned: %d", scanf("%x", &temp));

  //printf("1 arg: %s\n2 arg: %s\n", argv[1], argv[2]);

  //result = translateSAtoBIN(argv[1], argv[2]);
  /*
  printf("%s\n", cmndGOTO("GOTO 10"));
  printf("%s\n", cmndGOTO("GOTO 20"));
  printf("%s\n", cmndGOTO("GOTO 30"));
  printf("%s\n", cmndGOTO("GOTO 40"));
  printf("%s\n", cmndINPUT("OUTPUT A"));
  printf("%s\n", cmndINPUT("OUTPUT D"));
  printf("%s\n", cmndINPUT("OUTPUT C"));
  printf("%s\n", cmndINPUT("OUTPUT D"));
  printf("Result: %d\n", result);

  printf("Ends with: %d\n", IsStrEndsWith("base 20", "base %d"));

  FILE *file = fopen("bas.sb", "r");
  makeGOTOqueue(file);
  printf("Tets nums: %.2d\n", 5);
  printf("Divide : %d\n", 2 / 2);

  char test[50] = "ab";
  strcat(test, "vog");
  printf("%s", test);
  printf("hm\n");

  char test2[64];
  sprintf(test2, "%d %s", 5, "hm");
  sprintf(test2, "%d %s", 9, "jj");
  printf("%s\n", test2);

  printf("IF:\n%s\n", cmndIF("IF A > D INPUT C"));

  int mas[2] = {1, 2};
  int index = 0;
  printf("%d\n", mas[index++]);
  printf("%d\n", mas[index]);
  char polish[512];
  translate_to_rpn(polish, "4 + (A - B) * (C / D) + 1");
  char *fff = cmndLET("E = 4 + (A - B) * (C / D) + 1");
  printf("Polish: %s\n", polish);
  printf("LET:\n%s\n", cmndLET("LET E = 4 + (A - B) * (C / D) + 1"));
*/
  return 0;
}
