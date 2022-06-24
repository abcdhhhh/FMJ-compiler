%{
#include "fmjAST.h"
#include "util.h"
#include "y.tab.h"
#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line; \
    yylloc.first_column = yylloc.last_column; \
    for(int i = 0; yytext[i] != '\0'; i++) { \
        if(yytext[i] == '\n') { \
            yylloc.last_line++; \
            yylloc.last_column = 0; \
        } \
        else { \
            yylloc.last_column++; \
        } \
    }
int c;
%}
%option yylineno
%x comment multicomment
%%
"//" {
    BEGIN(comment);
}
<comment>"\n" {
    BEGIN(INITIAL);
}
<comment>. {}

"/*" {
    BEGIN(multicomment);
}
<multicomment>"*/" {
    BEGIN(INITIAL);
}
<multicomment>"\n" {}
<multicomment>. {}

"class"   {
    yylval.str = String(yytext);
    return(CLASS);
}
"public"   {
    yylval.str = String(yytext);
    return(PUBLIC);
}
"static"   {
    yylval.str = String(yytext);
    return(STATIC);
}
"int"     {
    yylval.str = String(yytext);
    return(INT);
}
"main"   {
    yylval.str = String(yytext);
    return(MAIN);
}
"String"   {
    yylval.str = String(yytext);
    return(STRING);
}
"extends"   {
    yylval.str = String(yytext);
    return(EXTENDS);
}
"bool"     {
    yylval.str = String(yytext);
    return(BOOL);
}
"if"      {
    yylval.str = String(yytext);
    return(IF);
}
"else"    {
    yylval.str = String(yytext);
    return(ELSE);
}
"while"    {
    yylval.str = String(yytext);
    return(WHILE);
}
"continue"    {
    yylval.str = String(yytext);
    return(CONTINUE);
}
"break"    {
    yylval.str = String(yytext);
    return(BREAK);
}
"return"    {
    yylval.str = String(yytext);
    return(RETURN);
}
"putint"   {
    yylval.str = String(yytext);
    return(PUTINT);
}
"putch"   {
    yylval.str = String(yytext);
    return(PUTCH);
}
"putarray"   {
    yylval.str = String(yytext);
    return(PUTARRAY);
}
"starttime"   {
    yylval.str = String(yytext);
    return(STARTTIME);
}
"stoptime"   {
    yylval.str = String(yytext);
    return(STOPTIME);
}
"length"   {
    yylval.str = String(yytext);
    return(LENGTH);
}
"this"   {
    yylval.str = String(yytext);
    return(THIS);
}
"new"   {
    yylval.str = String(yytext);
    return(NEW);
}
"getint"   {
    yylval.str = String(yytext);
    return(GETINT);
}
"getch"   {
    yylval.str = String(yytext);
    return(GETCH);
}
"getarray"   {
    yylval.str = String(yytext);
    return(GETARRAY);
}
"true"   {
    yylval.str = String(yytext);
    return(TRUE_);
}
"false"   {
    yylval.str = String(yytext);
    return(FALSE_);
}
"&&"   {
    yylval.str = String(yytext);
    return(AND);
}
"||"   {
    yylval.str = String(yytext);
    return(OR);
}
"<="   {
    yylval.str = String(yytext);
    return(LE);
}
"=="   {
    yylval.str = String(yytext);
    return(EQ);
}

" "       ;
[\t\r]      ;
[\n]      {
    // printf("(enter)\n");
}
[a-zA-Z][a-zA-Z0-9_]*     {
    yylval.str = String(yytext);
    // printf("got id: %s\n", yytext);
    return(id);
}
[0-9]     {
    c = yytext[0];
    yylval.i = c - '0';
    // printf("got digit: %c\n", c);
    return(DIGIT);
}
[^a-z0-9\b]    {
    c = yytext[0];
    return(c);
}
<<EOF>> {
    static int cnt = 0;
    // printf("EOF %d\n", cnt);
    return cnt++ ? 0 : EOF_;
}
. {
    return yytext[0];
}
%%
