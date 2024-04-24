#ifndef cc9
#define cc9
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


//ユーザーの入力した文字列
extern char* userInput;

//エラー報告用関数。printfと同じ引数
void error(char *fmt,...);

void errorAt(char *loc,char *fmt,...);

extern Token *token;

bool consume(char* op);

void expect(char* op);

int expectNumber();

bool atEOF();

Token* newToken(TokenKind kind, Token *cur, char *str, int len);

Token* tokenizer(char *s);



Node* newNode(NodeKind kind, Node *left, Node *right);

Node* newNodeNum(int val);

                    //優先度低
Node* expr();       //expr=equality
Node* equality();   //relational ("==" relational | "!=" relational)*
Node* relational(); //add ("<" add | "<=" add | ">" add | ">=" add)*
Node* add();        //mul ("+" mul | "-" mul)*
Node* mul();        //unary ("*" unary | "/" unary)*
Node* unary();      //("+" |"-")? primary
Node* primary();    //num|"("expr")"
                    //優先度高

void gen(Node* node);
#endif