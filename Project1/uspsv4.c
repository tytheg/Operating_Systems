/* Tyler Green          tgreen2         CIS 415 Project1
//
// This is all my own work except for the enum child_status and the do_nanosleep
// function that were taken from Lab, Other code was influenced by examples in the
// lecture and lab
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h>
#include <time.h>
#include "p1fxns.h"

#define UNUSED __attribute__((unused))

const int MAX_ARG_SIZE = 100;

//run=0 when it hasn't started, 1 when it is currently running, and 2 when paused
volatile int run = 0;
int fd = -1;
int quantum = -1;

enum child_status
{
	new = 0,
	running,
	paused,
	terminated
};

typedef struct llnode
{
	int argsize;
	char **args;
	char *command;
	struct llnode *next;
	pid_t pid;
	enum child_status status;
} LLnode;

typedef struct llist
{
	struct llnode *head;
	int size;
}LList;

LList *ready = NULL;
LList *complete = NULL;

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

void append(LList *ll, LLnode *n)
{
	LLnode *head = ll->head;
	if (head == NULL)
		ll->head = n;
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
	ll->size++;
}


LLnode *remove_head( LList *ll)
{
	LLnode* n = ll->head;
	if (n == NULL)
		return n;
	ll->head = n->next;
	n->next = NULL;
	ll->size--;
	return n;
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

void childSignal(int signal)
{
	switch(signal)
	{
		case SIGUSR1:
			run = 1;
			break;
		//case SIGSTOP:
		//	run = 2;
		//	break;
		case SIGCONT:
			if(run == 2)
				run = 1;
			break;
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

int parse_main(int argc, char* argv[])
{
	char *p;
	char *qcheck = "--quantum";
	char qtmstr[MAX_ARG_SIZE];
	int j = -1, i = 1;

	if ((p = getenv("USPS_QUANTUM_MSEC")) != NULL)
		quantum = p1atoi(p);

	if (argc > 0)
	{
		if (p1strneq(argv[1], qcheck, 6) == 1)
		{
			p1strcpy(qtmstr, argv[1]);
			j = p1strchr(qtmstr, '=');
			quantum = p1atoi(qtmstr + (j + 1));
			i = 2;
		}
	}
	if (quantum == -1)
	{
		p1perror(2, "QUANTUM UNDEFINED");
		return -1;
	}

	if (quantum < 20 || quantum > 1000)
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
	return 0;
}

int build_list(void )
{
	int i, j, k, idx, argnum;
	ready = (LList *)malloc(sizeof(LList));
	ready->head = NULL;
	ready->size = 0;

	complete = (LList *)malloc(sizeof(LList));
	complete->head = NULL;
	complete->size = 0;

	char line[500], arg[MAX_ARG_SIZE], com[MAX_ARG_SIZE];

	while(1)
	{
		idx = 0;
		argnum = 0;
		j = p1getline(fd, line, 500);
		if (j == 0)
			break;

		if (line[j-1] == '\n')
			line[j-1] = '\0';

		k = getWordCount(line);
		char **args = (char**)malloc(sizeof(char*) * (k+1));
		if (args == NULL)
			return -1;
		p1getword(line, 0, com);

		LLnode *llnode = (LLnode *)malloc(sizeof(LLnode));
		if (llnode == NULL)
			return -1;
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
		llnode->pid = -1;
		llnode->status = 0;
		
		append(ready, llnode);
	}
	return 0;
}

int check_child(pid_t pid)
{
	int status;
	int id = waitpid(pid, &status, WNOHANG);
	
	//Return 1 if running
	if (id != pid)
		return 1;
	//Return 0 if exited
	return 0;
}

int first_time = 1;

int show_stats(LLnode *n)
{
	pid_t pid = n->pid;
	int f, i;
	char filename[MAX_ARG_SIZE];
	char buffer[MAX_ARG_SIZE];
	char stat[MAX_ARG_SIZE];
	p1strcpy(filename, "/proc/");
	p1itoa(pid, &filename[6]);
	p1strcat(filename, "/io");
//pid cmd status ppid ppid

	p1strcpy(stat, "pid: ");
	p1itoa(pid, &stat[5]);
	p1putstr(1, stat);
	p1putstr(1, "\n");

	p1strcpy(stat, "command: ");
	p1strcat(stat, n->command);
	p1putstr(1, stat);
	p1putstr(1, " ");

	for (i = 1; i < n->argsize; ++i)
	{
		p1putstr(1, n->args[i]);
	}
	
	p1putstr(1, "\n");
	f = open(filename, 0);
	if (f < 0)
	{
		p1perror(2, "UNABLE TO OPEN PROC/PID/STAT");
		return -1;
	}
	
	while(p1getline(f, buffer, MAX_ARG_SIZE) > 0)
	{
		p1putstr(1, buffer);
	}

	close(f);
	p1putstr(1, "\n");
	p1putstr(1, "\n");
	p1putstr(1, "\n");
	return 1;

}

void sigalarm( UNUSED int signal )
{
	LLnode *n;	

	while(1)
	{
		n = ready->head;
		if (n == NULL)
			return;
	//	printf("IN SIGALARM, pid = %d, status = %d\n", n->pid, n->status);
		switch(n->status)
		{
		 	case new:
				kill(n->pid, SIGUSR1);
				n->status = running;
				return;
	
		 	case running:
				if (check_child(n->pid) == 1)
				{
					if (ready->size == 1)	//ONLY ONE TASK LEFT, LET IT RUN
						return;
					kill(n->pid, SIGSTOP);
					n->status = paused;
					show_stats(n);
				}
				else
					n->status = terminated;
				remove_head(ready);
				append(ready, n);
				break;
	
		 	case paused:
				kill(n->pid, SIGCONT);
				n->status = running;
				return;
	
		 	case terminated:
				remove_head(ready);
				append(complete, n);
				break;
		}
	}
}

int main( int argc, char *argv[])
{

	int i;
	int ec = parse_main(argc, argv);
	if (ec != 0)
		return ec;
	ec = build_list();	
	if (ec != 0)
		return ec;


	struct itimerval itv;
	itv.it_value.tv_sec = quantum/1000;
	itv.it_value.tv_usec = (quantum*1000) % 1000000;
	itv.it_interval = itv.it_value;

	LLnode *current = ready->head;

	for (i = 0; i < ready->size; ++i)
	{
		current->pid = fork();
		if (current->pid < 0)
		{
			p1perror(2, "FORK FAILED");
			ec = 1;
			goto cleanup;
		}
		if (current->pid == 0)
		{
			signal(SIGUSR1, childSignal);
			while(run != 1)
			{	
				do_nanosleep(50000);
			}
			execvp(current->command, current->args);
			p1perror(2, "EXECVP FAILED ");
			ec = 2;
			goto cleanup;
		}
		do_nanosleep(10000000);
		current = current->next;
	}
	signal(SIGALRM, sigalarm);

	if( setitimer(ITIMER_REAL, &itv, NULL) == -1)
	{
		p1perror(2, "UNABLE TO SET TIMER\n");
		exit(1);
	}

	while(ready->head !=NULL)
	{
		do_nanosleep(50000000);
	}

cleanup:
	free_list(ready);
	free_list(complete);
	free(ready);
	free(complete);
	exit(ec);
}
