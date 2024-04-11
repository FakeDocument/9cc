#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef enum
{
  TK_RESERVED, //記号
  TK_NUM,     //数値
  TK_EOF
} TokenKind;  //トークンの型

typedef struct Token Token;
struct Token
{
  TokenKind kind;
  Token *next;
  int num;    //トークンが数値の場合、その数値
  char *str;  //トークン文字列
};

//現在着目してるトークン
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

/*
次のトークンが期待している記号の時はトークンを進めてTrue
それ以外ならFalse
*/
bool consume(char op)
{
  if(token->kind!=TK_RESERVED || token->str[0]!=op)
    return false;
  token=token->next;
  return true;
}

/*
次のトークンが期待している記号の時はトークンを進める
それ以外ならエラーを出す
*/
void expect(char op)
{
  if(token->kind!=TK_RESERVED || token->str[0]!=op)
    error("%cではありません",op);
  token=token->next;
}

/*
次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
それ以外の場合にはエラーを報告する。
*/
int expectNumber()
{
  if(token->kind!=TK_NUM)
    error("数値ではありません");
  int val=token->num;
  token=token->next;
  return val;
}

bool atEOF()
{
  return token->kind==TK_EOF;
}

Token* new_token(TokenKind kind,Token *cur,char *str)
{
  Token *tkn=calloc(1,sizeof(Token));
  tkn->kind=kind;
  tkn->str=str;
  cur->next=tkn;
  return tkn;
}

//入力文字列をトークン化する
Token* tokenizer(char *s){
  Token head;
  head.next=NULL;
  Token *cur=&head;
  while(*s)
  {
    if(isspace(*s))
    {
      s++;
      continue;
    }
    if(*s=='+'||*s=='-')
    {
      cur=new_token(TK_RESERVED,cur,s++);
      continue;
    }
    if(isdigit(*s))
    {
      cur=new_token(TK_NUM,cur,s);
      cur->num=strtol(s,&s,10);
      continue;
    }
    error("トークナイズできません");
  }
  new_token(TK_EOF,cur,s);
  return head.next;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  char *p = argv[1];
  //printf("%s",p);
  token=tokenizer(p);

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");
  //式の最初は数字なので数字であることをチェックし出力
  printf("  mov rax, %d\n", expectNumber());


  while (!atEOF()) 
  {
    if(consume('+'))
    {
      printf("  add rax, %d\n",expectNumber());
      continue;
    }
    expect('-');
    printf("  sub rax, %d\n",expectNumber());
  }

  printf("  ret\n");
  return 0;
}
