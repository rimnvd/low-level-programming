#include <stdio.h>
#include "ast.h"
#include "parser.tab.h"

int main() {
    return yyparse();
}