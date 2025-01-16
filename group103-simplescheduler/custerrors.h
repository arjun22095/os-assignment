//
// Created by arj on 16/10/23.
//

#ifndef OS_AS3_CUSTERRORS_H
#define OS_AS3_CUSTERRORS_H

void handle_fork(pid_t fork_status);
void handle_seek(long int seek_response);
void handle_open(int open_response);
void handle_read(ssize_t read_response);
void handle_dup(int dup_response);
void handle_exec(int exec_response);
void handle_pipe(int pipe_response);
void my_dup(int fd1, int fd2);
void exec_error();

#endif //OS_AS3_CUSTERRORS_H
