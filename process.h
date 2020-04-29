#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <sys/types.h>

#define MAX_LENGTH 1000000

#define CORE_FOR_CHILD 	1
#define CORE_FOR_PARENT 0

// The definition of unit of time 
#define DEF_UNIT_TIME() \
{							\
	volatile unsigned long i;	\
	for (i = 0; i < 1000000UL; i++);	\
}						\

struct child {
	char name[MAX_LENGTH];
	int ready_time;
	int execute_time;
	pid_t pid;
};

int process_priority(int isHighPriority, int pid);
int process_to_cpu(int pid, int core);
int process_execute(struct child process);

#endif
