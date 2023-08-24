#include "loader.h"

// Set DEBUG = 1, to get detailed step-wise output
int DEBUG = 0;
#define dprint      \
	if (DEBUG == 1) \
	printf

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
void *MAPPED_MEMORY;
int fd;

/*
 * release memory and other cleanups
 */

void loader_cleanup()
{
	dprint("[TASK] Cleaning up\n");
	munmap(MAPPED_MEMORY, phdr->p_memsz);
	close(fd);
	free(ehdr);
	free(phdr);
	dprint("[SUCCESS] Cleanup done!\n\n");
}

void print_elf_header()
{
	if (DEBUG != 1)
	{
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

void print_program_header(Elf32_Phdr *p)
{
	if (DEBUG != 1)
	{
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

void *load_header_to_memory()
{
	dprint("[TASK] Loading the segment containing the entry point\n");

	dprint("[TASK] Executing mmap\n");
	MAPPED_MEMORY = mmap(
		NULL,
		phdr->p_memsz,
		PROT_READ | PROT_WRITE | PROT_EXEC,
		MAP_ANONYMOUS | MAP_PRIVATE,
		0,
		0);

	dprint("[SUCCESS] mmap Location created at %p\n\n", MAPPED_MEMORY);

	long int seek_response;
	seek_response = lseek(fd, phdr->p_offset, SEEK_SET);
	if (seek_response == -1)
	{
		dprint("[ERROR] Seek operation couldn't be performed\n");
		exit(1);
	}

	dprint("[INFO] Seeked to header offset | Seek Response is %ld\n", seek_response);

	ssize_t read_response;
	read_response = read(fd, MAPPED_MEMORY, phdr->p_memsz);
	dprint("[INFO] Read header to MAPPED_MEMORY | Read Response is %zd\n", read_response);
	close(fd);

	dprint("[SUCCESS] phdr Loaded at Mem Loc : 0x%x | Entry Point Address is 0x%x\n\n", phdr->p_vaddr, ehdr->e_entry);
	dprint("[INFO] Mapped memory location is %p\n", MAPPED_MEMORY);
	dprint("[INFO] Delta of phdr v_addr and Entry Point Address is 0x%x\n", ehdr->e_entry - phdr->p_vaddr);
	dprint("[SUCCESS] Successfully loaded the segment to memory!\n\n");
	return MAPPED_MEMORY;
}

void read_program_header()
{
	dprint("[TASK] Reading program header\n");
	ssize_t read_response = read(fd, phdr, sizeof(Elf32_Phdr));
	if (read_response != sizeof(Elf32_Phdr))
	{
		dprint("[ERROR] Could not read program header\n");
		close(fd);
		exit(1);
	}

	dprint("[SUCCESS] Successfully read the program header\n\n");
}

void find_header_containing_entry_point()
{
	dprint("[TASK] Find header containing entry point\n");
	for (int i = 0; i < ehdr->e_phnum; i++)
	{
		read_program_header();

		if (phdr->p_type == 1)
		{
			if (phdr->p_vaddr - ehdr->e_entry + phdr->p_memsz >= 0)
			{
				if (phdr->p_vaddr - ehdr->e_entry + phdr->p_memsz <= phdr->p_filesz)
				{
					dprint("This program header below contains the entry point!\n");
					dprint("[SUCCESS] Found header containing entry point\n\n");
					print_program_header(phdr);
					return;
				}
			}
		}
		print_program_header(phdr);
	}

	dprint("[ERROR] No segment containing the entry point found!\n");
	exit(1);
}

void check_elf_validity()
{
	dprint("[TASK] Checking for ELF Header validity\n");
	if (ehdr->e_ident[1] == 'E' && ehdr->e_ident[2] == 'L' && ehdr->e_ident[3] == 'F')
	{
		dprint("[SUCCESS] ELF Header is valid!\n\n");
		return;
	}

	dprint("[ERROR] ELF Header is invalid\n");
	exit(1);
}

void go_to_program_header_offset()
{
	dprint("[TASK] Seeking to program header offset\n");
	long int seek_response;
	seek_response = lseek(fd, ehdr->e_phoff, SEEK_SET);
	if (seek_response == -1)
	{
		dprint("[ERROR] Seek operation couldn't be performed\n");
		exit(1);
	}

	dprint("[SUCCESS] Seek : Response is %ld\n", seek_response);
}

void load_elf_header()
{
	dprint("[TASK] Loading ELF Header\n");
	ssize_t read_response = read(fd, ehdr, sizeof(Elf32_Ehdr));
	dprint("[INFO] Read Response is %zd\n", read_response);
	if (read_response != sizeof(Elf32_Ehdr))
	{
		dprint("[ERROR] ELF Header could not be loaded\n");
		exit(1);
	}

	dprint("[SUCCESS] ELF Header loaded successfully!\n\n");
}

void load_file()
{
	dprint("[TASK] Opening file...\n");
	fd = open("fib", O_RDONLY);
	if (fd == -1)
	{
		dprint("[ERROR] File could not be opened\n");
		exit(1);
	}

	dprint("[SUCCESS] File opened successfully!\n\n");
}

void init_global()
{
	dprint("[TASK] Initializing global variables\n");
	ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
	if (ehdr == NULL)
	{
		dprint("[ERROR] Memory allocation of ehdr failed!\n");
		exit(1);
	}

	phdr = (Elf32_Phdr *)malloc(sizeof(Elf32_Phdr));
	if (ehdr == NULL)
	{
		dprint("[ERROR] Memory allocation of phdr failed!\n");
		exit(1);
	}

	dprint("[SUCCESS] Global variables initialized!\n\n");
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char **exe)
{
	//   1. Load entire binary content into the memory from the ELF file.
	//   2. Iterate through the PHDR table and find the section of PT_LOAD
	//      type that contains the address of the entrypoint method in fib.c
	//   3. Allocate memory of the size "p_memsz" using mmap function
	//      and then copy the segment content
	//   4. Navigate to the entrypoint address into the segment loaded in the memory in above step
	//   5. Typecast the address to that of function pointer matching "_start" method in fib.c.
	//   6. Call the "_start" method and print the value returned from the "_start"

	// Start
	fd = open(*exe, O_RDONLY);

	init_global();
	load_elf_header();
	check_elf_validity();
	print_elf_header();
	go_to_program_header_offset();
	find_header_containing_entry_point();
	load_header_to_memory();

	uintptr_t start_loc = (uintptr_t)MAPPED_MEMORY + (ehdr->e_entry - phdr->p_vaddr);
	int (*_start)() = (int (*)())start_loc;
	dprint("[INFO] Start function calculated to be at %p\n", _start);
	dprint("\n[TASK] Calling the _start() function\n");

	// End
	int result = _start();
	printf("User _start return value = %d\n", result);
	dprint("[SUCCESS] Start function call was successful!\n\n");
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
