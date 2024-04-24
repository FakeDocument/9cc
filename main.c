#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "9cc.h"

char* userInput;
Token *token;

//エラー報告用関数。printfと同じ引数
void error(char *fmt,...)
{
  va_list ap;       //可変長引数を一つの変数にまとめるようやね
  va_start(ap,fmt);
  vfprintf(stderr,fmt,ap);
  fprintf(stderr,"\n");
  exit(1);
}

void errorAt(char *loc,char *fmt,...)
{
  va_list ap;
  va_start(ap,fmt);

  int pos = loc-userInput;
  fprintf(stderr,"%s\n",userInput);
  fprintf(stderr,"%*s",pos," ");
  fprintf(stderr,"^ ");
  vfprintf(stderr,fmt,ap);
  fprintf(stderr,"\n");
  exit(1);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }
  userInput=argv[1];
  char *p = argv[1];
  //printf("%s",p);
  token=tokenizer(p);
  //fprintf(stderr,"%s\n",token->str);
  Node* node=expr();
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");
  
  gen(node);

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
