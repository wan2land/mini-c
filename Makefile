CC = gcc
PP = g++
LEX = flex
YACC = bison
TESTER = sh tester.sh

minic: minic.y minic.l minic_ast.c minic_ast.h ucode.c ucode.h
	$(YACC) -d minic.y
	$(LEX) minic.l
	$(CC) -o minic lex.yy.c minic.tab.c ucode.c minic_ast.c -ly -ll

clean:
	@rm -rf *.tab.? lex.* minic

ucodei: clean_ucodei
	$(PP) -o ucodei ucode-interpreter/main.cpp

clean_ucodei:
	@rm -f ucodei

test: minic
	$(TESTER)
