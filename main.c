#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "9cc.h"

char *userInput;
Token *token;
LoVar *loVarList;

// エラー報告用関数。printfと同じ引数
void error(char *fmt, ...)
{
  va_list ap; // 可変長引数を一つの変数にまとめるようやね
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void errorAt(char *loc, char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - userInput;
  fprintf(stderr, "%s\n", userInput);
  fprintf(stderr, "%*s", pos, " ");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

int main(int argc, char **argv)
{
  loVarList = (LoVar *)calloc(1, sizeof(LoVar));
  loVarList->next = NULL;
  loVarList->str = "";
  loVarList->len = 0;
  loVarList->offset = 0;
  loVarList->next = NULL;

  if (argc != 2)
  {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }
  userInput = argv[1];
  // printf("%s",p);
  token = tokenizer(userInput);
  program();
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // プロローグ
  // 変数個分の領域を確保する
  DEBUG_PRINT("#プロローグ\n");
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %d\n", loVarList->offset);
  DEBUG_PRINT("#プロローグここまで\n");

  for (int i = 0; code[i]; i++)
  {
    gen(code[i]);

    // 式の評価結果としてスタックに一つの値が残っている
    // はずなので、スタックが溢れないようにポップしておく
    printf("  pop rax\n");
  }

  // 最後の式の結果がraxに残ってるらしい
  DEBUG_PRINT("#エピローグ\n");
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  DEBUG_PRINT("#エピローグここまで\n");
  return 0;
}
