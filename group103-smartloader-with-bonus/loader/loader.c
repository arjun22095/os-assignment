#include "loader.h"

// Set DEBUG = 1, to get detailed step-wise output
int READ_TOGGLE = 0;
int DEBUG = 1;


volatile void *saved_pc;
Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;
uintptr_t start_loc;

int PAGE_FAULT_COUNT;
size_t TOTAL_BYTES_WRITTEN;
int PAGES_ALLOCED_COUNT;
size_t TOTAL_MEM_SZ;
size_t TOTAL_FILE_SZ;

struct page_node {
	void *addr;
	struct page_node *next;
};

struct page_node *HEAD;

void add_page_to_ll(void *addr) {
	struct page_node *to_add = (struct page_node *)malloc(sizeof(struct page_node));
	if (to_add == NULL) {
		dprint("[ERROR] Malloc failed!\n");
		exit(1);
	}

	to_add->next = NULL;
	to_add->addr = addr;

	if (HEAD == NULL) {
		HEAD = to_add;
	} else {
		struct page_node *cur = HEAD;
		while (cur->next != NULL) {
			cur = cur->next;
		}
		cur->next = to_add;
	}
}

void unmap_and_free_ll() {
	struct page_node *cur = HEAD;
	struct page_node *next = NULL;
	for (int i = 0; i < PAGES_ALLOCED_COUNT; ++i) {
		next = cur->next;
		dprint("[TASK] munmap on address %p\n", cur->addr);
		munmap(cur->addr, PAGE_SIZE);
		free(cur);
		cur = next;
	}
	dprint("[SUCCESS] unmapped all pages and freed page linked list\n");
}

void read_program_header() {
	if (READ_TOGGLE == 1) {
		dprint("[TASK] Reading program header\n");
	}
	ssize_t read_response = read(fd, phdr, sizeof(Elf32_Phdr));
	if (read_response != sizeof(Elf32_Phdr)) {
		dprint("[ERROR] Could not read program header\n");
		close(fd);
		exit(1);
	}

	if (READ_TOGGLE == 1) {
		dprint("[SUCCESS] Successfully read the program header\n\n");
	}

}

void go_to_program_header_offset() {
	dprint("[TASK] Seeking to program header offset\n");
	long int seek_response;
	seek_response = lseek(fd, ehdr->e_phoff, SEEK_SET);
	if (seek_response == -1) {
		dprint("[ERROR] Seek operation couldn't be performed\n");
		exit(1);
	}

	dprint("[SUCCESS] Seek : Response is %ld\n", seek_response);
}

void load_elf_header() {
	dprint("[TASK] Loading ELF Header\n");
	ssize_t read_response = read(fd, ehdr, sizeof(Elf32_Ehdr));
	dprint("[INFO] Read Response is %zd\n", read_response);
	if (read_response != sizeof(Elf32_Ehdr)) {
		dprint("[ERROR] ELF Header could not be loaded\n");
		exit(1);
	}

	dprint("[SUCCESS] ELF Header loaded successfully!\n\n");
}

void print_program_header(Elf32_Phdr *p) {
	if (DEBUG != 1) {
		return;
	}

	printf("+----------Program Header----------+\n");
	printf("| Type: 0x%x\n", p->p_type);
	printf("| Offset: 0x%x\n", p->p_offset);
	printf("| Virtual Address: 0x%x\n", p->p_vaddr);
	printf("| Physical Address: 0x%x\n", p->p_paddr);
	printf("| File Size: 0x%x\n", p->p_filesz);
	printf("| Memory Size: 0x%x\n", p->p_memsz);
	printf("| Alignment: 0x%x\n", p->p_align);
	printf("+----------------------------------+\n\n");
}

uintptr_t find_page_start(uintptr_t fault_address) {
	uintptr_t page_size = PAGE_SIZE;
	uintptr_t page_start = ((uintptr_t) fault_address) & -page_size;
	return page_start;
}

void *alloc_page(uintptr_t page_start) {
	size_t page_size = PAGE_SIZE;
	void *new_page = mmap((void *) page_start, page_size, PROT_READ | PROT_WRITE | PROT_EXEC,
						  MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
	return new_page;
}

int find_header_containing_addr(uintptr_t addr) { // returns 1 if found and 0 if not found
	dprint("[TASK] Find header containing addr %p\n", (void *)addr);
	go_to_program_header_offset();
	for (int i = 0; i < ehdr->e_phnum; i++) {
		read_program_header();
		if (phdr->p_type == 1) // Check if it is PT_LOAD type
		{
			if (phdr->p_vaddr - addr + phdr->p_memsz >= 0) {
				if (phdr->p_vaddr - addr + phdr->p_memsz <= phdr->p_filesz) {
					dprint("This program header below contains the given address %p!\n", (void *)addr);
					dprint("[SUCCESS] Found header containing given address %p\n\n", (void *)addr);
					// print_program_header(phdr);
					return 1;
				}
			}
		}
		// print_program_header(phdr);
	}

	dprint("[ERROR] No segment containing the entry point found!\n");
	return 0;
}

int write_part_of_segment_onto_page(void *page_addr) {
	dprint("[TASK] Writing segment data to newly allocated page\n");
	long int seek_response;
	__off_t seek_offset = (__off_t) (phdr->p_offset + (page_addr - phdr->p_vaddr));

	// Again using filesz instead of memsz, so that it works for sections like .bss which take more memsz than their filesz
	// size_t bytes_to_read = phdr->p_memsz - (size_t) (page_addr - phdr->p_vaddr); // <--- avoiding this
	ssize_t bytes_to_read = phdr->p_filesz - (size_t) (page_addr - phdr->p_vaddr);
	dprint("[INFO] Bytes to write %d\n", bytes_to_read);

	// In case of a section like .bss it is possible that the filesz is only a few bytes
	// Whereas the program tries to access data which is out of bounds of the filesz
	// So we need to check if the bytes to be read is greater than 0 or not
	if (bytes_to_read <= 0) {
		dprint("[INFO] Bytes to write on page are less than 0\n");
		dprint("[INFO] Implies nothing to write here! Returning\n");
		// Returning 0 for success because nothing had to be written
		return 0;
	}

	seek_response = lseek(fd, seek_offset, SEEK_SET);
	if (seek_response == -1) {
		dprint("[ERROR] Seek operation couldn't be performed\n");
		return -1;
	}

	dprint("[INFO] Seeked to header offset | Seek Response is %ld\n", seek_response);

	ssize_t read_response;
	read_response = read(fd, page_addr, bytes_to_read);
	dprint("[INFO] Read header to page | Read Response is %zd\n", read_response);
	if (read_response <= 0) {
		dprint("[ERROR] Error in reading to page\n");
		return -1;
	}

	TOTAL_BYTES_WRITTEN += read_response;
	dprint("[SUCCESS] Segment data written successfully to page\n");
	return 0;
}

void get_total_sizes() { // returns 1 if found and 0 if not found
	go_to_program_header_offset();
	for (int i = 0; i < ehdr->e_phnum; i++) {
		read_program_header();
		if (phdr->p_type == 1) // Check if it is PT_LOAD type
		{
			dprint("adding to total!\n");
			TOTAL_MEM_SZ += phdr->p_memsz;
			TOTAL_FILE_SZ += phdr->p_filesz;
		}
	}
}


void segfault_handler(int signo, siginfo_t *info, void *context) {
	dprint("\n[HANDLER] Segfault Handler Invoked\n");
	++PAGE_FAULT_COUNT;

	// The memory address that led to the SEGFAULT
	uintptr_t error_addr = (uintptr_t) info->si_addr;

	ucontext_t *ucontext = (ucontext_t *) context;

	// The program counter at which the SEGFAULT occured, only for debugging purposes
	saved_pc = (void *) ucontext->uc_mcontext.gregs[REG_EIP];

	dprint("[INFO] SegFault Address : %p\n", (void *)error_addr);
	dprint("[INFO] SegFault at PC : %p\n", saved_pc);

	uintptr_t page_start = find_page_start(error_addr);

	void *new_page = alloc_page(page_start);

	if (new_page == MAP_FAILED) {
		dprint("[ERROR] Page allocation failed!\n");
		perror("mmap");
	}

	dprint("[INFO] Page allocated at address : %p\n\n", new_page);
	++PAGES_ALLOCED_COUNT;
	add_page_to_ll(new_page);

	int header_found = find_header_containing_addr(error_addr);
	if (header_found == 1) {
		dprint("[INFO] Corresponding header found!\n");
		print_program_header(phdr);

		int written_to_page = write_part_of_segment_onto_page(new_page);

		if (written_to_page == -1) {
			dprint("[ERROR] Error occurred while writing data into the page");
			perror("read");
		}
	} else {
		dprint("\n[INFO] No matching header found!\n");
		dprint("Page was still allocated for program to access\n");
		dprint("But nothing as such to load\n");
	}


	printf("[HANDLER] Successfully handled the segmentation fault! Returning..\n\n");
}

void loader_cleanup() {
	dprint("[TASK] Cleaning up\n");
	unmap_and_free_ll();
	close(fd);
	free(ehdr);
	free(phdr);
	dprint("[SUCCESS] Cleanup done!\n\n");
}

void print_elf_header() {
	if (DEBUG != 1) {
		return;
	}

	printf("+------------Elf Header------------+\n");
	printf("| Section Header - Offset: 0x%x\n", ehdr->e_shoff);
	printf("| Section Header - Count: 0x%x\n", ehdr->e_shnum);
	printf("| Program Header - Offset: 0x%x\n", ehdr->e_phoff);
	printf("| Program Header - Count: 0x%x\n", ehdr->e_phnum);
	printf("| Entry Point - Virtual Add: 0x%x\n", ehdr->e_entry);
	printf("+----------------------------------+\n\n");
}


void check_elf_validity() {
	dprint("[TASK] Checking for ELF Header validity\n");
	if (ehdr->e_ident[1] == 'E' && ehdr->e_ident[2] == 'L' && ehdr->e_ident[3] == 'F') {
		dprint("[SUCCESS] ELF Header is valid!\n\n");
		return;
	}

	dprint("[ERROR] ELF Header is invalid\n");
	exit(1);
}


void load_file() {
	dprint("[TASK] Opening file...\n");
	fd = open("fib", O_RDONLY);
	if (fd == -1) {
		dprint("[ERROR] File could not be opened\n");
		exit(1);
	}

	dprint("[SUCCESS] File opened successfully!\n\n");
}

void init_global() {
	dprint("[TASK] Initializing global variables\n");
	ehdr = (Elf32_Ehdr *) malloc(sizeof(Elf32_Ehdr));
	if (ehdr == NULL) {
		dprint("[ERROR] Memory allocation of ehdr failed!\n");
		exit(1);
	}

	phdr = (Elf32_Phdr *) malloc(sizeof(Elf32_Phdr));
	if (phdr == NULL) {
		dprint("[ERROR] Memory allocation of phdr failed!\n");
		exit(1);
	}

	PAGE_FAULT_COUNT = 0;
	TOTAL_BYTES_WRITTEN = 0;
	PAGES_ALLOCED_COUNT = 0;
	TOTAL_FILE_SZ = 0;
	TOTAL_MEM_SZ = 0;

	dprint("[SUCCESS] Global variables initialized!\n\n");
}

void print_summary() {
	size_t fragmented_bytes = ((PAGES_ALLOCED_COUNT * PAGE_SIZE) - TOTAL_BYTES_WRITTEN);
	printf("\n+------------Summary---------------+\n");
	printf("| Page faults : %d\n", PAGE_FAULT_COUNT);
	printf("| Pages allocated : %d\n", PAGES_ALLOCED_COUNT);
	printf("| Bytes written : %zu bytes\n", TOTAL_BYTES_WRITTEN);
	printf("| Space lost in internal fragmentation : %zu kilobytes\n", fragmented_bytes / 1024);
	printf("| Space lost in internal fragmentation : %zu bytes (precise)\n", fragmented_bytes);
	printf("| Total memory bytes of PT_LOAD segments : %zu bytes\n", TOTAL_MEM_SZ);
	printf("| Total file bytes of PT_LOAD segments : %zu bytes\n", TOTAL_FILE_SZ);
	printf("+----------------------------------+\n\n");
}


void load_and_run_elf(char **exe) {
	// Load the ELF file in read-only mode
	fd = open(*exe, O_RDONLY);

	init_global();
	load_elf_header();
	check_elf_validity(); // Will exit the program if the ELF file is invalid
	print_elf_header();
	get_total_sizes();

	// Set the ELF header entry point as start location
	start_loc = ehdr->e_entry;

	// Typecasting the _start function pointer
	int (*_start)() = (int (*)()) start_loc;
	dprint("[INFO] Start function is located at %p\n", _start);

	// Setting up the signal handler before calling the start function
	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = segfault_handler;
	sigaction(SIGSEGV, &sa, NULL);

	dprint("\n[TASK] Calling the _start() function\n");
	int result = _start(); // Seg fault(s) will be received here
	printf("User _start return value = %d\n", result);
	dprint("[SUCCESS] Start function call was successful!\n\n");

	print_summary();
}

// int main(int argc, char **argv) {
// 	if (argc != 2) {
// 		printf("Usage: %s <ELF Executable> \n", argv[0]);
// 		exit(1);
// 	}
// 	// 1. carry out necessary checks on the input ELF file
// 	// 2. passing it to the loader for carrying out the loading/execution
// 	load_and_run_elf(&argv[1]);

// 	// 3. invoke the cleanup routine inside the loader
// 	loader_cleanup();
// 	return 0;
// }
