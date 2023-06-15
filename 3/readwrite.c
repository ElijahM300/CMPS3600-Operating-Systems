/*  cs360/Code/readwrite.c
 *  demonstrate 
 *    o  open(2), read(2), write(2), fopen(2)
 *    o  reading/writing to files or stdin/stdout 
 *    o  use of command-line arguments
 *
 *  argv[0] holds name of executable; argv[1] holds first cmdline arg, etc.
 *  argc holds the number of cmdline args 
 *
 *  Usage: ./readwrite <ifname> <ofname> <bytes> 
 *
 *  Example:
 *     $ gcc -o readwrite readwrite.c
 *     $ ./readwrite in out 20   # copy 20 bytes from in to out 
 *
 *  Use read(),write() in handlers - they are async-safe
 *  Use printf for debugging purposes only.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define DEBUG 1     /* flip this if you want verbose output */
#define BUFSIZE 1024  /* this is the maximum number of bytes you can read */

int stdinYES = 0;  
char *ifname;   /* the input filename */
char *ofname;   /* the output filename */
int num_bytes;  /* the number of bytes you want to copy */
int infd;       /* file descriptor for ifname */
int outfd;      /* file descriptor for ofname */
int errno;      /* this global is set by the kernel for any system call */

void cleanup();

int main( int argc, char * argv[])
{
  char buf[BUFSIZE];  
  memset(buf, 0, BUFSIZE);   /* fill the buffer with null bytes */
  int sz;

  if (argc == 4) {  

     /* malloc grabs memory from heap */
     ifname = (char *) malloc(sizeof(argv[1]));  
     strncpy(ifname,argv[1],sizeof(argv[1]));  
     if (strncmp(ifname,"stdin",5)== 0) 
         stdinYES = 1;

     ofname = (char *) malloc(sizeof(argv[2]));  
     strcpy(ofname,argv[2]);

     /* convert a string to an integer */
     num_bytes = atoi(argv[3]);

     /* convert an int to a string to display it with write */
     char str[10];
     sprintf(str,"%d\0",num_bytes);
     write(1,strcat(str,"\n"),strlen(str)+1);
     fsync(1);
   }
   else {
     printf("\nUsage: ./readwrite [ifname | stdin] ofname bytes\n");
     exit(1);
   } 

  /*  The open() system call returns a file descriptor. The various flags 
   *  control how the file is opened. O_CREAT creates the file if it does not 
   *  exist; if the file does exist O_TRUNC sets its size to 0; O_WRONLY
   *  opens the file in write only mode; 0644 sets file permissions to 644; 
   *  another useful flag is O_APPEND which appends to an existing file
   */

  /* open the output file for writing - truncate if already created */
  outfd = open(ofname, O_CREAT | O_WRONLY | O_TRUNC , 0644);
  if (outfd < 0) {
     perror("open ofname");
  }

  /* open the input file for reading unless stdin flag is set */
  if (stdinYES) 
     infd = 0;
  else {
     infd = open(ifname, O_RDONLY);
     if (infd < 0) { 
        perror("open ifname");
        fprintf(stderr,"Global errno is set: %d\n", errno);
        cleanup();
     }
  }
  if (num_bytes > BUFSIZE) num_bytes = BUFSIZE; 
  sz = read(infd, buf, num_bytes);  /* returns the number of bytes read */
  if (DEBUG) printf("bytes read: %d\n",sz);
  if (sz < 0) { 
     perror("read");
  } 
  buf[sz] = '\0';   /* always mark the end of a string with \0 */

  if (DEBUG) printf("\n%s\nWe just read %d bytes.\n",buf,sz);

  if (write(outfd,buf,sz) < 0)   {  /* write to output file */
     perror("write");
  } 

  /*
   * DEMONSTRATE FILE STREAMS 
   */
  FILE *fin;
  char c;
  if (fin = fopen("foo", "rt"))
  {
    /* use fscanf for reading */
    for (c; !feof(fin); fscanf(fin, "%c", &c))
       fprintf(stderr,"%c", c);
    fprintf(stderr,"\n");
    fclose(fin);
  }

  /* demonstrate fstat(2) */
  char chin[1];
  char tmp[60];
  strcpy(tmp,"Show file stats? [y|n]: ");
  write(1,tmp,strlen(tmp));
  read(0,chin,1);
  if (chin[0]=='n')
     exit(EXIT_SUCCESS);
  if (chin[0]=='y')
     write(1,"Are you sure?\n",15);
  /* display file information */
  struct stat fileStat;
  if (fstat(infd, &fileStat) == -1)
      perror("fstat: ");
  printf("OutFile Name:  \t%s\n",ofname);
  printf("OutFile Handle:\t%d\n",outfd);
  printf("Infile Name:   \t%s\n",ifname);
  printf("Infile Handle: \t%d\n",infd);
  printf("Infile Size:   \t%d bytes\n",fileStat.st_size);
  printf("Infile Links:  \t%d\n",fileStat.st_nlink);
  printf("Infile inode:  \t%d\n",fileStat.st_ino);
  printf("Permissions:   \t");
  printf( (S_ISDIR(fileStat.st_mode)) ? "d" : "-");
  printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
  printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
  printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
  printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
  printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
  printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
  printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
  printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
  printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-");
  printf("\n");

  cleanup();
}

void cleanup() {
  if (close(infd) < 0) { 
     perror("close ifname");
  } 
  if (close(outfd) < 0) { 
     perror("close ofname");
  } 
  free(ifname);  /* free what you allocate to prevent memory leaks */ 
  free(ofname);
  exit(0);
}
