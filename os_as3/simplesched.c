//
// Created by arj on 16/10/23.
//
#include "subsched.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "filelocks.h"
#include "texts.h"

process_queue *SQ;
process_queue *UQ;

int SCHED_DEBUG = 1;
#define dprint      \
    if (SCHED_DEBUG == 1) \
    printf


void handle_submitted_procs(int *fd) {
	// Lock the file for reading
	open_file_for_reading(fd);

	struct flock lock;
	lock_file_for_reading(&lock, fd);

	dprint("i am reading!\n");
	// Read and display the file content
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));
	ssize_t bytes_read = read(*fd, buffer, sizeof(buffer));
	dprint("bytes read are %ld\n", bytes_read);

	if (bytes_read > 0) {
		buffer[bytes_read] = '\0';

		const char *new_line_delim = "\n";
		int arr_len = get_char_count(buffer, *new_line_delim) + 2; // Assumed
		char *out_names[arr_len];
		out_names[0] = strtok(buffer, new_line_delim);
		dprint("currently outnames 0 is %s\n", out_names[0]);
		for (int j = 1; j < arr_len; ++j) {
			out_names[j] = strtok(NULL, new_line_delim);
			if (out_names[j] == NULL) {
				arr_len = j;
				break;
			}
		}

		for (int i = 0; i < arr_len; ++i) {
			if (((i + 1) < arr_len) && is_char_num(out_names[i + 1][0])) {
				int priority = strtol(out_names[i + 1], NULL, 10);
				add_process(out_names[i], priority, SQ, UQ);
				++i;
				continue;
			}
			add_process(out_names[i], 1, SQ, UQ);
		}
	}

	kill(getppid(), SIGSTOP);

	// Release the lock and close the file
	unlock_file_for_reading(&lock, fd);
	close(*fd);

	// now clearing the file after it has been read
	open_file_for_writing(fd);

	lock_file_for_writing(&lock, fd);

	// clear the file
	clear_file(fd);
	// Release the lock
	unlock_file_for_reading(&lock, fd);

	// resume producer
	close(*fd);
	kill(getppid(), SIGCONT);
}

int check_and_reap_if_proc_terminated(pid_t proc_pid){
	int wait_status;
	waitpid(proc_pid, &wait_status, WNOHANG);
	if (WIFEXITED(wait_status)){
		dprint("Reaping process with pid %d\n", proc_pid);
		dprint("Exit status was %d\n", WEXITSTATUS(wait_status));
		return 1;
	}
	return 0;
}


void start_scheduling(process_queue *sq, int ncpu, int tslice, int *fd) {
	if (sq->len == 0){
		dprint("SCHEDULER QUEUE IS EMPTY\n");
		sleep(tslice);
		return;
	}

	int rear_reached = 0;

	process *original_rear = sq->rear->proc;
	process_node *cur_proc_node;
	process *cur_proc;
	process_node *procs_started[ncpu];
	int batch_size = 0;

	while (rear_reached != 1) {
		int i;
		for (i = 0; i < ncpu; ++i){
			if (sq->len <= 0){
				break;
			}

			cur_proc_node = dequeue(sq);
			procs_started[i] = cur_proc_node;
			cur_proc = cur_proc_node->proc;
			cur_proc->exec_time += tslice;
			++cur_proc->ran_continuously;
			kill(cur_proc->pid, SIGCONT);
		}

		sleep(tslice);
		int cur_batch_size = i;
		if (i == 0){
			return;
		}
		dprint("Current batch size is %d\n", cur_batch_size);
		for (int j = 0; j < cur_batch_size; ++j){
			cur_proc_node = procs_started[j];
			cur_proc = cur_proc_node->proc;

			if (check_and_reap_if_proc_terminated(cur_proc->pid) == 1){
				printf("value of j is %d and pid is %d\n", j, cur_proc->pid);
				// free_proc
				cur_proc->status = 1;
				continue;
			}

			kill(cur_proc->pid, SIGSTOP);
			if (cur_proc->ran_continuously < cur_proc->priority){
				dprint("Enqueuing in front%s\n", cur_proc->name);
				enqueue_in_front(cur_proc_node, sq);
			} else {
				cur_proc->ran_continuously = 0;
				if (cur_proc == original_rear) {
					rear_reached = 1;
				}
				dprint("Enqueuing %s\n", cur_proc->name);
				enqueue(cur_proc_node, sq);
			}

		}
	}
}


int main(int argc, char *argv[]) {
	int fd;
	if (argc < 3) {
		perror("insufficient args for scheduler!\n");
	}

	int ncpu = strtol(argv[1], NULL, 10);
	int tslice = strtol(argv[2], NULL, 10);
	printf("Scheduler processes started\n");
	// Create a process queue
	// Take user inputs for processes
	// Create structs for each of them and add them to the queue
	// Take user input for ncpu
	// Pick first ncpu processes from the queue
	// Run all of them for the input timeslice

	SQ = alloc_process_queue();
	UQ = alloc_process_queue();

	printf("NCPU: %d\n", ncpu);
	printf("TSLICE: %d\n", tslice);


	add_process("./b.out", 1, SQ, UQ);
	add_process("./c.out", 1, SQ, UQ);
	add_process("./d.out", 1, SQ, UQ);
	add_process("./e.out", 1, SQ, UQ);

	for (int i = 0; i < 8; ++i) {
		handle_submitted_procs(&fd);
		printf("\nSCHED QUEUE IS: \n");
		start_scheduling(SQ, ncpu, tslice, &fd);
		printf("\nUNIVERSAL QUEUE IS: \n");
		print_queue(UQ);
	}

	printf("\nSCHED QUEUE IS: \n");
	print_queue(SQ);
	printf("\nUNIVERSAL QUEUE IS: \n");
	print_queue(UQ);
}
