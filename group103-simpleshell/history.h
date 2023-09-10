//
// Created by arj on 10/9/23.
//

#ifndef SHALPHA_HISTORY_H
#define SHALPHA_HISTORY_H

#include "texts.h"
#include <time.h>

struct ll_node{
	text *t_val;
	struct ll_node *next;
	struct ll_node *prev;
	pid_t pid;
	double duration;
	char start_time_str[20];
	clock_t start_time;
	int author; // 0 for user and 1 for subprocess
	// if user typed the command then author is set to 0
	// if user called a command which in turn called this, author is set to 1
};


struct text_ll{
	struct ll_node *front;
	struct ll_node *rear;
	int len;
};

struct ll_node *alloc_node();

struct text_ll *init_history();

struct ll_node *add_to_history(struct text_ll *h, struct ll_node *node);

void set_call_time(struct ll_node *node);

void print_history(struct text_ll *h);


#endif //SHALPHA_HISTORY_H
