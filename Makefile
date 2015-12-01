CC = gcc
PP = g++
LEX = flex
YACC = bison
NAME = minic

minic: clean
	$(YACC) -d $(NAME).y
	$(LEX) $(NAME).l
	$(CC) -o $(NAME) lex.yy.c $(NAME).tab.c $(NAME)_ast.c -ly -ll

clean:
	@rm -rf *.tab.? lex.* $(NAME)

ucodei: clean_ucodei
	$(PP) -o ucodei ucode-interpreter/main.cpp

clean_ucodei:
	@rm -f ucodei
