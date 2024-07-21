#include "9cc.h"

int currentLabelID = 0;

Node *codeHead = NULL;
Node *codeEnd = NULL;

LoVar *findLoVar(Token *tkn)
{
  if (!loVarList)
  {
    return NULL;
  }
  for (LoVar *crnt = loVarList; crnt; crnt = crnt->next)
  {
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

  LoVar *lovar = findLoVar(tkn);
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
  node->str = lovar->str;
  node->len = lovar->len;
  return node;
}

void program()
{

  int i = 0;
  codeHead = stmt();
  codeEnd = codeHead;
  // EOFでなく、iが100未満でループ
  while (!atEOF())
  {
    Node *node = stmt();
    codeEnd->nextCode = node;
    codeEnd = node;
  }
  codeEnd->nextCode = NULL;
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
  if (consume("{"))
  {
    Node *head = newNode(ND_BLOCK);
    Node *end = head;
    /* @todo }がない場合の処理も考える*/
    while (!consume("}"))
    {
      Node *crnt = stmt();
      end->nextCode = crnt;
      end = crnt;
    }
    end->nextCode = NULL;
    return head;
  }
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
  if (consumeByTokenKind(TK_FOR))
  {
    Node *init = NULL;
    Node *condition = NULL;
    Node *update = NULL;
    expect("(");
    if (!peek(";"))
    {
      init = expr();
    }
    expect(";");
    if (!peek(";"))
    {
      condition = expr();
    }
    expect(";");
    if (!peek(")"))
    {
      update = expr();
    }
    expect(")");
    Node *then = stmt();
    node = newNode(ND_FOR);
    node->init = init;
    node->condition = condition;
    node->update = update;
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
  if (peekByTokenKind(TK_IDENT))
  {
    Node *node = newNodeIdent(expectIdent());
    if (!consume("("))
    {
      // 変数の場合
      return node;
    }
    // 関数の場合
    expect(")");
    node->kind = ND_CALL;
    DEBUG_PRINT("# node len = %d\n", node->len);
    node->str[node->len] = '\0';
    return node;
  }
  return newNodeNum(expectNumber());
}