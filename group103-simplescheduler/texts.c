//
// Created by arj on 16/10/23.
//

#include "texts.h"

//typedef struct {
//	char *str_val;
//	int str_len;
//	int capacity;
//} text;

int get_char_count(char *str, char delim) {
	int result = 0;
	for (int i = 0; str[i] != '\0'; ++i) {
		if (str[i] == delim) {
			++result;
		}
	}
	return result;
}


int my_ceil(float x) {
	if (x > (int) x) {
		return (int) x + 1;
	}
	return (int) x;
}

char *alloc_str(int capacity) {
	char *str = malloc(capacity * (sizeof(char)));
	return str;
}


text *alloc_text() {
	text *t = malloc(sizeof(text));
	t->str_len = 0;
	t->str_val = NULL;
	return t;
}


void print_str(char *str, int len) {
	for (int i = 0; i < len; ++i) {
		printf("%c", str[i]);
	}
}


void printx(text *t) {
	printf("\n");
	printf("Capacity is %d\n", t->capacity);
	printf("Length is %d\n", t->str_len);
	print_str(t->str_val, t->str_len);
}


char *realloc_str(int old_capacity, const char *old_str) {
	int new_capacity = old_capacity * 2;
	char *new_str = malloc(new_capacity * sizeof(char));
	for (int i = 0; i < old_capacity; ++i) {
		new_str[i] = old_str[i];
	}
	return new_str;
}


char *vacuum_str(int len, const char *old_str) {
	char *new_str = malloc((len + 1) * sizeof(char));
	for (int i = 0; i < len; ++i) {
		new_str[i] = old_str[i];
	}
	new_str[len] = '\0';
	return new_str;
}


void vacuum(text *t) {
	if (t->capacity == t->str_len) {
		return;
	}

	char *new_str = vacuum_str(t->str_len, t->str_val);
	free(t->str_val);
	t->str_val = new_str;
	t->capacity = t->str_len;
}


int len(text *t) {
	return t->str_len;
}

// + 32 to go from UPPER to lower

int capacity(text *t) {
	return t->capacity;
}


int is_char_lower(char c) {
	if (c >= 97 && c <= 122) {
		return 1;
	}
	return 0;
}


int is_char_upper(char c) {
	if (c >= 65 && c <= 90) {
		return 1;
	}
	return 0;
}


int is_char_alpha(char c) {
	if (is_char_lower(c) == 1 || is_char_upper(c) == 1) {
		return 1;
	}
	return 0;
}


int is_char_num(char c) {
	if (c >= 48 && c <= 57) {
		return 1;
	}
	return 0;
}


int int_of_char(char c) {
	return c - 48;
}


char lower_of_char(char c) {
	return c + 32;
}


char upper_of_char(char c) {
	return c - 32;
}


int is_lower(text *t) {
	for (int i = 0; i < t->str_len; ++i) {
		if (is_char_lower(t->str_val[i]) == 0) {
			return 0;
		}
	}
	return 1;
}


int is_upper(text *t) {
	for (int i = 0; i < t->str_len; ++i) {
		if (is_char_upper(t->str_val[i]) == 0) {
			return 0;
		}
	}
	return 1;
}


int is_num(text *t) {
	for (int i = 0; i < t->str_len; ++i) {
		if (is_char_num(t->str_val[i]) == 0) {
			return 0;
		}
	}
	return 1;
}

int is_just_spaces(text *t) {
	for (int i = 0; i < t->str_len; ++i) {
		if (t->str_val[i] != ' ') {
			return -1;
		}
	}
	return 1;
}


int is_alpha(text *t) {
	for (int i = 0; i < t->str_len; ++i) {
		if (is_char_alpha(t->str_val[i]) == 0) {
			return 0;
		}
	}
	return 1;
}


void lower(text *t) {
	for (int i = 0; i < t->str_len; ++i) {
		if (is_char_upper(t->str_val[i])) {
			t->str_val[i] = lower_of_char(t->str_val[i]);
		}
	}
}


void upper(text *t) {
	for (int i = 0; i < t->str_len; ++i) {
		if (is_char_lower(t->str_val[i])) {
			t->str_val[i] = upper_of_char(t->str_val[i]);
		}
	}
}


int intx(text *t) {
	if (is_num(t) == 0) {
		return -1;
	}

	int int_val = 0;
	for (int i = 0; i < t->str_len; ++i) {
		int_val *= 10;
		int_val += int_of_char(t->str_val[i]);
	}
	return int_val;
}


int is_empty(char c) {
	if (c == ' ' || c == '\n' || c == '\t' || c == '\0') {
		return 1;
	}
	return 0;
}


char *slice_str(int new_len, int start, int end, int gap, const char *old_str) {
	char *new_str = malloc(new_len * sizeof(char));
	for (int i = start, j = 0; i < end; i = i + gap, ++j) {
		new_str[j] = old_str[i];
	}
	return new_str;
}

char *str_copy(char *str, int str_len) {
	char *new_str = alloc_str(str_len + 1);
	for (int i = 0; i < str_len; ++i) {
		new_str[i] = str[i];
	}
	new_str[str_len] = '\0';
	return new_str;
}

text *copy(text *t) {
	text *new_t = alloc_text();
	new_t->str_len = t->str_len;
	new_t->capacity = t->capacity;
	new_t->str_val = str_copy(t->str_val, t->str_len);
	return new_t;
}


text *slice(int start, int end, int gap, text *t) {
	if (start > t->str_len) {
		printf("[SliceError] : Starting index invalid\n");
		return NULL;
	}

	if (end > t->str_len) {
		end = t->str_len;
	}

	// If len is 7
	// negative index N -> positive index P
	// N can be converted to P
	// by using p = N + len
	// -1 -> 6
	// -2 -> 5

	if (start < 0) {
		start += t->str_len;
	}

	if (end < 0) {
		end += t->str_len;
	}

	// End index not included in string
	// 2, 5, 3
	// Len is ceil[(end-start)/gap]
	float nr = 1.0 * (end - start);
	float dr = 1.0 * (gap);
	int new_len = my_ceil(nr / dr);
	if (new_len == t->str_len) {
		return NULL;
	}

	char *new_str = slice_str(new_len, start, end, gap, t->str_val);
	text *new_t = alloc_text();
	new_t->str_val = new_str;
	new_t->capacity = new_len;
	new_t->str_len = new_len;
	return new_t;

}


void strip(text *t) {
	int front_spaces = 0;
	for (int i = 0, empty_till_now = 1; i < t->str_len, empty_till_now == 1; ++i, ++front_spaces) {
		if (is_empty(t->str_val[i]) == 0) {
			empty_till_now = 0;
			--front_spaces;
		}
	}

	slice(front_spaces, t->str_len, 1, t);

	int leading_spaces = 0;
	for (int i = t->str_len, empty_till_now = 1; i >= 0, empty_till_now == 1; --i, ++leading_spaces) {
		if (is_empty(t->str_val[i]) == 0) {
			empty_till_now = 0;
			--leading_spaces;
		}
	}

	slice(0, t->str_len - leading_spaces + 1, 1, t);
}


void reverse_str(int len, char *str) {
	char tmp;
	// if len is 5
	// replace 5-1 with 0
	// replace 5-2 with 1
	for (int i = 0; i < (len / 2); ++i) {
		tmp = str[i];
		str[i] = str[len - (i + 1)];
		str[len - (i + 1)] = tmp;
	}
}


void reverse(text *t) {
	reverse_str(t->str_len, t->str_val);
}


void text_input(text *t) {
	int capacity = 20;
	int len = 0;
	char *str_val = alloc_str(capacity);
	for (int i = 0, input_char = getchar(); input_char != DEFAULT_DELIMITER; ++len, ++i, input_char = getchar()) {
		if (i >= capacity - 1) {
			char *tmp_old_str = str_val;
			str_val = realloc_str(capacity, str_val);
			free(tmp_old_str);
			capacity *= 2;
		}
		str_val[i] = input_char;
	}
	t->str_val = str_val;
	t->str_len = len;
	t->str_val[len] = '\0';
	t->capacity = capacity;
}

int str_equal(char *str_1, char *str_2, int str_len) {
	for (int i = 0; i < str_len; ++i) {
		if (str_1[i] != str_2[i]) {
			return -1;
		}
	}
	return 0;
}

// Returns 0 for true and -1 for false
int equals(text *t1, text *t2) {
	if (t1->str_len != t2->str_len) {
		return -1;
	}
	return str_equal(t1->str_val, t2->str_val, t1->str_len);
}

text *create_text_from_params(char *str, char delimiter) {
	text *t = alloc_text();
	int len = 0;
	for (;; ++len) {
		if (str[len] == delimiter) {
			break;
		}
	}

	t->str_len = len;
	t->str_val = str_copy(str, len);
	return t;
}

int count(char c, text *t) {
	int result = 0;
	for (int i = 0; i < t->str_len; ++i) {
		if (t->str_val[i] == c) {
			++result;
		}
	}
	return result;
}

//text **split(text *t) {
//	int spaces = count(' ', t);
//	text *arr[spaces + 1];
//
//	return arr;
//}

void free_text(text *t) {
	free(t->str_val);
	free(t);
}

void free_text_arr(text **t, int len) {
	for (int i = 0; i < len; ++i) {
		free_text(t[i]);
	}
}


text *inputx() {
	text *t = alloc_text();
	text_input(t);
	return t;
}


//int main() {
//	printf("Enter text\n");
//	text *t = inputx();
//	printx(t);
//	vacuum(t);
//	reverse(t);
//	printx(t);
//	printf("-------\n");
//}
