#include "9cc.h"

int currentLabelID = 0;

/*
次のトークンが期待している記号の時はトークンを進めてTrue
それ以外ならFalse
*/
bool consume(char *op)
{
  if (
      token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
  {
    return false;
  }
  token = token->next;
  return true;
}

bool consumeByTokenKind(TokenKind tk)
{
  if (token->kind != tk)
  {
    return false;
  }
  token = token->next;
  return true;
}

/*
次のトークンが期待している文字の時はTrue
それ以外ならFalse
*/
bool consume(char *op)
{
  if (
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
  {
    return false;
  }
  return true;
}

/*
次のトークンが変数の時はトークンを進めずTrue
それ以外ならFalse
*/
bool peekIdent()
{
  if (token->kind != TK_IDENT)
  {
    return false;
  }
  return true;
}

/*
次のトークンが期待している記号の時はトークンを進める
それ以外ならエラーを出す
*/
void expect(char *op)
{
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
  {
    errorAt(token->str, "%sではありません\n%sです\n", op, token->str);
  }
  token = token->next;
}

/*
次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
それ以外の場合にはエラーを報告する。
*/
int expectNumber()
{
  if (token->kind != TK_NUM)
    errorAt(token->str, "数値ではありません");
  int val = token->num;
  token = token->next;
  return val;
}

/*
次のトークンが変数の場合、トークンを1つ読み進めてその変数を返す。
それ以外の場合にはエラーを報告する。
*/
Token *expectIdent()
{
  if (token->kind != TK_IDENT)
    errorAt(token->str, "変数ではありません");
  Token *tkn = token;
  token = token->next;
  return tkn;
}

bool atEOF()
{
  return token->kind == TK_EOF;
}

bool isAlNumBar(char c)
{
  return ('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') ||
         (c == '_');
}

Token *newToken(TokenKind kind, Token *cur, char *str, int len)
{
  Token *tkn = (Token *)calloc(1, sizeof(Token));
  tkn->kind = kind;
  tkn->str = str;
  tkn->len = len;
  cur->next = tkn;
  return tkn;
}

// 入力文字列をトークン化する
Token *tokenizer(char *s)
{
  Token head;
  head.next = NULL;
  Token *cur = &head;
  while (*s)
  {

    if (isspace(*s))
    {
      s++;
      continue;
    }
    if (isdigit(*s))
    {
      cur = newToken(TK_NUM, cur, s, 1);
      // strtolは数字じゃないところまでポインタを進める
      cur->num = strtol(s, &s, 10);
      continue;
    }
    {
      const int len = 6;
      if (strncmp(s, "return", len) == 0 && !isAlNumBar(s[len]))
      {
        cur = newToken(TK_RETURN, cur, s, len);
        s += len;
        continue;
      }
    }
    {
      const int len = 2;
      if (strncmp(s, "if", len) == 0 && !isAlNumBar(s[len]))
      {
        cur = newToken(TK_IF, cur, s, len);
        s += len;
        continue;
      }
    }
    {
      const int len = 4;
      if (strncmp(s, "else", len) == 0 && !isAlNumBar(s[len]))
      {
        cur = newToken(TK_ELSE, cur, s, len);
        s += len;
        continue;
      }
    }
    {
      const int len = 5;
      if (strncmp(s, "while", len) == 0 && !isAlNumBar(s[len]))
      {
        cur = newToken(TK_WHILE, cur, s, len);
        s += len;
        continue;
      }
    }
    if (
        strncmp(s, "==", 2) == 0 ||
        strncmp(s, "!=", 2) == 0 ||
        strncmp(s, "<=", 2) == 0 ||
        strncmp(s, ">=", 2) == 0)
    {
      cur = newToken(TK_RESERVED, cur, s, 2);
      s += 2;
      continue;
    }

    if (
        *s == '+' ||
        *s == '-' ||
        *s == '*' ||
        *s == '/' ||
        *s == '(' ||
        *s == ')' ||
        *s == '<' ||
        *s == '>' ||
        *s == ';' ||
        *s == '=')
    {
      cur = newToken(TK_RESERVED, cur, s, 1);
      s++;
      continue;
    }

    // 文字が入ったならば変数のはず
    if (isalpha(*s) || *s == '_')
    {
      // 変数文字列の長さ
      int len = 0;
      char *tmp = s;
      // 変数はアルファベットと数字とアンダーバーで構成されているはずなので、その長さを記録する
      while (isAlNumBar(*tmp))
      {
        len++;
        tmp++;
      }
      cur = newToken(TK_IDENT, cur, s, len);
      s += len;
      continue;
    }
    errorAt(s, "トークナイズできません");
  }
  newToken(TK_EOF, cur, s, 1);
  return head.next;
}

Node *code[100];

LoVar *findLoVar(Token *tkn)
{
  if (!loVarList)
  {
    return NULL;
  }
  for (LoVar *crnt = loVarList; crnt; crnt = crnt->next)
  {
    DEBUG_PRINT("#crnt->str=%s\n", crnt->str);
    DEBUG_PRINT("#len=%d\n", crnt->len);
    if (tkn->len == crnt->len && !memcmp(tkn->str, crnt->str, crnt->len))
    {
      return crnt;
    }
  }
  return NULL;
}

Node *newNode(NodeKind kind)
{
  Node *node = (Node *)calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

Node *newLRNode(NodeKind kind, Node *left, Node *right)
{
  Node *node = (Node *)calloc(1, sizeof(Node));
  node->kind = kind;
  node->left = left;
  node->right = right;
  return node;
}

Node *newNodeNum(int val)
{
  Node *node = (Node *)calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

/**
 * 変数のノードを作る
 */
Node *newNodeIdent(Token *tkn)
{
  Node *node = (Node *)calloc(1, sizeof(Node));
  node->kind = ND_LVAR;

  DEBUG_PRINT("#変数：%s\n", tkn->str);
  LoVar *lovar = findLoVar(tkn);
  DEBUG_PRINT("#変数あったかな？：%s\n\n", lovar ? "あった" : "なかった");
  if (!lovar)
  {
    // 変数新規作成
    lovar = (LoVar *)calloc(1, sizeof(LoVar));
    lovar->len = tkn->len;
    lovar->str = tkn->str;
    lovar->offset = loVarList->offset + 8;

    lovar->next = loVarList;
    loVarList = lovar;
  }
  node->offset = lovar->offset;
  return node;
}

void program()
{
  int i = 0;
  // EOFでなく、iが100未満でループ
  while (!atEOF() && i < 100)
  {
    code[i++] = stmt();
  }
  code[i] = NULL;
}

/*
    expr ";"|
    "return" expr ";"|
    "if" "(" expr ")" stmt ("else" stmt)?|
    "while" "(" expr ")" stmt|
    "for" "(" expr? ";" expr?";" expr? ")" stmt
*/
Node *stmt()
{
  Node *node;
  if (consumeByTokenKind(TK_IF))
  {
    expect("(");
    Node *condition = expr();
    expect(")");
    Node *then = stmt();
    Node *els = NULL;
    if (consumeByTokenKind(TK_ELSE))
    {
      els = stmt();
    }
    node = newNode(ND_IF);
    node->condition = condition;
    node->then = then;
    node->els = els;
    node->labelID = currentLabelID++;

    return node;
  }
  if (consumeByTokenKind(TK_WHILE))
  {
    expect("(");
    Node *condition = expr();
    expect(")");
    Node *then = stmt();
    node = newNode(ND_WHILE);
    node->condition = condition;
    node->then = then;
    node->labelID = currentLabelID++;

    return node;
  }
  if (consumeByTokenKind(TK_RETURN))
  {
    node = newLRNode(ND_RETURN, expr(), NULL);
  }
  else
  {
    node = expr();
  }
  expect(";");
  return node;
}

// assign
Node *expr()
{
  return assign();
}

// equality ("=" assign)?
Node *assign()
{
  Node *node = equality();
  if (consume("="))
  {
    node = newLRNode(ND_ASSIGN, node, assign());
  }
  return node;
}

// relational ("==" relational | "!=" relational)*
Node *equality()
{
  // 入れ替えがめんどくさかったから変数にまとめた
  Node *(*next)() = relational;
  Node *node = next();
  while (1)
  {
    if (consume("=="))
    {
      node = newLRNode(ND_EQL, node, next());
    }
    else if (consume("!="))
    {
      node = newLRNode(ND_NEQL, node, next());
    }
    else
    {
      return node;
    }
  }
}

// add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational()
{
  Node *(*next)() = add;
  Node *node = next();
  while (1)
  {
    if (consume(">="))
    {
      // アセンブリには以下の判定しかないため左右を入れ替える
      node = newLRNode(ND_LESS_THAN, next(), node);
    }
    else if (consume("<="))
    {
      node = newLRNode(ND_LESS_THAN, node, next());
    }
    else if (consume(">"))
    {
      node = newLRNode(ND_LESS, next(), node);
    }
    else if (consume("<"))
    {
      node = newLRNode(ND_LESS, node, next());
    }
    else
    {
      return node;
    }
  }
}

// mul ("+" mul | "-" mul)*
Node *add()
{
  Node *(*next)() = mul;
  Node *node = next();
  while (1)
  {
    if (consume("+"))
    {
      node = newLRNode(ND_ADD, node, next());
    }
    else if (consume("-"))
    {
      node = newLRNode(ND_SUB, node, next());
    }
    else
    {
      return node;
    }
  }
}

// mul=unary('*'unary | '/'unary)*
Node *mul()
{
  Node *(*next)() = unary;
  Node *node = next();
  while (1)
  {
    if (consume("*"))
    {
      node = newLRNode(ND_MUL, node, next());
    }
    else if (consume("/"))
    {
      node = newLRNode(ND_DIV, node, next());
    }
    else
    {
      return node;
    }
  }
}

// 単項＋ー　unary=('+'|'-')?primary
Node *unary()
{
  if (consume("+"))
  {
    return primary();
    ;
  }
  else if (consume("-"))
  {
    return newLRNode(ND_SUB, newNodeNum(0), primary());
  }
  return primary();
  ;
}

// primary=num|ident|'('expr')'
Node *primary()
{
  if (consume("("))
  {
    Node *node = expr();
    expect(")"); //)は閉じないとおかしい
    return node;
  }
  if (peekIdent())
  {
    return newNodeIdent(expectIdent());
  }
  return newNodeNum(expectNumber());
}