CC = gcc
LEX = flex
YACC = bison
NAME = minic

minic: clean
	$(YACC) -d $(NAME).y
	$(LEX) $(NAME).l
	$(CC) -o minic lex.yy.c $(NAME).tab.c -ly -ll

clean:
	@rm -rf *.tab.? lex.* $(NAME)
