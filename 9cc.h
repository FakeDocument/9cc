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
  TK_RESERVED,  //記号
  TK_NUM,       //数値
  TK_IDENT,     //識別子
  TK_EOF
} TokenKind;  //トークンの型

typedef struct Token Token;

typedef enum
{
  ND_NUM,
  ND_LVAR,      // ローカル変数
  ND_ADD,       // +
  ND_SUB,       // -
  ND_MUL,       // *
  ND_DIV,       // /
  ND_ASSIGN,    // =
  ND_EQL,       // ==
  ND_NEQL,      // !=
  ND_LESS,      // <
  ND_LESS_THAN, // <=
} NodeKind;


typedef struct Node Node;

//100文まで格納する
extern Node *code[100];

//ユーザーの入力した文字列
//extern char* userInput;

//エラー報告用関数。printfと同じ引数
void error(char *fmt,...);

void errorAt(char *loc,char *fmt,...);

extern Token *token;

bool consume(char* op);

bool consumeIdent();

void expect(char* op);

int expectNumber();

char* expectIdent();

bool atEOF();

Token* newToken(TokenKind kind, Token *cur, char *str, int len);

Token* tokenizer(char *s);



Node* newNode(NodeKind kind, Node *left, Node *right);

Node* newNodeNum(int val);
Node* newNodeIdent(char* str);

                    //優先度低
Node* program();    //stmt*　stmtの連なり
Node* stmt();       //expr";" exprを;で区切ったもの   
Node* expr();       //assign 
Node* assign();     //equality ("="assign)? 右結合で=assignが続くかもね、みたいな
Node* equality();   //relational ("==" relational | "!=" relational)*
Node* relational(); //add ("<" add | "<=" add | ">" add | ">=" add)*
Node* add();        //mul ("+" mul | "-" mul)*
Node* mul();        //unary ("*" unary | "/" unary)*
Node* unary();      //("+" |"-")? primary
Node* primary();    //num|ident|"("expr")"
                    //優先度高

void gen(Node* node);
void genLval(Node* node);

int main(int argc, char **argv);
#endif