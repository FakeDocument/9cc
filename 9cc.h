#ifndef cc9
#define cc9
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) \
    do { \
        printf((fmt), ##__VA_ARGS__); \
    } while (0)
#else
#define DEBUG_PRINT(fmt, ...) \
    do { } while (0)
#endif

typedef enum
{
  TK_RESERVED, // 記号
  TK_NUM,      // 数値
  TK_IDENT,    // 識別子
  TK_EOF,
  TK_RETURN
} TokenKind; // トークンの型

struct TokenStruct
{
  TokenKind kind;
  struct TokenStruct *next;
  int num;   // トークンが数値の場合、その数値
  char *str; // トークン文字列
  unsigned int len;
};
typedef struct TokenStruct Token;

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

/*ローカル変数*/
struct LoVarStruct
{
    struct LoVarStruct *next;
    char *str;
    unsigned int len;
    int offset;
};
typedef struct LoVarStruct LoVar;
typedef struct Node Node;

/*ローカル変数の先頭ポインタ*/
extern LoVar *loVarList;

// 100文まで格納する
extern Node *code[100];

// ユーザーの入力した文字列
// extern char* userInput;

// エラー報告用関数。printfと同じ引数
void error(char *fmt, ...);

void errorAt(char *loc, char *fmt, ...);

extern Token *token;

/*
次のトークンが期待している記号の時はトークンを進めてTrue
それ以外ならFalse
*/
bool consume(char *op);

/*
次のトークンが変数の時はトークンを進めずTrue
それ以外ならFalse
*/
bool peekIdent();

void expect(char *op);

int expectNumber();

Token *expectIdent();

bool atEOF();

bool isAlNumBar(char c);

Token *newToken(TokenKind kind, Token *cur, char *str, int len);

Token *tokenizer(char *s);

Node *newNode(NodeKind kind, Node *left, Node *right);

Node *newNodeNum(int val);
Node *newNodeIdent(Token *tkn);

// 優先度低
void program();     // stmt*　stmtの連なり
Node *stmt();       // expr";" exprを;で区切ったもの
Node *expr();       // assign
Node *assign();     // equality ("="assign)? 右結合で=assignが続くかもね、みたいな
Node *equality();   // relational ("==" relational | "!=" relational)*
Node *relational(); // add ("<" add | "<=" add | ">" add | ">=" add)*
Node *add();        // mul ("+" mul | "-" mul)*
Node *mul();        // unary ("*" unary | "/" unary)*
Node *unary();      //("+" |"-")? primary
Node *primary();    // num|ident|"("expr")"
                 // 優先度高

void gen(Node *node);
/**
 * gen_lvalは、与えられたノードが変数を指しているときに、その変数のアドレスを計算して、それをスタックにプッシュする
 */
void genLval(Node *node);

extern char *userInput;
extern Token *token;
int main(int argc, char **argv);
#endif