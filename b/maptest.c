/*
 * maptest.c
 * 
 *   $ gcc -o maptest -lm maptest.c
 *   $ ./maptest poem 5 25         # write 25 chars from offset 5 
 * 
 * Prints part of the file specified in its first command-line argument to 
 * standard output. The range of bytes to be printed is specified via offset 
 * and length values in the second and third command-line arguments. The 
 * program creates a memory mapping of the required pages of the file and then 
 * uses write(2) to output the desired bytes.
 *
 * Note: mmap allows multiple processes to share virtual memory; unlike SysV 
 * IPC, mmap structures are not persistent; when attached to a file there is 
 * no inherent synchronization so processes will not always see the same data;
 * the segment is copied into RAM and periodically flushed to disk; 
 * synchronization can be forced with the msync system call.
 * 
 * A file is mapped in multiples of the page size so the offset into the file
 * must be page aligned.  
 */

#include <sys/mman.h>   /* mmap() is defined in this header */
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define perror(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc, char *argv[])
{
	int fd;
	struct stat sb;  /* see below for struct stat definition */
	off_t offset, pa_offset;
	size_t length;

	if (argc != 4 ) {
		fprintf(stdout,"Usage: %s <infile> <offset> <length>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	fd = open(argv[1], O_RDONLY);  /* open the input file for reading */
	if (fd == -1)
		perror("open");

	if (fstat(fd, &sb) == -1)     /* returns file size in struct stat sb */
		perror("fstat");

	/* fyi: for complete descripton of struct stat see fstat manpage;
	 * this is just a partial definition - fstat returns all known
	 * metadata on a file
	 *
	 *   struct stat {
	 *      ino_t  st_ino;     # inode number 
	 *      mode_t st_mode;    # protection 
	 *      uid_t  st_uid;     # user ID of owner 
	 *      gid_t  st_gid;     # group ID of owner 
	 *      off_t  st_size;    # total file size, in bytes 
	 *      time_t st_atime;   # time of last access 
	 *      time_t st_mtime;   # time of last modification 
	 *      time_t st_ctime;   # time of last status change
	 *           .... 
	 *    }; 
	 */

	printf("filesize: %ld bytes\n", sb.st_size); 
	offset = atoi(argv[2]);
	/* determine page offset in case you need to re-align things;
	   this is just finding your starting page */
	pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1); /* ~ is bitwise not */ 
	printf("pa_offset %ld\n", pa_offset); 

	/* offset for mmap() must be page aligned - this is done as follows
	 *         
	 *   0001 0000 0000 0000   # 4096 is the page size
	 *   0000 1111 1111 1111   # 4096 - 1
	 *
	 *    1111 0000 0000 0000   # ~4095         
	 *  & 0000 0000 0000 0011   # for example use an offset of 3 
	 *    -------------------  
	 *    0000 0000 0000 0000 
	 */ 

	if (offset >= sb.st_size) {
		fprintf(stderr, "offset is past end of file\n");
		exit(EXIT_FAILURE);
	}

	if (argc == 4) {
		length = atoi(argv[3]);
		if (offset + length > sb.st_size)
			length = sb.st_size - offset;
		/* Can't display bytes past end of file */
	} else {  
		/* if you made it here there is no length arg so just display to EOF */
		length = sb.st_size - offset;
	}

	/* From mmap manpage:
	 *
	 *   void *mmap(void *addr, size_t length, int prot, int flags, 
	 *           int fd, off_t offset);
	 *
	 * mmap() creates a new mapping in the virtual address space of the calling
	 * process. The starting address for the new mapping is specified
	 * in addr. The length argument specifies the length of the mapping.
	 *
	 * If addr is NULL, the kernel chooses the address to create the mapping
	 * otherwise addr is a *recommendation*; the address of new mapping is
	 * returned.
	 *
	 * The contents of a file mapping are initialized using length bytes
	 * starting at offset offset in the file referred to by the file descriptor
	 * fd. Offset must be a multiple of the page size as returned by
	 * sysconf(_SC_PAGE_SIZE).
	 *
	 * The prot arg defines the memory protection of the mapping:
	 *       PROT_EXEC  Pages may be executed.
	 *       PROT_READ  Pages may be read.
	 *       PROT_WRITE Pages may be written.
	 *       PROT_NONE  Pages may not be accessed.
	 *
	 * The flags argument determines whether updates to the mapping are visible
	 * to other processes mapping the same region, and whether updates are
	 * carried through to the underlying file.This behavior is determined by
	 * including exactly one of the following values in flags:
	 *
	 * MAP_SHARED Share this mapping. Updates to the mapping are visible to
	 * other processes that map this file and are carried through to the
	 * underlying file. The file may not actually be updated until msync(2) or
	 * munmap() is called.
	 *
	 * MAP_PRIVATE
	 * Create a private copy-on-write mapping. Updates to the mapping are not 
	 * visible to other processes mapping the same file, and are not carried 
	 * through to the underlying file.
	 *
	 * Memory mapped by mmap() is preserved across fork(2) with the same
	 * attributes.
	 *
	 * A file is mapped in multiples of the page size. For a file that is not
	 * a multiple of the page size, the remaining memory is zeroed when mapped,
	 * and writes to that region are not written out to the file.  
	 *
	 * MAP_ANONYMOUS
	 * The mapping is not backed by any file; its contents are initialized to
	 * zero. The fd and offset arguments are ignored; however, some
	 * implementations require fd to be -1 for MAP_ANONYMOUS and portable
	 * applications should do this. The use of MAP_ANONYMOUS with MAP_SHARED
	 * is supported since kernel 2.4.
	 *
	 * RETURN VALUE 
	 * On success, mmap() returns a pointer to the mapped area.  
	 *
	 * Use of a mapped region can result in these signals:
	 *
	 * SIGSEGV 
	 * Attempted write into a region mapped as read-only.
	 *
	 * SIGBUS 
	 * Attempted access to a portion of the buffer that does not correspond to
	 * the file (for example, beyond end of the file.)
	 * 
	 */

	char *addr = mmap(NULL, length + offset - pa_offset, PROT_READ,
												MAP_PRIVATE, fd, pa_offset);
	if (addr == MAP_FAILED)
		perror("mmap");

	int pagesz = getpagesize(); 
	pid_t mypid = getpid();
	printf("pid: %d pagesize: %d \n", mypid, pagesz);

	ssize_t s;
	/*write(STDOUT_FILENO,"\n",2);*/
	write(STDOUT_FILENO, "\n", 1);
	s = write(STDOUT_FILENO, addr + offset - pa_offset, length); 
	write(STDOUT_FILENO, "\n", 1);
	if (s != length) {
		if (s == -1)
			perror("write");
		fprintf(stderr, "partial write");
		exit(EXIT_FAILURE);
	}
	munmap(addr, length + offset - pa_offset);
	close(fd);
	exit(EXIT_SUCCESS);
}

