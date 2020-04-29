#define _GNU_SOURCE

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <sched.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include "process.h"

int process_priority(int isHighPriority, int pid){
	struct sched_param param;
	param.sched_priority = 0;
	int order;

	if (isHighPriority){
		order = sched_setscheduler(pid, SCHED_OTHER, &param);
	}
	else{
		order = sched_setscheduler(pid, SCHED_IDLE, &param);
	} 
	
	if (order < 0) {
		perror("sched_setscheduler");
		return -1;
	}
	return order;
}

int process_to_cpu(int pid, int core)
{
	if (core > sizeof(cpu_set_t)) {
		perror("Unrecognize the core");
		return -1;
	}

	cpu_set_t set;
	CPU_ZERO(&set);
	CPU_SET(core, &set);
		
	if (sched_setaffinity(pid, sizeof(set), &set) < 0) {
		perror("sched_setaffinity");
		exit(1);
	}

	return 0;
}

int process_execute(struct child process)
{
	int pid = fork();
	int this_pid;

	if (pid < 0) {
		perror("fork");
		return -1;
	}

	if (pid == 0) {
		unsigned long start_sec, start_nsec, end_sec, end_nsec;
		this_pid = getpid();
		syscall(334, 1, &this_pid, &start_sec, &start_nsec, &end_sec, &end_nsec);
		for (int i = 0; i < process.execute_time; i++) {
			DEF_UNIT_TIME();
#ifdef DEBUG
			if (i % 100 == 0)
				fprintf(stderr, "%s: Time %d Total Execution Time %d\n", process.name, i, process.execute_time);
#endif
		}		
		syscall(334, 0, &this_pid, &start_sec, &start_nsec, &end_sec, &end_nsec);
#ifdef DEBUG			
		fprintf(stdout, "%s %d\n", process.name, this_pid);
#endif			

		exit(0);
	}
	
	process_to_cpu(pid, CORE_FOR_CHILD);
	return pid;
}

