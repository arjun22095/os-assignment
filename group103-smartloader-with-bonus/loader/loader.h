/*
 * No changes are allowed to this file
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <elf.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <ucontext.h>

#define dprint      \
    if (DEBUG == 1) \
    printf

#define PAGE_SIZE 4096

void load_and_run_elf(char **exe);
void loader_cleanup();
