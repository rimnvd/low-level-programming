bison -d parser.y
flex lexer.l
gcc -w lex.yy.c parser.tab.c main.c -o out
./out < query.ms