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

//ユーザーの入力した文字列
char* userInput;

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
    errorAt(token->str,"%cではありません",op);
  token=token->next;
}

/*
次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
それ以外の場合にはエラーを報告する。
*/
int expectNumber()
{
  if(token->kind!=TK_NUM)
    errorAt(token->str,"数値ではありません");
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
    //printf(";%s\n",s);
    if(isspace(*s))
    {
      s++;
      continue;
    }
    if(*s=='+'||*s=='-')
    {
      cur=new_token(TK_RESERVED,cur,s++);
      //printf(";%s\n",s);
      continue;
    }
    if(isdigit(*s))
    {
      cur=new_token(TK_NUM,cur,s);
      cur->num=strtol(s,&s,10);
      continue;
    }
    errorAt(s,"トークナイズできません");
  }
  new_token(TK_EOF,cur,s);
  return head.next;
}


typedef enum
{
  ND_NUM,
  ND_ADD,
  ND_SUB
} NodeKind;

typedef struct Node Node;
struct Node
{
  NodeKind kind;
  Node *left,*right;
  int val;
};

Node* newNode(NodeKind kind,Node *left,Node *right)
{
  Node *node=calloc(1,sizeof(Node));
  node->kind=kind;
  node->left=left;
  node->right=right;
  return node;
}

Node* newNodeNum(int val)
{
  Node *node=calloc(1,sizeof(Node));
  node->kind=ND_NUM;
  node->val=val;
  return node;
}
/*
EBNF
expr=num('+'num | '-'num)*
*/
Node* expr()
{
  Node *node=newNodeNum(expectNumber());
  while(1)
  {
    if(consume('+'))
    {
      node=newNode(ND_ADD,node,newNodeNum(expectNumber()));
    }
    else if(consume('-'))
    {
      node=newNode(ND_SUB,node,newNodeNum(expectNumber()));
    }
    else
    {
      return node;
    }
  }
}

void gen(Node* node)
{
  if(node->kind==ND_NUM)
  {
    printf("  push %d\n",node->val);
    return;
  }
  gen(node->left);
  gen(node->right);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch(node->kind)
  {
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
  }
  printf("  push rax\n");
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
  Node* node=expr();
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");
  
  gen(node);

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
