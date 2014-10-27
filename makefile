all:

	gcc -o RCOM rcom.c application.c general.c validation.c -w

 clean:

	rm -f RCOM