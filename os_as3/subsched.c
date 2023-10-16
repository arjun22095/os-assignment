//
// Created by arj on 16/10/23.
//

#include "subsched.h"


void enqueue(process_node *pn, process_queue *pq) {
	if (pq->len == 0) {
		pn->next = NULL;
		pq->head = pn;
		pq->rear = pn;
	} else {
		pq->rear->next = pn;
		pq->rear = pn;
	}
	++pq->len;
}

void enqueue_in_front(process_node *pn, process_queue *pq) {
	if (pq->len == 0) {
		pn->next = NULL;
		pq->head = pn;
		pq->rear = pn;
	} else {
		pn->next = pq->head;
		pq->head = pn;
	}
	++pq->len;
}

process_node *dequeue(process_queue *pq) {
	if (pq->len == 0) {
		return NULL;
	} else if (pq->len == 1) {
		process_node *to_ret = pq->head;
		pq->head = NULL;
		pq->rear = NULL;
		pq->len = 0;
		to_ret->next = NULL;
		return to_ret;
	} else {
		process_node *to_ret = pq->head;
		pq->head = pq->head->next;
		--pq->len;
		to_ret->next = NULL;
		return to_ret;
	}
}

process *alloc_process(char *name, int priority, pid_t proc_pid) {
	process *to_ret = (process *) malloc(sizeof(process));
	to_ret->priority = priority;
	to_ret->name = name;
	to_ret->pid = proc_pid;
	to_ret->exec_time = 0;
	to_ret->ran_continuously = 0;
	return to_ret;
}

process_node *alloc_process_node(process *proc) {
	process_node *to_ret = (process_node *) malloc(sizeof(process_node));
	to_ret->proc = proc;
	to_ret->next = NULL;
	return to_ret;
}

process_queue *alloc_process_queue() {
	process_queue *pq = (process_queue *) malloc(sizeof(process_queue));
	pq->head = NULL;
	pq->rear = NULL;
	pq->len = 0;
	return pq;
}

int run_proc(process *p, int tslice) {
	printf("Running proc %d\n", p->pid);
	int cont_result = kill(p->pid, SIGCONT);
	sleep(tslice);
	int stop_result = kill(p->pid, SIGSTOP);
	if (stop_result == -1) {
		printf("SIGSTOP error received!\n");
		p->status = 1;
	}
	exit(1);
}

void print_proc(process *p) {
	printf("proc name:\t%s\n", p->name);
	printf("proc pid:\t%d\n", p->pid);
	printf("proc status:\t%d\n", p->status);
	printf("proc priority:\t%d\n", p->priority);
	printf("proc contd:\t%d\n", p->ran_continuously);
	printf("proc execd:\t%f\n", p->exec_time);
	printf("------------------\n");
}

void print_proc_node(process_node *pn) {
	print_proc(pn->proc);
}

void print_queue(process_queue *pq) {
	process_node *cur_node = pq->head;
	for (int i = 0; i < pq->len; ++i) {
		print_proc_node(cur_node);
		cur_node = cur_node->next;
	}
}


void add_process(char *command, int priority, process_queue *sq, process_queue *uq) {
	int fork_status = fork();
	if (fork_status == 0) {
		setpgid(0, 0);
		int exec_status = execlp(command, command, (char *) NULL);
		perror("Execution failed!\n");
	} else if (fork_status > 0) {
		int stop_status = kill(fork_status, SIGSTOP);
		process *p = alloc_process(command, priority, fork_status);
		process_node *pn_for_sq = alloc_process_node(p);
		process_node *pn_for_uq = alloc_process_node(p);
		enqueue(pn_for_sq, sq);
		enqueue(pn_for_uq, uq);
	} else {
		perror("Fork failed!\n");
	}
}