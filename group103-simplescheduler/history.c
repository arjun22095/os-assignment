//
// Created by arj on 16/10/23.
//

#include "history.h"


struct ll_node *alloc_node() {
	struct ll_node *node = (struct ll_node *) malloc(sizeof(struct ll_node));
	if (node == NULL) {
		printf("[ERROR IN MALLOC]\n");
		return NULL;
	}
	node->t_val = NULL;
	node->next = NULL;
	node->prev = NULL;
	return node;
}

struct text_ll *init_history() {
	struct text_ll *h = (struct text_ll *) malloc(sizeof(struct text_ll));
	if (h == NULL) {
		printf("[ERROR IN MALLOC]\n");
		return NULL;
	}
	h->front = NULL;
	h->rear = NULL;
	h->len = 0;
	return h;
}

struct ll_node *add_to_history(struct text_ll *h, struct ll_node *node) {
	if (h->len == 0) {
		h->rear = node;
		h->front = node;
		++h->len;
		return NULL;
	}

	node->prev = h->front;
	h->front->next = node;
	h->front = node;
	++h->len;
	return node;
}

void set_call_time(struct ll_node *node) {
    time_t cur_time;
    time(&cur_time);
    struct tm *temp = localtime(&cur_time);;
    strftime(node->start_time_str, sizeof(node->start_time_str), "%I:%M:%S %p", temp);
}

void print_history(struct text_ll *h) {
	struct ll_node *cur_node = h->rear;
	for (int i = 1; cur_node != NULL; ++i) {
		printf("[%d.] [auth: %d] [PID : %d] %s | [Started : %s] [%f]\n", i, cur_node->author, cur_node->pid,cur_node->t_val->str_val, cur_node->start_time_str,
			   cur_node->duration);
		cur_node = cur_node->next;
	}
}
