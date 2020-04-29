#define _GNU_SOURCE
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/wait.h>
#include <sys/syscall.h>
#include <sched.h>
#include <errno.h>

#include "process.h"

#define FIFO 1
#define RR 	 2
#define SJF	 3
#define PSJF 4

static int running;
static int proc_complete;
static int n_proc_time;
static int last_time;

int FIFO_scheduler(struct child *process, int num_process){
	if (running != -1)
		return running;
	int order = -1;
	for (int i = 0; i < num_process; i++){
		if(process[i].pid == -1 || process[i].execute_time == 0){
			continue;
		}
		if(order == -1 || process[i].ready_time < process[order].ready_time){
			order = i;
		}
	}
	return order;		
}

int SJF_PSJF_scheduler(struct child *process, int num_process, int policy){
	if((policy == SJF) && (running != -1)){
		return running;
	}		
	int order = -1;
	for (int i = 0; i < num_process; i++){
		if(process[i].pid == -1 || process[i].execute_time == 0){
			continue;
		}
		if(order == -1 || process[i].execute_time < process[order].execute_time){
			order = i;
		}
	}
	return order;
}

int RR_scheduler(struct child *process, int num_process, int n_proc_time, int last_time){
	int order = -1;
	if (running == -1) {
		for (int i = 0; i < num_process; i++) {
			if (process[i].pid != -1 && process[i].execute_time > 0){
				order = i;
				break;
			}
		}
	}
	else if ((n_proc_time - last_time) % 500 == 0)  {
		order = (running + 1) % num_process;
		while (process[order].pid == -1 || process[order].execute_time == 0)
			order = (order + 1) % num_process;
	}
	else
		order = running;
	return order;
}

int cmp_func(const void *a, const void *b) {
	return ((struct child *)a)->ready_time - ((struct child *)b)->ready_time;
}

int scheduling(struct child *process, int num_process, int policy)
{
	qsort(process, num_process, sizeof(struct child), cmp_func);
	for (int i = 0; i < num_process; i++)
		process[i].pid = -1;

	process_to_cpu(getpid(), CORE_FOR_PARENT);
	process_priority(1, getpid());
	
	// Initial value
	n_proc_time = 0;
	running = -1;
	proc_complete = 0;
	
	while(1) {
		if (running != -1 && process[running].execute_time == 0) {
		
#ifdef DEBUG
			fprintf(stderr, "%s end_time %d.\n", process[running].name, n_proc_time);
#endif
			waitpid(process[running].pid, NULL, 0);
#ifdef DEBUG			
			fprintf(stdout, "%s %d\n", process[running].name, process[running].pid);
#endif			
			running = -1;
			proc_complete++;

			// finish all 
			if (proc_complete == num_process)
				break;
		}

		// check the process ready time to excute the process
		for (int i = 0; i < num_process; i++) {
			if (process[i].ready_time == n_proc_time) {
				
#ifdef DEBUG
				fprintf(stderr, "%s ready_time %d.\n", process[i].name, n_proc_time);
#endif
				process[i].pid = process_execute(process[i]);
				process_priority(0, process[i].pid);			
			}

		}

		// running  process prototype
		int next;
		if(policy == 1){
			next = FIFO_scheduler(process, num_process);
		}
		else if(policy == 2){
			next = RR_scheduler(process, num_process, n_proc_time, last_time);
		}
		else if((policy == 3) || (policy == 4)){
			next = SJF_PSJF_scheduler(process, num_process, policy);
		}

		if (next != -1) {
			// Context switch
			if (running != next) {
				process_priority(1, process[next].pid);
				process_priority(0, process[running].pid);
				running = next;
				last_time = n_proc_time;
			}
		}

		// Run an unit of time
		DEF_UNIT_TIME();
		if (running != -1)
			process[running].execute_time--;
		n_proc_time++;
	}

	return 0;
}

int main() {
	char policy_name[MAX_LENGTH];
	int policy;
	int num_process;
	struct child *process; //Input num_process of the child processes, P1, P2...

	scanf("%s", policy_name);
	scanf("%d", &num_process);
	
	process = (struct child *)malloc(num_process * sizeof(struct child));

	for (int i = 0; i < num_process; i++) {
		scanf("%s%d%d", process[i].name, &process[i].ready_time, &process[i].execute_time);
	}

	if (strcmp(policy_name, "FIFO") == 0) {
		policy = FIFO;
	}
	else if (strcmp(policy_name, "SJF") == 0) {
		policy = SJF;
	}
	else if (strcmp(policy_name, "PSJF") == 0) {
		policy = PSJF;
	}
	else if (strcmp(policy_name, "RR") == 0) {
		policy = RR;
	}
	else {
		perror("Invalid_Policy");
		exit(1);
	}

	scheduling(process, num_process, policy);

	exit(0);
}

