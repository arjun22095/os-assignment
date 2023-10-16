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
#include <time.h>


process_queue *SQ;
process_queue *UQ;

int EXIT_CALLED;
int SCHED_DEBUG = 1;
#define dprint      \
    if (SCHED_DEBUG == 1) \
    printf


void set_status_as_terminated(pid_t pid) {
	printf("FUNCTION TO SET STATUS WAS CALLED and pid is %d!\n", pid);
	process_node *cur_node = UQ->head;
	for (int i = 0; i < UQ->len; ++i) {
		if (cur_node->proc->pid == pid) {
			dprint("SET STATUS AS 1\n");
			cur_node->proc->status = 1;
			cur_node->proc->end_time_t = clock();
			return;
		}
		cur_node = cur_node->next;
	}
}


void signalHandler(int signal) {
	if (signal == SIGCHLD) {
		int status;
		pid_t terminated_pid = waitpid(-1, &status, WNOHANG); // Check for any terminated child process.
		if (terminated_pid > 0) {
			printf("Received SIGCHLD from child process with PID %d.\n", terminated_pid);
			set_status_as_terminated(terminated_pid);
		}
	}
}


void handle_submitted_procs(int *fd) {
	if (EXIT_CALLED == 1){
		dprint("Exit was called so not handling anymore procs\n");
		return;
	}
	// Lock the file for reading
	open_file_for_reading(fd);

	struct flock lock;
	lock_file_for_reading(&lock, fd);

	dprint("Reading the shared file!\n");
	// Read and display the file content
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));
	ssize_t bytes_read = read(*fd, buffer, sizeof(buffer));
	dprint("Bytes read are %ld\n", bytes_read);

	if (bytes_read > 0) {
		buffer[bytes_read] = '\0';

		const char *new_line_delim = "\n";
		int arr_len = get_char_count(buffer, *new_line_delim) + 2; // Assumed
		char *out_names[arr_len];
		out_names[0] = strtok(buffer, new_line_delim);
		dprint("Currently outnames[0] is %s\n", out_names[0]);
		for (int j = 1; j < arr_len; ++j) {
			out_names[j] = strtok(NULL, new_line_delim);
			if (out_names[j] == NULL) {
				arr_len = j;
				break;
			}
		}

//		for (int i = 0; i < arr_len; ++i) {
//			int exit_called = 1;
//			if (out_names[i][0] == 'e') {
//				char exit_str[4] = "exit";
//				for (int j = 0; j < 4; ++j) {
//					if (out_names[i][j] != exit_str[j]) {
//						exit_called = 0;
//						break;
//					}
//				}
//			}
//
//			if (exit_called == 1) {
//				printf("EXIT WAS CALLED!\n");
//				EXIT_CALLED = 1;
//				continue;
//			}
//		}

		for (int i = 0; i < arr_len; ++i) {

			// adding from here
			int exit_called = 0;
			dprint("outnames[i] is %s\n", out_names[i]);
			if (out_names[i][0] == 'e') {
				exit_called = 1;
				char exit_str[4] = "exit";
				for (int j = 0; j < 4; ++j) {
					if (out_names[i][j] != exit_str[j]) {
						exit_called = 0;
						break;
					}
				}
			}

			if (exit_called == 1){
				dprint("EXIT WAS CALLED!\n");
				EXIT_CALLED = 1;
				break;
			}
			// to here


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

int check_and_reap_if_proc_terminated(pid_t proc_pid) {
	int wait_status;
	waitpid(proc_pid, &wait_status, WNOHANG);
	if (WIFEXITED(wait_status)) {
		dprint("Reaping process with pid %d\n", proc_pid);
		dprint("Exit status was %d\n", WEXITSTATUS(wait_status));
		return 1;
	}
	return 0;
}


void start_scheduling(process_queue *sq, int ncpu, int tslice, int *fd) {
	if (sq->len == 0) {
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
		for (i = 0; i < ncpu; ++i) {
			if (sq->len <= 0) {
				break;
			}

			cur_proc_node = dequeue(sq);
			procs_started[i] = cur_proc_node;
			cur_proc = cur_proc_node->proc;
			cur_proc->exec_time += tslice;
			++cur_proc->ran_continuously;
			kill(cur_proc->pid, SIGCONT);
		}

//		sleep(tslice);

		clock_t start_time = clock();
		double duration_ms = tslice;  // 300 milliseconds

		while (((double)(clock() - start_time) / CLOCKS_PER_SEC) * 1000.0 < duration_ms) {
			signal(SIGCHLD, signalHandler);
		}


		int cur_batch_size = i;
		if (i == 0) {
			return;
		}
		dprint("Current batch size is %d\n", cur_batch_size);
		for (int j = 0; j < cur_batch_size; ++j) {
			cur_proc_node = procs_started[j];
			cur_proc = cur_proc_node->proc;

			if (cur_proc->status == 1) {
				continue;
			}

			kill(cur_proc->pid, SIGSTOP);
			if (cur_proc->ran_continuously < cur_proc->priority) {
				dprint("Enqueuing in front%s\n", cur_proc->name->str_val);
				enqueue_in_front(cur_proc_node, sq);
			} else {
				cur_proc->ran_continuously = 0;
				if (cur_proc == original_rear) {
					rear_reached = 1;
				}
				dprint("Enqueuing %s\n", cur_proc->name->str_val);
				enqueue(cur_proc_node, sq);
			}

		}
	}
}

void sched_cleanup(){
	printf("Scheduler Queue is: \n");
	print_queue(SQ);
	printf("\nUniversal Queue is: \n");
	print_queue(UQ);
	free_queue(SQ);
	free_queue(UQ);
}


int main(int argc, char *argv[]) {
	EXIT_CALLED = 0;
	int fd;
	if (argc < 3) {
		perror("Insufficient Args for Scheduler!\n");
	}


	int ncpu = strtol(argv[1], NULL, 10);
	int tslice = strtol(argv[2], NULL, 10);

//	ncpu = 4;
//	tslice = 4;
	dprint("Scheduler processes started\n");
	// Create a process queue
	// Take user inputs for processes
	// Create structs for each of them and add them to the queue
	// Take user input for ncpu
	// Pick first ncpu processes from the queue
	// Run all of them for the input timeslice

	SQ = alloc_process_queue();
	UQ = alloc_process_queue();

	dprint("NCPU: %d\n", ncpu);
	dprint("TSLICE: %d\n", tslice);


//	add_process("./b.out", 1, SQ, UQ);
//	add_process("./c.out", 1, SQ, UQ);
//	add_process("./d.out", 1, SQ, UQ);
//	add_process("./e.out", 1, SQ, UQ);

	int temp = 0;
	while (1) {
		dprint("temp time counter: %d\n", temp++);
		handle_submitted_procs(&fd);
		dprint("EXIT VALUE IS %d\n", EXIT_CALLED);
		if (EXIT_CALLED == 1 && SQ->len == 0){
			break;
		}
		while (SQ->len > 0){
			start_scheduling(SQ, ncpu, tslice, &fd);
			dprint("\nUNIVERSAL QUEUE IS: \n");
			print_queue(UQ);
		}
		if (EXIT_CALLED == 1){
			break;
		}
		usleep(tslice * 1000);
	}

	sched_cleanup();
	dprint("Cleanup done...\nExiting scheduler!\n");
}
