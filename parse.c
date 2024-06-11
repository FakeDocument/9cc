#include "9cc.h"

struct Token
{
  TokenKind kind;
  Token *next;
  int num;    //トークンが数値の場合、その数値
  char *str;  //トークン文字列
  unsigned int len;
};

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
次のトークンが変数の時はトークンを進めずTrue
それ以外ならFalse
*/
bool peekIdent()
{
  if(token->kind!=TK_IDENT)
    {
      return false;
    }
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
      errorAt(token->str,"%sではありません\n%sです\n",op,token->str);
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

/*
次のトークンが変数の場合、トークンを1つ読み進めてその変数を返す。
それ以外の場合にはエラーを報告する。
*/
char* expectIdent()
{
  if(token->kind!=TK_IDENT)
    errorAt(token->str,"変数ではありません");
  char* ident=token->str;
  token=token->next;
  return ident;
}

bool atEOF()
{
  return token->kind==TK_EOF;
}

Token* newToken(TokenKind kind,Token *cur,char *str,int len)
{
  Token *tkn=(Token*)calloc(1,sizeof(Token));
  tkn->kind=kind;
  tkn->str=str;
  tkn->len=len;
  cur->next=tkn;
  return tkn;
}

//入力文字列をトークン化する
Token* tokenizer(char *s){
  Token head;
  head.next=NULL;
  Token *cur=&head;
  while(*s)
  {

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
      *s=='>'||
      *s==';'||
      *s=='=')
    {
      cur=newToken(TK_RESERVED,cur,s,1);
      s++;
      continue;
    }

    if('a'<=*s&&*s<='z')
    {
      printf("#%s\n",s);
      cur=newToken(TK_IDENT,cur,s,1);
      s++;
      continue;
    }
    errorAt(s,"トークナイズできません");
  }
  newToken(TK_EOF,cur,s,1);
  return head.next;
}