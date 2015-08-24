#include <linux/module.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>

#include <linux/seq_file.h>
#include <linux/proc_fs.h>

#include "mmap_kmalloc.h"

MODULE_DESCRIPTION("Allocating mmapable memory buffer.");
MODULE_AUTHOR("Aakarsh Nair"); 
MODULE_LICENSE("GPL");

// The pointer to memory area
char *kmalloc_ptr = NULL;

// The memory area allocated be kmalloc
static char *kmalloc_area = NULL;
static char* initial_value = "012345789";

// Length of memory area?
#define LEN (16*1024) 

// Virtual Address to memory area.
unsigned long virt_addr;

static int __init mmap_kmalloc_proc_init(void);

int mmap_kmalloc(struct file * filp, struct vm_area_struct * vma) {
  int ret;
  unsigned long length;
  length = vma->vm_end - vma->vm_start;

  // Restrict to size of device memory

  if (length > LEN * PAGE_SIZE)
    return -EIO;

  /**
   * remap_pfn_range function arguments:
   * vma: vm_area_struct has passed to the mmap method
   * vma->vm_start: start of mapping user address space
   * Page frame number of first page that you can get by:
   *   virt_to_phys((void *)kmalloc_area) >> PAGE_SHIFT
   * size: length of mapping in bytes which is simply vm_end - vm_start
   * vma->>vm_page_prot: protection bits received from the application
   */
  ret = remap_pfn_range(vma, vma->vm_start,
                        virt_to_phys((void*)((unsigned long)kmalloc_area)) >> PAGE_SHIFT,
                        vma->vm_end - vma->vm_start,
                        vma->vm_page_prot);
  if(ret != 0) {
    return -EAGAIN;
  }
  return 0;
}

int __init mmap_kmalloc_init_module (void) 
{
  /**
   * kmalloc() returns memory in bytes instead of PAGE_SIZE
   * mmap memory should be PAGE_SIZE and aligned on a PAGE boundary.
   */
  kmalloc_ptr = kmalloc(LEN + (2 * PAGE_SIZE), GFP_KERNEL);

  if (!kmalloc_ptr) {
    printk("kmalloc failed\n");
    return -ENOMEM;
  }

  printk("kmalloc_ptr at 0x%p \n", kmalloc_ptr);

  /**
   * This is the same as: 
   * (int *)((((unsigned long)kmalloc_ptr) + ((1<<12) - 1)) & 0xFFFF0000);
   * where: PAGE_SIZE is defined as 1UL <<PAGE_SHIFT. 
   * That is 4k on x86. 0xFFFF0000 is a PAGE_MASK to mask out the upper 
   * bits in the page. This will align it at 4k page boundary that means 
   * kmalloc start address is now page aligned.
   */
  kmalloc_area = (char *)(((unsigned long) kmalloc_ptr + PAGE_SIZE -1) & PAGE_MASK);

  printk("kmalloc_area: 0x%p\n", kmalloc_area);

  /* reserve kmalloc memory as pages to make them remapable */
  for (virt_addr=(unsigned long)kmalloc_area; virt_addr < (unsigned long)kmalloc_area + LEN;
       virt_addr+=PAGE_SIZE) {
    SetPageReserved(virt_to_page(virt_addr));
  }

  printk("kmalloc_area: 0x%p\n" , kmalloc_area);
  printk("kmalloc_area :0x%p \t physical Address 0x%lx)\n", kmalloc_area,
         virt_to_phys((void *)(kmalloc_area)));


  // Initialize kmalloc buffer
  memcpy(kmalloc_area,initial_value,10);
	  
  mmap_kmalloc_proc_init();
  
  return 0;
}

void __exit mmap_kmalloc_cleanup_module (void) {
  printk("cleaning up module\n");

  remove_proc_entry("mmap_kmalloc",NULL);

  for (virt_addr = (unsigned long) kmalloc_area; virt_addr < (unsigned long)kmalloc_area + LEN;
       virt_addr+=PAGE_SIZE) {
    // clear all pages
    ClearPageReserved(virt_to_page(virt_addr));
  }
  
  kfree(kmalloc_ptr);
}


static int mmap_kmalloc_proc_show(struct seq_file *m, void *v)
{
  seq_printf(m, "%s",kmalloc_ptr);
  return 0;
}

static int mmap_kmalloc_proc_open(struct inode *inode, struct file *file)
{
  return single_open(file, mmap_kmalloc_proc_show, NULL);
}


static const struct file_operations proc_operations = {
  .open		= mmap_kmalloc_proc_open,
  .read		= seq_read,
  .llseek	= seq_lseek,
  .release	= single_release,
};

static int __init mmap_kmalloc_proc_init(void)
{
  proc_create("mmap_kmalloc", 0, NULL, &proc_operations);
  return 0;
}

