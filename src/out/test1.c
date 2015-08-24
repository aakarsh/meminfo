#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#define BUFSIZE 64*1024

int test_kmalloc()
{
  int i, fd, len, wlen ;
  char * mptr;
  size_t size = BUFSIZE;
  char buffer[BUFSIZE];

  fd = open("/dev/mmap_kmalloc", O_RDWR | O_SYNC);
  if( fd == -1) {
    printf("open error...\n");
    return -1;
  }

  mptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if(mptr == MAP_FAILED) {
    printf("mmap() failed\n");
    return -1;
  }

  /**
   * Now mmap memory region can be access as user memory. No syscall 
   * overhead! 
   */

  /* read from mmap memory */
  printf("mptr is %p\n", mptr);
  memset(buffer, 0, size);       		/* Clear the buffer */
  memcpy(buffer, mptr, size-1);  		/* Reading from kernel */
  printf("Current Contents mmap:  '%s'\n", buffer);
  printf("mptr is %p\n", mptr);

  /* write to mmap memory */
  memcpy(mptr, "MY TEST STRING!", 16);
  memset(buffer, 0, size);
  // Read back 
  memcpy(buffer, mptr, size-1);
  printf("read back mmap:  '%s'\n", buffer);

  munmap(mptr, size);
  close(fd); 
}


int test_vmalloc()
{
  int i, fd, len, wlen ;
  char * mptr;
  size_t size = 16*1024;
  char buffer[16*1024];

  fd = open("/dev/mmap_vmalloc", O_RDWR | O_SYNC);
  if( fd == -1) {
    printf("open error...\n");
    return -1;
  }

  mptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if(mptr == MAP_FAILED) {
    printf("mmap() failed\n");
    return -1;
  }

  /**
   * Now mmap memory region can be access as user memory. No syscall 
   * overhead! 
   */

  /* read from mmap memory */
  printf("mptr is %p\n", mptr);
  memset(buffer, 0, size);       		/* Clear the buffer */
  memcpy(buffer, mptr, size-1);  		/* Reading from kernel */
  printf("Current Contents mmap:  '%s'\n", buffer);
  printf("mptr is %p\n", mptr);
  
  /* write to mmap memory */
  memcpy(mptr, "MY TEST STRING!", 16);
  memset(buffer, 0, size);
  // Read back 
  memcpy(buffer, mptr, size-1);
  printf("read back mmap:  '%s'\n", buffer);

  munmap(mptr, size);
  close(fd); 
}

int main()
{
  test_kmalloc();
  test_vmalloc();
  
}
