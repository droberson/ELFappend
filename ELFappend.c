#include <stdio.h>
#include <elf.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>


/* get_file_size() -- returns size of file in bytes
 */
size_t get_file_size(const char *filename) {
  struct stat s;

  if (stat(filename, &s) == -1) {
    fprintf(stderr, "Failed to stat file %s: %s\n", filename, strerror(errno));
    return -1;
  }

  return s.st_size;
}


/* get_elf_size() -- returns length of ELF data for a file.
 *
 * TODO: validate ELF file
 *       figure out actual header size of ELF (malloc)
 */
size_t get_elf_size(const char *progname) {
  int fd;
  void *ELFheaderdata;
  Elf64_Ehdr *ELFheader;
  size_t elfsize;


  ELFheaderdata = malloc(64);

  fd = open(progname, O_RDONLY);
  if (fd == -1) {
    fprintf(stderr, "Failed to open input file %s: %s\n",
	    progname,
	    strerror(errno));
    fprintf(stderr, "Exiting.\n");

    exit(EXIT_FAILURE);
  }

  read(fd, ELFheaderdata, 64);
  ELFheader = (Elf64_Ehdr *)ELFheaderdata;


  elfsize = ELFheader->e_shoff + (ELFheader->e_shnum * ELFheader->e_shentsize);
  
  close(fd);
  free(ELFheaderdata);

  return elfsize;
}

int main(int argc, char *argv[]) {
  char ch;
  FILE *fp;
  char progname[PATH_MAX];
  size_t filesize;
  size_t elfsize;

  realpath("/proc/self/exe", progname);
  printf("Hello there! I am %s\n", progname);

  filesize = get_file_size(progname);
  printf("I am %ld bytes long\n", filesize);

  elfsize = get_elf_size(progname);
  printf("%ld bytes is valid ELF data.\n", elfsize);

  printf("This means that there are %ld extra bytes at the end of this file\n",
	 filesize - elfsize);

  if (filesize - elfsize > 0) {
    printf("dumping data....\n");

    fp = fopen(progname, "rb");
    if (fp == NULL) {
      printf("Unable to open %s for reading: %s", progname, strerror(errno));
      exit(EXIT_FAILURE);
    }
    
    fseek(fp, elfsize, SEEK_SET);

    while (fread(&ch, 1, 1, fp))
      fprintf(stderr, "%c", ch);

    fclose(fp);
  }
}
