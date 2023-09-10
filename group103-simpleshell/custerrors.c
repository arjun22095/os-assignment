//
// Created by arj on 10/9/23.
//
#include <unistd.h>
#include "stdlib.h"
#include "stdio.h"
#include "custerrors.h"

void handle_fork(pid_t fork_status) {
	if (fork_status < 0) {
		perror("[ERROR] Fork failed!\n");
		exit(1);
	}
}

void handle_seek(long int seek_response) {
	if (seek_response == -1) {
		perror("[ERROR] Error while seeking\n");
		exit(1);
	}
}

void handle_open(int open_response) {
	if (open_response == -1) {
		perror("[ERROR] Error while opening\n");
		exit(1);
	}
}

void handle_read(ssize_t read_response) {
	if (read_response == -1) {
		perror("[ERROR] Error while reading\n");
		exit(1);
	}
}

void handle_dup(int dup_response) {
	if (dup_response == -1) {
		perror("[ERROR] Error while duping\n");
		exit(1);
	}
}

void handle_exec(int exec_response) {
	if (exec_response == -1) {
		perror("[ERROR] Error while executing\n");
		exit(1);
	}
}

void exec_error() {
	perror("[ERROR] Error while executing\n");
	exit(1);
}

void handle_pipe(int pipe_response) {
	if (pipe_response == -1) {
		perror("[ERROR] Error while piping\n");
		exit(1);
	}
}

void my_dup(int fd1, int fd2) {
	int dup_response = dup2(fd1, fd2);
	handle_dup(dup_response);
	close(fd1);
}