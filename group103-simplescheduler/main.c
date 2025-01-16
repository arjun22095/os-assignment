#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "texts.h"
#include <string.h>
#include <time.h>
#include "history.h"
#include <fcntl.h>
#include "custerrors.h"
#include "filelocks.h"
#include <signal.h>
#include <math.h>

int ORIGINAL_STDIN;
int ORIGINAL_STDOUT;
struct text_ll *HISTORY;
const char *USER = "@usr";
const char *SHELL_NAME = "ash";

int fd;
pid_t sched_pid;
int SCHED_WORKING = 0;

int MAIN_DEBUG = 0;
#define dprint      \
    if (MAIN_DEBUG == 1) \
    printf

void start_scheduler(char *ncpu, char *tslice){
//	setpgid(0, 0);
	int fork_status = fork();
	if (fork_status == 0){
		sched_pid = execl("./ss", "./ss", ncpu, tslice, (char*)NULL);
	} else {
		sched_pid = fork_status;
	}
}

void save_stdio() {
	dprint("[TASK] Saving STDIN and STDOUT\n");
	ORIGINAL_STDIN = dup(0);
	ORIGINAL_STDOUT = dup(1);
	dprint("[SUCCESS] Saved STDIN and STDOUT\n");
}

void restore_stdio() {
	dprint("[TASK] Restoring original STDIN and STDOUT\n");
	dup2(ORIGINAL_STDIN, 0);
	dup2(ORIGINAL_STDOUT, 1);
	dprint("[SUCCESS] Restored original STDIN and STDOUT\n");
}

pid_t execute_command(char *command) {
	const char *pipe_delim = "|";
	const char *space_delim = " ";
	pid_t wait_result;

	// Get '|' symbol count in the command
	int pipe_count = get_char_count(command, *pipe_delim);


	// If count(|) == 0 => No piping required
	if (pipe_count == 0) {
		// Initial exec_argc is going to be > actual argc
		// + 2 is important to add at least one NULL at the end of the exec_argv
		int exec_argc = get_char_count(command, *space_delim) + 2; // Assumed
		char *exec_argv[exec_argc];
		exec_argv[0] = strtok(command, space_delim);
		for (int j = 1; j < exec_argc; ++j) {
			exec_argv[j] = strtok(NULL, space_delim);
			if (exec_argv[j] == NULL) {
				// Assigning true argc, not resizing the array since that's not possible
				// The array exec_argv is anyway NULL terminated, that is what execvp requires
				exec_argc = j;
				break;
			}
		}

		// Create a fork that will exec the command
		pid_t fork_status = fork();
		handle_fork(fork_status);

		if (fork_status == 0) {
			int exec_response = execvp(exec_argv[0], exec_argv);
			exec_error();
		}

		// Reap the process called and return its PID
		int status;
		wait_result = waitpid(-1, &status, 0);
		if (WIFEXITED(status)) {
			dprint("Reaped child process successfully!\n");
			return wait_result;
		} else {
			perror("[ERROR] Couldn't reap the child process");
			return -1;
		}
	}

	// Creating an array which is split by '|' delimiter
	int len_pipe_sep = pipe_count + 1; // This is assumed value
	char *pipe_sep[len_pipe_sep];

	pipe_sep[0] = strtok(command, pipe_delim);
	for (int i = 1; i < len_pipe_sep; ++i) {
		pipe_sep[i] = strtok(NULL, pipe_delim);
		if (pipe_sep[i] == NULL) {
			// Assigning true len, not resizing the array since that's not possible
			len_pipe_sep = i + 1;
			break;
		}
	}

	// Create a pipe_file_descriptor
	int pipe_fd[2];
	int old_pipe = STDIN_FILENO;
	pid_t first_proc_pid;

	for (int i = 0; i < len_pipe_sep; ++i) {
		// Generating the current exec's argv!
		// Assumed len will always be greater than actual
		int exec_argc = get_char_count(pipe_sep[i], *space_delim) + 1; // Assumed
		char *exec_argv[exec_argc];
		exec_argv[0] = strtok(pipe_sep[i], space_delim);
		for (int j = 1; j < exec_argc; ++j) {
			exec_argv[j] = strtok(NULL, space_delim);
			if (exec_argv[j] == NULL) {
				// Assigning true len, not resizing the array since that's not possible
				exec_argc = j; // The array exec_argv is anyway NULL terminated, that is what execvp requires
				break;
			}
		}
		// argv generated, now we can set up pipes and execute

		// Calling the pipe() function
		if (i != len_pipe_sep - 1) {
			int pipe_response = pipe(pipe_fd);
			handle_pipe(pipe_response);
		}

		// Forking to create a child that can execvp later
		pid_t fork_status = fork();
		handle_fork(fork_status);

		if (i == 0 & fork_status != 0) {
			first_proc_pid = fork_status;
		}

		if (fork_status == 0) {
			// Child process running
			if (old_pipe != STDIN_FILENO) {
				// my_dup wrapper handling all necessary close and error handling
				my_dup(old_pipe, STDIN_FILENO);
			}

			if (i != len_pipe_sep - 1) {
				my_dup(pipe_fd[1], STDOUT_FILENO);
				execvp(exec_argv[0], exec_argv);
				// wrapper to throw error if fork reaches past exec statement
				exec_error();
			}

			execvp(exec_argv[0], exec_argv);
			exec_error();
		}

		if (i != len_pipe_sep - 1) {
			close(old_pipe);
			close(pipe_fd[1]);
			old_pipe = pipe_fd[0];
		}
	}


	// Catch all processes
	int status;
	for (int i = 0; i < len_pipe_sep; ++i) {
		waitpid(-1, &status, 0);
		if (WIFEXITED(status)) {
			dprint("Reaped child process successfully!\n");
		} else {
			perror("[ERROR] Couldn't reap the child process");
			return -1;
		}
	}

	close(pipe_fd[0]);
	close(pipe_fd[1]);
	return first_proc_pid;
}


int clean_exec(char *command, int author) {
	text *node_text = create_text_from_params(command, '\0');
	struct ll_node *cur_node = alloc_node();
	clock_t start = clock();
	set_call_time(cur_node);
	pid_t proc_pid = execute_command(command);
	clock_t end = clock();
	double duration = (double) (end - start) / CLOCKS_PER_SEC;
	cur_node->t_val = node_text;
	cur_node->pid = proc_pid;
	cur_node->duration = duration;
	cur_node->start_time = start;
	cur_node->author = author;
	add_to_history(HISTORY, cur_node);
	return 0;
}

pid_t create_bg_process(char *command) {
	int fork_status = fork();
	if (fork_status == 0) { // inside the fork
		int subfork_status = fork();
		if (subfork_status == 0) {
			printf("[PID of the background process] : %d\n", getpid());
			clean_exec(command, 0);
		} else if (subfork_status < 0) {
			perror("[ERROR] In sub-fork\n");
		} else {
			// abandons the sub-fork
			// so it has no parent and acts independently
			exit(0);
		}
	} else if (fork_status < 0) {
		perror("fork error\n");
	} else {
		return fork_status;
	}
	return 0;
}

// Check and tell if the string is equal to the SHELL_NAME
int str_eq_shell_name(char *str) {
	for (int i = 0;; ++i) {
		if (str[i] == '\0' && SHELL_NAME[i] == '\0') {
			return 1;
		}
		if (str[i] != SHELL_NAME[i]) {
			return -1;
		}
	}
}

// returns 1 if str is equal to "exit"
int str_eq_exit(char *str) {
	const char *exit_str = "exit";
	for (int i = 0;; ++i) {
		if (str[i] == '\0' && exit_str[i] == '\0') {
			return 1;
		}
		if (str[i] != exit_str[i]) {
			return -1;
		}
	}
}

int str_eq_sched_signal(char *str) {
	const char *start_str = "startsched";
	for (int i = 0;; ++i) {
		if (str[i] == '\0' && start_str[i] == '\0') {
			return 1;
		}
		if (str[i] != start_str[i]) {
			return -1;
		}
	}
}

int str_eq_submit(char *str) {
	const char *exit_str = "submit";
	for (int i = 0;; ++i) {
		if (str[i] == '\0' && exit_str[i] == '\0') {
			return 1;
		}
		if (str[i] != exit_str[i]) {
			return -1;
		}
	}
}

// --- *BONUS*
// returns length of the current line in the file
int get_line_size(int fd, int start) {
	int bytes = 0;
	ssize_t read_response;
	for (char c;; ++bytes) {
		read_response = read(fd, &c, 1);
		if (read_response == 0 || c == '\n') {
			return bytes;
		}
	}
}

// checks if file ends with .sh or not
int filename_ends_with_sh(char *filename) {
	int i;
	for (i = 0; filename[i] != '\0'; ++i);
	if (i < 3) {
		return -1;
	}

	if (filename[i - 1] == 'h' && filename[i - 2] == 's' && filename[i - 3] == '.') {
		return 1;
	}

	return -1;
}

// checks if the command ends with '&' symbol or not
int command_ends_with_ampersand(char *command) {
	int i;
	for (i = 0; command[i] != '\0'; ++i);
	if (i < 1) {
		return -1;
	}

	if (command[i - 1] == '&') {
		command[i - 1] = '\0';
		return 1;
	}

	return -1;
}

// take sh file name as input and execute it by calling clean_exec
int execute_sh_file(char *filename) {
	if (filename_ends_with_sh(filename) == -1) {
		return -1;
	}

	int fd = open(filename, O_RDONLY);
	if (fd == -1) {
		perror("No such .sh file exists!\n");
		return -1;
	}

	int start = 0;
	int line_size;
	ssize_t read_response;
	long int seek_response;

	for (int j = 0;; ++j) {
		line_size = get_line_size(fd, start);
		char line[line_size + 1];

		seek_response = lseek(fd, start, SEEK_SET);
		handle_seek(seek_response);

		read_response = read(fd, &line, line_size + 1);
		handle_read(read_response);

		// Whole file has been read
		if (read_response == 0) {
			break;
		}

		line[line_size] = '\0'; // replacing \n with \0
		clean_exec(line, 1);
		restore_stdio();
		start += line_size + 1;
	}

	close(fd);
	return 1;
}

// tells if the command is to call the .sh file or not, if yes, it also calls the .sh file
int is_sh_file_called(text *t) {
	text *tc = copy(t);
	char *command = tc->str_val;
	const char *space_delim = " ";

	int exec_argc = get_char_count(command, *space_delim) + 2; // Assumed
	char *exec_argv[exec_argc];
	exec_argv[0] = strtok(command, space_delim);
	for (int j = 1; j < exec_argc; ++j) {
		exec_argv[j] = strtok(NULL, space_delim);
		if (exec_argv[j] == NULL) {
			exec_argc = j;
			break;
		}
	}

	if (exec_argc != 2) {
		free_text(tc);
		return -1;
	}

	if (str_eq_shell_name(exec_argv[0]) == 1) {
		if (filename_ends_with_sh(exec_argv[1]) == 1) {
			text *node_text = create_text_from_params(t->str_val, '\0');
			struct ll_node *cur_node = alloc_node();
			clock_t start = clock();
			set_call_time(cur_node);
			pid_t proc_pid = getpid();
			execute_sh_file(exec_argv[1]);
			clock_t end = clock();
			double duration = (double) (end - start) / CLOCKS_PER_SEC;
			cur_node->t_val = node_text;
			cur_node->pid = proc_pid;
			cur_node->duration = duration;
			cur_node->start_time = start;
			cur_node->author = 0;
			add_to_history(HISTORY, cur_node);
			free_text(tc);
			return 1;
		}
		free_text(tc);
		return -1;
	}

	free_text(tc);
	return -1;
}

int is_bg_proc_command(text *t) {
	text *tcc = copy(t);
	text *tc = copy(t);
	char *command = tc->str_val;
	const char *space_delim = " ";

	int exec_argc = get_char_count(command, *space_delim) + 2; // Assumed
	char *exec_argv[exec_argc];
	exec_argv[0] = strtok(command, space_delim);
	for (int j = 1; j < exec_argc; ++j) {
		exec_argv[j] = strtok(NULL, space_delim);
		if (exec_argv[j] == NULL) {
			exec_argc = j;
			break;
		}
	}

	if (exec_argc == 0) {
		free_text(tc);
		return -1;
	}

	if (command_ends_with_ampersand(exec_argv[exec_argc - 1]) == 1) {
		text *node_text = create_text_from_params(t->str_val, '\0');
		for (int i = tcc->str_len; i >= 0; --i) {
			if (tcc->str_val[i] == '&') {
				tcc->str_val[i] = '\0';
				break;
			}
		}

		struct ll_node *cur_node = alloc_node();
		clock_t start = clock();
		set_call_time(cur_node);
		pid_t proc_pid = create_bg_process(tcc->str_val);
		clock_t end = clock();
		double duration = (double) (end - start) / CLOCKS_PER_SEC;
		cur_node->t_val = node_text;
		cur_node->pid = proc_pid;
		cur_node->duration = duration;
		cur_node->start_time = start;
		cur_node->author = 0;
		add_to_history(HISTORY, cur_node);
		free_text(tc);
		free_text(tcc);
		return 1;
	}

	free_text(tc);
	return -1;
}

// --- *BONUS*

int was_exit_called(text *t) {
	text *tc = copy(t);
	char *command = tc->str_val;
	const char *space_delim = " ";

	int exec_argc = get_char_count(command, *space_delim) + 2; // Assumed
	char *exec_argv[exec_argc];
	exec_argv[0] = strtok(command, space_delim);
	for (int j = 1; j < exec_argc; ++j) {
		exec_argv[j] = strtok(NULL, space_delim);
		if (exec_argv[j] == NULL) {
			exec_argc = j;
			break;
		}
	}

	if (exec_argc != 1) {
		free_text(tc);
		return -1;
	}

	char *exit_str = "exit";
	if (str_eq_exit(exec_argv[0]) == 1) {
		free_text(tc);
		return 1;
	}

	free_text(tc);
	return -1;
}

void submit_procs_to_sched(char *array[], int array_len){
	struct flock lock;
	lock_file_for_writing(&lock, &fd);

	// Write to the file
	for (int i = 1; i < array_len; ++i){
		printf("writing to file! : %s\n", array[i]);
		write(fd, array[i], strlen(array[i]));
		write(fd, "\n", 1);
	}

	// Release the lock
	unlock_file_for_writing(&lock, &fd);
}

int was_submit_called(text *t) {
	text *tc = copy(t);
	char *command = tc->str_val;
	const char *space_delim = " ";

	int exec_argc = get_char_count(command, *space_delim) + 2; // Assumed
	char *exec_argv[exec_argc];
	exec_argv[0] = strtok(command, space_delim);
	for (int j = 1; j < exec_argc; ++j) {
		exec_argv[j] = strtok(NULL, space_delim);
		if (exec_argv[j] == NULL) {
			exec_argc = j;
			break;
		}
	}

	if (str_eq_submit(exec_argv[0]) == 1) {
		printf("submit was called!\n");
		submit_procs_to_sched(exec_argv, exec_argc);
		free_text(tc);
		return 1;
	}

	free_text(tc);
	return -1;
}

int was_scheduler_signalled(text *t) {
	text *tc = copy(t);
	char *command = tc->str_val;
	const char *space_delim = " ";

	int exec_argc = get_char_count(command, *space_delim) + 2; // Assumed
	char *exec_argv[exec_argc];
	exec_argv[0] = strtok(command, space_delim);
	for (int j = 1; j < exec_argc; ++j) {
		exec_argv[j] = strtok(NULL, space_delim);
		if (exec_argv[j] == NULL) {
			exec_argc = j;
			break;
		}
	}

	if (str_eq_sched_signal(exec_argv[0]) == 1) {
		printf("scheduler was signalled!\n");
		SCHED_WORKING = 1;
		start_scheduler(exec_argv[1], exec_argv[2]);
		free_text(tc);
		return 0;
	}

	free_text(tc);
	return -1;
}

text *get_command() {
	printf("\x1B[34m\n%s: \x1B[0m", USER);
	text *command = inputx();
	vacuum(command);
	return command;
}


int shell() {
	restore_stdio();
	text *command = get_command();
	if (is_just_spaces(command) == 1) {
		// can add to history but omitting right now
		free_text(command);
		return 0;
	}

	text *command_copy = copy(command);

	if (SCHED_WORKING == 0){
		int sched_signal_result = was_scheduler_signalled(command_copy);
		if (sched_signal_result == 0){
			free_text(command);
			return 0;
		}
	}

	int submit_call_result = was_submit_called(command_copy);
	if (submit_call_result == 1){
		free_text(command);
		return 0;
	}

	int exit_call_result = was_exit_called(command_copy);
	if (exit_call_result == 1) {
		printf("Exiting...\n");
		free_text(command_copy);
		free_text(command);
		restore_stdio();
		print_history(HISTORY);
		char *signal_cmd[] = {"submit", "exit"};
		submit_procs_to_sched(signal_cmd, 2);
		return 5;
	}

	int bg_proc_result = is_bg_proc_command(command_copy);
	int sh_call_result = is_sh_file_called(command_copy);

	if (sh_call_result == -1 && bg_proc_result == -1) {
		clean_exec(command->str_val, 0);
	}

	// Active print history is on
//	print_history(HISTORY); // Comment this line to disable that


	free_text(command_copy);
	free_text(command);
	return 0;
}

int INTERRUPTED = 0;

void on_interrupt() {
	printf("\nExit initiated\n");
	restore_stdio();
	print_history(HISTORY);
	printf("closing scheduler!\n");
//	kill(sched_pid, SIGKILL);
	char *command[] = {"submit", "exit"};
	submit_procs_to_sched(command, 2);
	int status;
	pid_t terminated_pid = waitpid(sched_pid, &status, WEXITED);
//    close(fd);
	INTERRUPTED = 1;
	exit(1);
}

int main() {
	ceil(3.0);
	open_file_for_writing(&fd);
	clear_file(&fd);
	HISTORY = init_history();
	save_stdio();
	signal(SIGINT, on_interrupt);
	while (shell() == 0);
    close(fd);
	return 0;
}