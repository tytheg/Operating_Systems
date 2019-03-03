/* Tyler Green          tgreen2         CIS 415 Project1
//
// This is all my own work except for the do_nanosleep function that
// was taken from Lab, Other code was influenced by examples in the
// lecture and lab
*/

#include "stdlib.h"
#include "stdio.h"
#include "p1fxns.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include  <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>
const int MAX_ARG_SIZE = 100;

typedef struct llnode
{
	int argsize;
	char **args;
	char *command;
	struct llnode *next;
	pid_t pid;
} LLnode;


typedef struct llist
{
	struct llnode *head;
	int size;
}LList;

int getWordCount(char str[])
{
	int counter = 0, idx = 0;
	char buff[MAX_ARG_SIZE];
	while( idx != -1)
	{
		idx = p1getword(str, idx, buff);
		if (idx != -1)
			++counter;
	}
	return counter;
	
}

void append(LLnode *head, LLnode *n)
{
	if (head->next == NULL)
	{
		head->next = n;
	}
	else
	{
		LLnode *current = head;
		while(1)
		{
			if(current->next == NULL)
			{
				current->next = n;
				break;
			}
			current = current->next;
		}
	}
}

void free_list(LList *ll)
{
	int i;
	LLnode *n = ll->head;
	LLnode *tmp;
	while(n != NULL)
	{
		tmp = n;
		n = n->next;
		free(tmp->command);
		for (i = 0; i < tmp->argsize; ++i)
		{
			free(tmp->args[i]);
		}
		free(tmp->args);
		free(tmp);
	}
}

void do_nanosleep(int nseconds) /* helper sleep function.*/
{
        struct timespec time, time2;
        time.tv_sec = 0;
        time.tv_nsec = nseconds;
        time2.tv_sec =0;
        time2.tv_nsec = 0;
        nanosleep( &time, &time2);
}


int main( int argc, char *argv[])
{
	char *p;
	char *qcheck = "--quantum";
	char qtmstr[MAX_ARG_SIZE];
	int qtm = -1, j = -1, i = 1;
	int fd;

	if ((p = getenv("USPS_QUANTUM_MSEC")) != NULL)
		qtm = p1atoi(p);

	if (argc > 0)
	{
		if (p1strneq(argv[1], qcheck, 6) == 1)
		{
			p1strcpy(qtmstr, argv[1]);
			j = p1strchr(qtmstr, '=');
			qtm = p1atoi(qtmstr + (j + 1));
			i = 2;
		}
	}
	if (qtm == -1)
	{
		p1perror(2, "QUANTUM UNDEFINED");
		return -1;
	}

	if (qtm < 20 || qtm > 1000)
	{
		p1putstr(2, "QUANTUM NOT VALID");
		return -1;
	}

	if (argc <= i)   //no filename passed in arguments
		fd = 0;
	else
	{
		fd = open(argv[i], 0);
		if (fd < 0)
		{
			p1perror(2, "FILE NOT FOUND");
			return -1;
		}
	}

	LList *ll = (LList *)malloc(sizeof(LList));
	ll->head = NULL;
	ll->size = 0;
	char line[500], arg[MAX_ARG_SIZE], com[MAX_ARG_SIZE];

	while(1)
	{
		int k = 0, idx = 0, argnum = 0;
		j = p1getline(fd, line, 500);
		if (j == 0)
			break;

		if (line[j-1] == '\n')
			line[j-1] = '\0';

		ll->size++;
		k = getWordCount(line);
		char **args = (char**)malloc(sizeof(char*) * (k+1));
		p1getword(line, 0, com);

		LLnode *llnode = (LLnode *)malloc(sizeof(LLnode));
		llnode->command = p1strdup(com);

		for (i = 0; i < k; ++i)
		{
			idx = p1getword(line, idx, arg);
			args[i] = p1strdup(arg);
			argnum++;
		}
		args[k] = NULL;

		llnode->argsize = argnum;
		llnode->args = args;
		llnode->next = NULL;
		

		if (ll->head == NULL)
			ll->head = llnode;
		else
			append(ll->head, llnode);
	}
	int ec = 0;
	LLnode *current = ll->head;
	pid_t pid[ll->size];
	for (i = 0; i < ll->size; ++i)
	{
		pid[i] = fork();
		if (pid[i] < 0)
		{
			p1perror(2, "FORK FAILED");
			ec = 1;
			goto cleanup;
		}
		if (pid[i] == 0)
		{
			execvp(current->command, current->args);
			p1perror(2, "EXECVP FAILED");
			ec = 2;
			goto cleanup;
		}
		do_nanosleep(100000000);
		current = current->next;
	}
	for (i = 0; i < ll->size; ++i)
	{
		wait(&pid[i]);
	}

cleanup:
	free_list(ll);
	free(ll);
	exit(ec);

}
