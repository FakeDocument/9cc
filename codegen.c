#include "9cc.h"

void gen(Node *node)
{
    if (node->kind == ND_BLOCK)
    {
        DEBUG_PRINT("\n# start block state\n");
        node = node->nextCode;
        while (node)
        {
            gen(node);
            printf("  pop rax\n");
            node = node->nextCode;
        }
        DEBUG_PRINT("\n# end block state\n");
        return;
    }
    if (node->kind == ND_IF)
    {
        genIf(node);
        return;
    }
    if (node->kind == ND_WHILE)
    {
        genWhile(node);
        return;
    }
    if (node->kind == ND_FOR)
    {
        genFor(node);
        return;
    }
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
        DEBUG_PRINT("\n# start local variable\n");
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
        DEBUG_PRINT("\n# end local variable\n");
        return;
    case ND_ASSIGN:
        DEBUG_PRINT("\n# start assign state\n");
        genLval(node->left);
        gen(node->right);

        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  mov [rax], rdi\n");
        printf("  push rdi\n");
        DEBUG_PRINT("\n# end assign state\n");
        return;
    }
    gen(node->left);
    gen(node->right);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    genExpr(node);

    printf("  push rax\n");
}

void genExpr(Node *node)
{
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
}
void genLval(Node *node)
{
    if (node->kind != ND_LVAR)
        error("代入の左辺値が変数ではありません");
    DEBUG_PRINT("# start gen local variable\n");
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
    DEBUG_PRINT("# end gen local variable\n");
}

void genIf(Node *node)
{
    DEBUG_PRINT("# start if state\n");
    DEBUG_PRINT("\n# start condition expr\n");
    gen(node->condition);
    DEBUG_PRINT("\n#  end condition expr\n");

    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    if (node->els == NULL)
    {
        printf("  je  .Lend%d\n", node->labelID);
        DEBUG_PRINT("\n# start then state\n");
        gen(node->then);
        DEBUG_PRINT("\n# end then state\n");
        printf(".Lend%d:\n", node->labelID);
        return;
    }
    else
    {
        printf("  je  .Lelse%d\n", node->labelID);
        DEBUG_PRINT("\n# start then state\n");
        gen(node->then);
        DEBUG_PRINT("\n# end then state\n");
        printf("  jmp  .Lend%d\n", node->labelID);
        printf(".Lelse%d:\n", node->labelID);
        DEBUG_PRINT("\n# start else state\n");
        gen(node->els);
        DEBUG_PRINT("\n# end else state\n");
        printf(".Lend%d:\n", node->labelID);
        return;
    }
}

void genWhile(Node *node)
{
    DEBUG_PRINT("\n# start while state\n");
    printf(".Lbegin%d:\n", node->labelID);

    DEBUG_PRINT("\n# start condition state\n");
    gen(node->condition);
    DEBUG_PRINT("\n# end condition state\n");

    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .Lend%d\n", node->labelID);

    DEBUG_PRINT("\n# start then state\n");
    gen(node->then);
    DEBUG_PRINT("\n# end then state\n");

    printf("  jmp  .Lbegin%d\n", node->labelID);
    printf(".Lend%d:\n", node->labelID);
    DEBUG_PRINT("\n# end while state\n");
    return;
}

void genFor(Node *node)
{
    DEBUG_PRINT("\n# start for state\n");

    if (node->init)
    {
        DEBUG_PRINT("\n# start init expr\n");
        gen(node->init);
        DEBUG_PRINT("\n# start init expr\n");
    }

    printf(".Lbegin%d:\n", node->labelID);

    if (node->condition)
    {
        DEBUG_PRINT("\n# start condition expr\n");
        gen(node->condition);
        DEBUG_PRINT("\n# end condition expr\n");
    }

    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .Lend%d\n", node->labelID);

    DEBUG_PRINT("\n# start then state\n");
    gen(node->then);
    DEBUG_PRINT("\n# end then state\n");

    if (node->update)
    {
        DEBUG_PRINT("\n# start update expr\n");
        gen(node->update);
        DEBUG_PRINT("\n# end update expr\n");
    }
    printf("  jmp  .Lbegin%d\n", node->labelID);
    printf(".Lend%d:\n", node->labelID);
    DEBUG_PRINT("\n# end for state\n");
    return;
}

void genFuncCall(Node *node)
{
    DEBUG_PRINT("\n# start gen call")
}
