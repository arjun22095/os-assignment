//
// Created by arj on 16/10/23.
//

#ifndef OS_AS3_TEXTS_H
#define OS_AS3_TEXTS_H

#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_DELIMITER '\n'

typedef struct {
	char *str_val;
	int str_len;
	int capacity;
} text;

int get_char_count(char *str, char delim);
char *alloc_str(int capacity);
text *alloc_text();
void print_str(char *str, int len);
void printx(text *t);
char *realloc_str(int old_capacity, const char *old_str);
char *vacuum_str(int len, const char *old_str);
void vacuum(text *t);
int len(text *t);
int capacity(text *t);
int is_char_lower(char c);
int is_char_upper(char c);
int is_char_alpha(char c);
int is_char_num(char c);
int int_of_char(char c);
char lower_of_char(char c);
char upper_of_char(char c);
int is_lower(text *t);
int is_upper(text *t);
int is_num(text *t);
int is_just_spaces(text *t);
int is_alpha(text *t);
void lower(text *t);
void upper(text *t);
int intx(text *t);
int is_empty(char c);
char *slice_str(int new_len, int start, int end, int gap, const char *old_str);
char *str_copy(char *str, int str_len);
text *copy(text *t);
text *slice(int start, int end, int gap, text *t);
void strip(text *t);
void reverse_str(int len, char *str);
void reverse(text *t);
void text_input(text *t);
int str_equal(char *str_1, char *str_2, int str_len);
int equals(text *t1, text *t2);
text *create_text_from_params(char *str, char delimiter);
void free_text(text *t);
text *inputx();


#endif //OS_AS3_TEXTS_H
