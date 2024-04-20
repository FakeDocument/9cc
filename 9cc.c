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
  int len;
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
bool consume(char* op)
{
  if(
    token->kind!=TK_RESERVED ||
    strlen(op)!=token->len||
    memcmp(token->str,op,token->len))
    {
      return false;
    }
  token=token->next;
  return true;
}

/*
次のトークンが期待している記号の時はトークンを進める
それ以外ならエラーを出す
*/
void expect(char* op)
{
  if(token->kind!=TK_RESERVED ||
    strlen(op)!=token->len||
    memcmp(token->str,op,token->len))
    {
      errorAt(token->str,"%cではありません\n%sです\n",op,token->str);
    }
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

Token* newToken(TokenKind kind,Token *cur,char *str,int len)
{
  Token *tkn=calloc(1,sizeof(Token));
  tkn->kind=kind;
  tkn->str=str;
  tkn->len=len;
  cur->next=tkn;
  return tkn;
}
/*
main:
  push 2
  push 3
  pop rdi
  pop rax
  cmp rax, rdi
  sete al
  movzb rax, al
  push rax
  pop rax
  ret
*/
//入力文字列をトークン化する
Token* tokenizer(char *s){
  Token head;
  head.next=NULL;
  Token *cur=&head;
  while(*s)
  {
    //printf("; %s\n",s);

    if(isspace(*s))
    {
      s++;
      continue;
    }
    if(isdigit(*s))
    {
      cur=newToken(TK_NUM,cur,s,1);
      //strtolは数字じゃないところまでポインタを進める
      cur->num=strtol(s,&s,10);
      continue;
    }
    int len=2;
    if(
      strncmp(s,"==",len)==0||
      strncmp(s,"!=",len)==0||
      strncmp(s,"<=",len)==0||
      strncmp(s,">=",len)==0)
    {
      cur=newToken(TK_RESERVED,cur,s,len);
      s+=len;
      continue;
    }
    
    if(
      *s=='+'||
      *s=='-'||
      *s=='*'||
      *s=='/'||
      *s=='('||
      *s==')'||
      *s=='<'||
      *s=='>')
    {
      cur=newToken(TK_RESERVED,cur,s,1);
      s++;
      continue;
    }
    errorAt(s,"トークナイズできません");
  }
  newToken(TK_EOF,cur,s,1);
  return head.next;
}


typedef enum
{
  ND_NUM,
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_EQL,
  ND_NEQL,
  ND_LESS,
  ND_LESS_THAN,
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
1+2*(3+4)-5
EBNF
expr=mul('+'mul | '-'mul)*
*/
                    //優先度低
Node* expr();       //expr=equality
Node* equality();   //relational ("==" relational | "!=" relational)*
Node* relational(); //add ("<" add | "<=" add | ">" add | ">=" add)*
Node* add();        //mul ("+" mul | "-" mul)*
Node* mul();        //unary ("*" unary | "/" unary)*
Node* unary();      //("+" |"-")? primary
Node* primary();    //num|"("expr")"
                    //優先度高
Node* expr()
{
  return equality();
}

//relational ("==" relational | "!=" relational)*
Node* equality()
{
  Node* (*next)()=relational;
  Node *node=next();
  while(1)
  {
    if(consume("=="))
    {
      node=newNode(ND_EQL,node,next());
    }
    else if(consume("!="))
    {
      node=newNode(ND_NEQL,node,next());
    }
    else
    {
      return node;
    }
  }
}

//add ("<" add | "<=" add | ">" add | ">=" add)*
Node* relational()
{
  Node* (*next)()=add;
  Node *node=next();
  while(1)
  {
    if(consume(">="))
    {
      //アセンブリには以下の判定しかないため左右を入れ替える
      node=newNode(ND_LESS_THAN,next(),node);
    }
    else if(consume("<="))
    {
      node=newNode(ND_LESS_THAN,node,next());
    }
    else if(consume(">"))
    {
      node=newNode(ND_LESS,next(),node);
    }
    else if(consume("<"))
    {
      node=newNode(ND_LESS,node,next());
    }
    else
    {
      return node;
    }
  }
}

//mul ("+" mul | "-" mul)*
Node* add()
{
  Node* (*next)()=mul;
  Node *node=next();
  while(1)
  {
    if(consume("+"))
    {
      node=newNode(ND_ADD,node,next());
    }
    else if(consume("-"))
    {
      node=newNode(ND_SUB,node,next());
    }
    else
    {
      return node;
    }
  }
}

//mul=unary('*'unary | '/'unary)*
Node* mul()
{
  Node* (*next)()=unary;
  Node *node=next();
  if(consume("*"))
  {
    node=newNode(ND_MUL,node,next());
  }
  else if (consume("/"))
  {
    node=newNode(ND_DIV,node,next());
  }
  else
  {
    return node;
  }
}

//単項＋ー　unary=('+'|'-')?primary
Node* unary()
{
  if(consume("+"))
  {
    return primary();;
  }
  else if(consume("-"))
  {
    return newNode(ND_SUB,newNodeNum(0),primary());
  }
  return primary();;
}

//primary=num|'('expr')'
Node* primary()
{
  if(consume("(")){
    Node* node=expr();
    expect(")"); //)は閉じないとおかしい
    return node;
  }
  return newNodeNum(expectNumber());
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
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("  cqo\n");
      printf("  idiv rdi\n");
      break;
    case ND_EQL:
      printf( 
        "  cmp rax, rdi\n"
        "  sete al\n"
        "  movzb rax, al\n");
      break;
    case ND_NEQL:
      printf( 
        "  cmp rax, rdi\n"
        "  setne al\n"
        "  movzb rax, al\n");
      break;
    case ND_LESS:
      printf(
        "  cmp rax, rdi\n"
        "  setl al\n"
        "  movzb rax, al\n");
      break;
    case ND_LESS_THAN:
      printf(
        "  cmp rax, rdi\n"
        "  setle al\n"
        "  movzb rax, al\n");
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
