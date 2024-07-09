#include "9cc.h"

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