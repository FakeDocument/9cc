#include "9cc.h"

struct Node
{
    NodeKind kind;
    Node *left, *right;
    int val;
};

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
/*
1+2*(3+4)-5
EBNF
expr=mul('+'mul | '-'mul)*
*/

Node *expr()
{
    return equality();
}

// relational ("==" relational | "!=" relational)*
Node *equality()
{
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

// primary=num|'('expr')'
Node *primary()
{
    if (consume("("))
    {
        Node *node = expr();
        expect(")"); //)は閉じないとおかしい
        return node;
    }
    return newNodeNum(expectNumber());
}

void gen(Node *node)
{
    if (node->kind == ND_NUM)
    {
        printf("  push %d\n", node->val);
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