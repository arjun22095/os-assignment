//
// Created by arj on 16/10/23.
//

#ifndef OS_AS3_SUBSCHED_H
#define OS_AS3_SUBSCHED_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "texts.h"
#include <time.h>

typedef struct {
	pid_t pid;
	clock_t start_time_t;
	clock_t end_time_t;
	text *name;
	int priority;
	int cpu_index;
	int queue_time;
	double exec_time;
	int end_time;
	int ran_continuously;
	int status; // 1 for terminated, else 0
} process;

typedef struct process_node process_node;

struct process_node {
	process *proc;
	process_node *next;
};

typedef struct {
	process_node *head;
	process_node *rear;
	int len;
} process_queue;

void enqueue(process_node *pn, process_queue *pq);

void enqueue_in_front(process_node *pn, process_queue *pq);

process_node *dequeue(process_queue *pq);

process *alloc_process(text *name, int priority, pid_t proc_pid);

process_node *alloc_process_node(process *proc);

process_queue *alloc_process_queue();

int run_proc(process *p, int tslice);

void print_proc(process *p);

void print_proc_node(process_node *pn);

void print_queue(process_queue *pq);

void add_process(char *command, int priority, process_queue *sq, process_queue *uq);

void free_process(process *p);

void free_process_node(process_node *pn);

void free_queue(process_queue *pq);

#endif //OS_AS3_SUBSCHED_H
