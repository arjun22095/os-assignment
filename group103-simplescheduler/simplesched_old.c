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

process_queue *PQ;

int SCHED_DEBUG = 0;
#define dprint      \
    if (SCHED_DEBUG == 1) \
    printf


void handle_submitted_procs(int *fd){
	// Lock the file for reading
	open_file_for_reading(fd);

	struct flock lock;
	lock_file_for_reading(&lock, fd);

	dprint("i am reading!\n");
	// Read and display the file content
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));
	ssize_t bytes_read = read(*fd, buffer, sizeof(buffer));
	dprint("bytes read are %d\n", bytes_read);

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

		for (int i = 0; i < arr_len; ++i){
			if (((i + 1) < arr_len) && is_char_num(out_names[i + 1][0])){
				int priority = strtol(out_names[i + 1], NULL, 10);
				add_process(out_names[i], priority, PQ);
				++i;
				continue;
			}
			add_process(out_names[i], 1, PQ);
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

void start_scheduling(process_queue *pq, int ncpu, int tslice, int *fd) {
	dprint("\n\n[STARTING SCHEDULER]\n");
	int org_pq_len = pq->len;


	dprint("[INFO] Process queue len is %d\n", org_pq_len);
	if (org_pq_len == 0){
		dprint("[EXITING SCHEDULER] Process queue len is empty.\n");
		sleep(tslice);
		return;
	}

	// number of batches will be ceil(pq->len/ncpu) here nbatch = 3
	int n_batch = ceil((pq->len * 1.0) / (ncpu * 1.0));
	dprint("[INFO] Number of batches : %d\n", n_batch);
	dprint("[INFO] ncpu : %d\n", ncpu);
	dprint("[INFO] tslice : %d\n\n", tslice);
	process_node *dequeued_proc_nodes[pq->len];
	for (int i = 0; i < n_batch; ++i) {
//		print_queue(pq);
		dprint("[INFO] Process queue len is %d\n", pq->len);
		// each batch will have at max ncpu procs to run
		dprint("[TASK] Starting batch %d\n", i);
		// Continue n procs
		int cur_batch_size = 0;
		dprint("ncpu %d and tslice %d\n", ncpu, tslice);
		for (int j = 0; j < ncpu; ++j) {
			if (pq->len == 0) {
				break;
			}
			dprint("value of j starter is %d\n", j);
			++cur_batch_size;
			dequeued_proc_nodes[(ncpu * i) + j] = dequeue(pq);
			process *cur_proc = dequeued_proc_nodes[(ncpu * i) + j]->proc;
			int cont_result = kill(cur_proc->pid, SIGCONT);
			dprint("[TASK] Continuing process with name %s\tResult is %d\n", cur_proc->name, cont_result);
		}

		dprint("[INFO] Current batch-size was: %d\n", cur_batch_size);
		dprint("[TASK] Sleeping for %d time\n", tslice);

		// Sleep for tslice time
		sleep(tslice);

		// check for new submitted tasks
		dprint("\n\n");

		dprint("\n[TASK] Woken up, going to stop all processes\n");
		// Stop all n procs
		for (int k = 0; k < cur_batch_size; ++k) {
			dprint("value of k stopper is %d\n", k);
			process *cur_proc = dequeued_proc_nodes[(ncpu * i) + k]->proc;
			dprint("stopping proc with name %s\n", cur_proc->name);
			int stop_status = kill(cur_proc->pid, SIGSTOP);
			dprint("[INFO] Stopping process with pid %d\tResult is %d\n", cur_proc->pid, stop_status);
			cur_proc->exec_time += tslice;
			++cur_proc->ran_continuously;

			if (stop_status != 0) {
				dprint("[INFO] Process with PID : %d has terminated!\n", cur_proc->pid);
				cur_proc->status = 1; //cur proc has terminated
				int wait_status;
				waitpid(cur_proc->pid, &wait_status, 0);
				if (!WIFEXITED(wait_status)) {
					perror("Process didn't terminate properly!\n");
				}
			}
		}

		for (int k = 0; k < cur_batch_size; ++k){
			process_node *cur_proc_node = dequeued_proc_nodes[(ncpu * i) + k];
			process *cur_proc = cur_proc_node->proc;
			dprint("cur proc in talk is %s\n", cur_proc->name);
			if (cur_proc->status == 0){
				int wait_status;
				pid_t temp = waitpid(cur_proc->pid, &wait_status, WNOHANG);
				dprint("exit status is %d\n", WEXITSTATUS(wait_status));
				if (WIFEXITED(wait_status)) { // proc has terminated
					dprint("this proc in talk %s has terminated!\n", cur_proc->name);
					cur_proc->status = 1;
				} else if (cur_proc->ran_continuously >= cur_proc->priority){
					dprint("enqueuing this proc in talk normally\n");
					cur_proc->ran_continuously = 0; //reset
					enqueue(cur_proc_node, pq);
				} else {
					dprint("enqueuing this proc in talk at the back\n");
					enqueue_in_front(cur_proc_node, pq);
				}
			}
		}
	}
}


int main(int argc, char *argv[]) {
	int fd;
	if (argc < 3){
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

	PQ = alloc_process_queue();

	printf("NCPU: %d\n", ncpu);
	printf("TSLICE: %d\n", tslice);

	add_process("./b.out", 3, PQ);
	add_process("./c.out", 3, PQ);
	add_process("./d.out", 1, PQ);


	while(1){
//		handle_submitted_procs(&fd);
		print_queue(PQ);
		start_scheduling(PQ, ncpu, tslice, &fd);
	}
}
