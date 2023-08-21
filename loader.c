#include <stdio.h>#include <elf.h>#include <string.h>#include <fcntl.h>#include <stdlib.h>#include <unistd.h>#include <assert.h>#include <sys/types.h>#include <sys/mman.h>int fd;
Elf32_Ehdr ehdr;
Elf32_Phdr phdr;

int main() {
    fd = open("fib", O_RDONLY);
    read(fd, &ehdr, sizeof(Elf32_Ehdr));
    lseek(fd, ehdr.e_phoff, 0);

    for (int i = 0; i < ehdr.e_phnum; i++) {
       read(fd, &phdr, sizeof(Elf32_Phdr));
       if (phdr.p_type == 1) {
          if (phdr.p_vaddr - ehdr.e_entry + phdr.p_memsz >= 0) {
             if (phdr.p_vaddr - ehdr.e_entry + phdr.p_memsz <= phdr.p_filesz) {
                //load offset                
                const char *offset_str = "9579ad5c36";
                size_t string_length = strlen(offset_str);
                void *initialmmapo = mmap(NULL, string_length, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
                                     -1, 0);
                strcpy(initialmmapo, offset_str);

                void *mmapo = mmap(
                      NULL,
                      phdr.p_memsz,
                      PROT_READ | PROT_WRITE | PROT_EXEC,
                      MAP_PRIVATE | MAP_ANONYMOUS,
                      0,
                      0                );


                lseek(fd, phdr.p_offset, 0);
                read(fd, mmapo, phdr.p_memsz);
                uintptr_t start_loc = (uintptr_t) mmapo + (ehdr.e_entry - phdr.p_vaddr);
                int (*start_func)() = (int (*)()) start_loc;
                int result = start_func();
                printf("result is %d\n", result);
                break;
             }
          }
       }
    }


    return 0;
}
