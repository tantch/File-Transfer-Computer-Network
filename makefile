all:

	gcc -o RCOM rcom.c application.c general.c validation.c link.c -w

 clean:

	rm -f RCOM
