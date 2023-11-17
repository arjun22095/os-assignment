#include <iostream>
#include <list>
#include <functional>
#include <stdlib.h>
#include <cstring>
#include <chrono>


int DEBUG = 0;
#define dprint if (DEBUG == 1)printf

// Simple-Parallel-For : Args Struct
struct {
	int actual_start;
	int actual_end;
	int thread_no;
	int num_threads;
	int start;
	int end;
	std::function<void(int)> lambda;
} typedef args;


// Simple-Parallel-For : pthread Function
void *thread_func(void *arg) {
	args *casted_args = (args *) arg;
	int start = casted_args->start;
	int end = casted_args->end;
	if (casted_args->thread_no == casted_args->num_threads - 1) {
		end = casted_args->actual_end;
	}

	for (int i = start; i < end; ++i) {
		casted_args->lambda(i);
	}

	return nullptr;
}

// Simple-Parallel-For : main handler function
void parallel_for(int low, int high, std::function<void(int)> &&lambda, int numThreads) {
	dprint("Simple-Parallel-For Started\n");

	// Clocking the time
	auto start = std::chrono::high_resolution_clock::now();

	// Creating pthread_t array of numThread length
	pthread_t threads[numThreads];

	// Calculating chunk_size to process based on range of loop and number of threads
	int chunk_size = (high - low) / numThreads;
	dprint("[INFO] chunkSize is %d\n", chunk_size);
	dprint("[INFO] numThreads is %d\n", numThreads);

	// Using pthread create to start the multithreaded execution of the lambda function
	args cur_args[numThreads];
	for (int j = 0; j < numThreads; ++j) {
		cur_args[j] = {low, high, j, numThreads, (j * chunk_size), ((j + 1) * chunk_size), lambda};
		int create_result = pthread_create(&threads[j], nullptr, thread_func, &cur_args[j]);
		if (create_result != 0) {
			perror("[ERROR] pthread_create failed!\n");
			exit(1);
		}
	}

	// Using pthread_join on all the threads
	for (int j = 0; j < numThreads; ++j) {
		int join_result = pthread_join(threads[j], nullptr);
		if (join_result != 0) {
			perror("[ERROR] pthread_join failed!\n");
			exit(1);
		}
	}

	// Clocking the end time and calculating the execution time
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	printf("Simple-Parallel-For Started Completed! Time taken : %ld ms\n", duration.count());
}

// ------


// Nested-Parallel-For loop

// Nested-Parallel-For - Arg struct
struct {
	int outer_start;
	int outer_end;
	int inner_start;
	int inner_end;
	int thread_no;
	int num_threads;
	int chunk_size;
	std::function<void(int, int)> lambda;
} typedef args2;


// Nested-Parallel-For - pthread Function
void *nested_thread_func(void *arg) {
	args2 *casted_args = (args2 *) arg;
	int cur_pointer = (casted_args->thread_no * casted_args->chunk_size);
	dprint("Nested-Parallel-For thread[%d] function called\n", casted_args->thread_no);

	// Manual way of calculating cur_i and cur_j
	/*
	int cur_i = casted_args->outer_start;
	int cur_j = casted_args->inner_start;

	for (int i = 0; i < cur_pointer; ++i) {
		++cur_j;
		if (cur_j >= casted_args->inner_end) {
			cur_j = 0;
			++cur_i;
		}
		if (cur_i >= casted_args->outer_end) {
			break;
		}
	}
	*/

	// Mathematical way of calculating (more efficient)
	dprint("[ThreadFuncInfo : %d] cur pointer is %d\n", casted_args->thread_no, cur_pointer);
	int cur_j = (casted_args->inner_start + cur_pointer) % casted_args->inner_end;
	int cur_i = (casted_args->outer_start) + ((casted_args->inner_start + cur_pointer) / casted_args->inner_end);
	dprint("[ThreadFuncInfo : %d] cur_i is %d\tcur_j is %d\n", casted_args->thread_no, cur_i, cur_j);

	// If thread is the last thread (then chunk_size can be the dynamic, i.e. just run till the actual end of the whole loop)
	if (casted_args->thread_no == casted_args->num_threads - 1) {
		for (int i = 0; cur_i < casted_args->outer_end; ++i) {
			casted_args->lambda(cur_i, cur_j);
			++cur_j;
			if (cur_j >= casted_args->inner_end) {
				cur_j = 0;
				++cur_i;
			}
		}

		dprint("Nested-Parallel-For thread[%d] function completed!\n", casted_args->thread_no);
		return nullptr;
	}


	// If it is not the last thread
	for (int i = 0; i < casted_args->chunk_size && cur_i < casted_args->outer_end; ++i) {
		casted_args->lambda(cur_i, cur_j);
		++cur_j;
		if (cur_j >= casted_args->inner_end) {
			cur_j = 0;
			++cur_i;
		}
	}

	dprint("Nested-Parallel-For thread[%d] function completed!\n", casted_args->thread_no);
	return nullptr;
}

// Nested-Parallel-For - main handler function
void parallel_for(int low1, int high1, int low2, int high2, std::function<void(int, int)> &&lambda, int numThreads) {
	dprint("Nested-Parallel-For execution started\n");
	// Clocking the time
	auto start = std::chrono::high_resolution_clock::now();

	// Creating pthread_t array of numThread length
	pthread_t threads[numThreads];

	// Calculating chunk_size, based on outer loop range, inner loop range and number of threads
	int chunk_size = (high1 - low1) * (high2 - low2) / numThreads;
	args2 cur_args[numThreads];
	dprint("[INFO] chunkSize is %d\n", chunk_size);
	dprint("[INFO] numThreads are %d\n", numThreads);

	// Using pthread create to start the multithreaded execution of the lambda function
	for (int j = 0; j < numThreads; ++j) {
		dprint("Creating thread[%d]\n", j);
		cur_args[j] = {low1, high1, low2, high2, j, numThreads, chunk_size, lambda};
		int create_result = pthread_create(&threads[j], nullptr, nested_thread_func, &cur_args[j]);
		if (create_result != 0) {
			perror("[ERROR] pthread_create failed!\n");
			exit(1);
		}
	}

	// Using pthread_join on all the threads
	for (int j = 0; j < numThreads; ++j) {
		dprint("Joining threads[%d]\n", j);
		int join_result = pthread_join(threads[j], nullptr);
		if (join_result != 0) {
			perror("[ERROR] pthread_join failed!\n");
			exit(1);
		}
	}

	// Calculating the execution time of the loop
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	printf("Nested-Parallel-For Started Completed! Time taken : %ld ms\n", duration.count());
}

// Custom implemented functions over ---


int user_main(int argc, char **argv);

/* Demonstration on how to pass lambda as parameter.
 * "&&" means r-value reference. You may read about it online.
 */
void demonstration(std::function<void()> &&lambda) {
	lambda();
}

int main(int argc, char **argv) {
	/*
	 * Declaration of a sample C++ lambda function
	 * that captures variable 'x' by value and 'y'
	 * by reference. Global variables are by default
	 * captured by reference and are not to be supplied
	 * in the capture list. Only local variables must be
	 * explicity captured if they are used inside lambda.
	 */
	int x = 5, y = 1;
	// Declaring a lambda expression that accepts void type parameter
	auto /*name*/ lambda1 = /*capture list*/[/*by value*/ x, /*by reference*/ &y](void) {
		/* Any changes to 'x' will throw compilation error as x is captured by value */
		y = 5;
		std::cout << "====== Welcome to Assignment-" << y << " of the CSE231(A) ======\n";
		/* you can have any number of statements inside this lambda body */
	};
	// Executing the lambda function
	demonstration(lambda1); // the value of x is still 5, but the value of y is now 5

	int rc = user_main(argc, argv);

	auto /*name*/ lambda2 = [/*nothing captured*/]() {
		std::cout << "====== Hope you enjoyed CSE231(A) ======\n";
		/* you can have any number of statements inside this lambda body */
	};
	demonstration(lambda2);
	return rc;
}

#define main user_main
