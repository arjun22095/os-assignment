//
// Created by arj on 16/10/23.
//

#ifndef OS_AS3_FILELOCKS_H
#define OS_AS3_FILELOCKS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#define FILE_PATH "a.txt"

void open_file_for_writing(int *fd);

void lock_file_for_writing(struct flock *lock, int *fd);

void unlock_file_for_writing(struct flock *lock, int *fd);

void clear_file(int *fd);

void open_file_for_reading(int *fd);

void lock_file_for_reading(struct flock *lock, int *fd);

void unlock_file_for_reading(struct flock *lock, int *fd);

#endif //OS_AS3_FILELOCKS_H
