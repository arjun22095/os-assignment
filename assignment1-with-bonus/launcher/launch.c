#include "loader.h"

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    printf("Usage: %s <ELF Executable> \n", argv[0]);
    exit(1);
  }
  // 1. carry out necessary checks on the input ELF file

  int f = open(argv[1], O_RDONLY);
  uint8_t ideal[4] = {0x7F, 'E', 'L', 'F'};
  uint8_t observed[4];
  ssize_t read_resp = read(f, observed, sizeof(observed));
  for (int i = 0; i < 4; ++i)
  {
    if (observed[i] != ideal[i])
    {
      printf("[ERROR] ELF File Invalid\n");
      exit(1);
    }
  }

  // printf("[SUCCESS] ELF File Valid\n");
  lseek(f, 0, SEEK_SET);
  close(f);

  // 2. passing it to the loader for carrying out the loading/execution
  load_and_run_elf(&argv[1]);
  // 3. invoke the cleanup routine inside the loader
  loader_cleanup();
  return 0;
}
