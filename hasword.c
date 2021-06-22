
// JULIO 2020. CÉSAR BORAO MORATINOS: hasword.c

#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

enum {
	Max_word = 500,
	Max_files = 1024,
};

struct Cell {
	int pid;
	char word[Max_word];
};

typedef struct Cell Cell;

int
addtolist(int pid, char *word, Cell list[]) {

	int i = 0;
	while (strcmp(list[i].word,"\0") != 0) {
		i++;
	}

	if (i < Max_files) {
		strncpy(list[i].word,word,strlen(word)+1);
		list[i].pid = pid;
		list[i+1].word[0] = '\0';
		return 1;
	}
	return -1;
}

char *
getword(int pid, Cell *list) {
	int i = 0;
	while (strcmp(list[i].word,"\0") != 0) {
		if (list[i].pid == pid) {
			return list[i].word;
		}
		i++;
	}
	return NULL;
}

void
execute(char *input[], int *pid) {
	switch (*pid = fork()) {
		case -1:
			errx(EXIT_FAILURE, "error: fork failed!");
		case 0:
			execv("/bin/fgrep",input);
			errx(EXIT_FAILURE, "error: execv failed");
	}
}

void
runchilds(int argc, char *argv[], Cell *list) {
	int pid;
	for (size_t i = 0; i < argc; i+=2) {
		char *input[] = {"/bin/fgrep","-q","-s", argv[i+1],argv[i], NULL};
		execute(input,&pid);

		if (addtolist(pid,argv[i+1],list) < 0)
			errx(EXIT_FAILURE,"error: full list");
	}
}


void
collectstatus(Cell *list) {

	int status = 0;
	int pidwait = 0;

	while ((pidwait = wait(&status)) != -1) {
		if (!WIFEXITED(status))
			errx(EXIT_FAILURE, "child ends without exit() call");

		char *word;
		if ((word = getword(pidwait,list)) == NULL)
			errx(EXIT_FAILURE,"error: no word stored with this pid");

		switch (WEXITSTATUS(status)) {
			case 0:
				fprintf(stdout,"%s: si\n",word);
				break;
			case 1:
				fprintf(stdout,"%s: no\n",word);
				break;
			default:
				fprintf(stdout,"%s: error\n",word);
		}
	}
}

int
main(int argc, char *argv[]) {

	argv++;
	argc--;

	if (argc%2 != 0 || argc == 0)
		errx(EXIT_FAILURE, "usage: hasword [[file word]...]");

	Cell list[Max_files];
	list[0].word[0] = '\0';

	runchilds(argc,argv,list);
	collectstatus(list);

	exit(EXIT_SUCCESS);

}
