#include "9cc.h"

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

Node *newNode(NodeKind kind, Node *left, Node *right)
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
    if (consumeByTokenKind(TK_RETURN))
    {
        node = newNode(ND_RETURN, expr(), NULL);
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
        node = newNode(ND_ASSIGN, node, assign());
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
            node = newNode(ND_EQL, node, next());
        }
        else if (consume("!="))
        {
            node = newNode(ND_NEQL, node, next());
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
            node = newNode(ND_LESS_THAN, next(), node);
        }
        else if (consume("<="))
        {
            node = newNode(ND_LESS_THAN, node, next());
        }
        else if (consume(">"))
        {
            node = newNode(ND_LESS, next(), node);
        }
        else if (consume("<"))
        {
            node = newNode(ND_LESS, node, next());
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
            node = newNode(ND_ADD, node, next());
        }
        else if (consume("-"))
        {
            node = newNode(ND_SUB, node, next());
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
            node = newNode(ND_MUL, node, next());
        }
        else if (consume("/"))
        {
            node = newNode(ND_DIV, node, next());
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
        return newNode(ND_SUB, newNodeNum(0), primary());
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

void gen(Node *node)
{
    if (node->kind == ND_RETURN)
    {
        gen(node->left);
        printf("  pop rax\n");
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
        return;
    }
    // 終端生成
    switch (node->kind)
    {
    case ND_NUM:
        printf("  push %d\n", node->val);
        return;
    case ND_LVAR:
        genLval(node);
        DEBUG_PRINT("#変数ロード開始vvvv\n");
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
        DEBUG_PRINT("#変数ロード完了^^^^\n");
        return;
    case ND_ASSIGN:
        DEBUG_PRINT("#代入開始\n");
        genLval(node->left);
        gen(node->right);

        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  mov [rax], rdi\n");
        printf("  push rdi\n");
        DEBUG_PRINT("#代入ここまで\n");
        return;
    }
    gen(node->left);
    gen(node->right);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind)
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

void genLval(Node *node)
{
    if (node->kind != ND_LVAR)
        error("代入の左辺値が変数ではありません");
    DEBUG_PRINT("#変数\n");
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
    DEBUG_PRINT("#変数ここまで\n");
}