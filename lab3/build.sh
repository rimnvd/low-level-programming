cd client
bison -d parser.y
lexer lexer.l
cd ..
cmake .
make
rm client/lex.yy.c client/parser.tab.*
rm Makefile
