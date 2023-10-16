//
// Created by arj on 16/10/23.
//

#include "filelocks.h"

void open_file_for_writing(int *fd){
    if ((*fd = open(FILE_PATH, O_CREAT | O_WRONLY | O_APPEND, 0666)) == -1) {
        perror("Error in opening the file for writing!\n");
        exit(1);
    }
}

void lock_file_for_writing(struct flock *lock, int *fd){
    lock->l_type = F_WRLCK;
    lock->l_whence = SEEK_SET;
    lock->l_start = 0;
    lock->l_len = 0;
    fcntl(*fd, F_SETLKW, lock);
}

void unlock_file_for_writing(struct flock *lock, int *fd){
    lock->l_type = F_UNLCK;
    fcntl(*fd, F_SETLKW, lock);
}

void clear_file(int *fd){
	printf("Clearing File!\n");
    ftruncate(*fd, 0);
    lseek(*fd, 0, SEEK_SET);
}

void open_file_for_reading(int *fd){
    if ((*fd = open(FILE_PATH, O_RDONLY)) == -1) {
        perror("Error in opening the file for reading!\n");
        exit(1);
    }
}

void lock_file_for_reading(struct flock *lock, int *fd){
    lock->l_type = F_RDLCK;
    lock->l_whence = SEEK_SET;
    lock->l_start = 0;
    lock->l_len = 0;
    fcntl(*fd, F_SETLKW, &lock);
}

void unlock_file_for_reading(struct flock *lock, int *fd){
    lock->l_type = F_UNLCK;
    fcntl(*fd, F_SETLKW, lock);
}
