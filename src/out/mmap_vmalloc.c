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

static char *vmalloc_ptr = NULL;
static char *initial_value = "abcdefghijklmnopqrstuvwxyz"; 

#define LEN (16*1024)
static int __init mmap_vmalloc_proc_init(void);

int mmap_vmem(struct file *filp, struct vm_area_struct *vma)
{
  int ret;
  long length = vma->vm_end - vma->vm_start;
  unsigned long start = vma->vm_start;
  char *vmalloc_area_ptr = (char *) vmalloc_ptr;
  unsigned long pfn;

  /* Restrict it to size of device memory */
  if (length > LEN * PAGE_SIZE)
    return -EIO;

  /** 
   * Considering vmalloc pages are not contiguous in physical memory
   * You need to loop over all pages and call remap_pfn_range 
   * for each page individuallay. Also, use 
   * vmalloc_to_pfn(vmalloc_area_prt)
   * instead to get the page frame number of each virtual page
   */
  while (length > 0) {
    pfn = vmalloc_to_pfn(vmalloc_area_ptr);
    printk("vmalloc_area_ptr: 0x%p \n", vmalloc_area_ptr);

    if ((ret = remap_pfn_range(vma,  // user vma to map to
                               start, // target user address to start at
                               pfn,   // physical address of kernel memory
                               PAGE_SIZE, // size of map area
                               vma->vm_page_prot)) < 0) {
      return ret;
    }

    start += PAGE_SIZE;
    vmalloc_area_ptr += PAGE_SIZE;
    length -= PAGE_SIZE;

  }

  return 0;
}

int __init mmap_vmem_init_module (void) {

  unsigned long virt_addr;
  
  /* Do required char driver initialization, see helloplus.c as an example */

  /* Allocate  memory  with vmalloc. It is already page aligned */
  vmalloc_ptr = ((char *)vmalloc(LEN));
  if (!vmalloc_ptr) {
    printk("vmalloc failed\n");
    return -ENOMEM;
  }

  printk("vmalloc_ptr at 0x%p \n", vmalloc_ptr);

  /* reserve vmalloc memory to make them remapable */
  for (virt_addr=(unsigned long)vmalloc_ptr; 
       virt_addr < (unsigned long)vmalloc_ptr + LEN; 
       virt_addr+=PAGE_SIZE) {
    SetPageReserved(vmalloc_to_page((unsigned long *)virt_addr));
  }

  printk("vmalloc_ptr: 0x%p\n" , vmalloc_ptr);
  printk("vmalloc_ptr :0x%p \t physical Address 0x%lx)\n", vmalloc_ptr,
         (long unsigned int)virt_to_phys((void *)(vmalloc_ptr)));

  // Initialize kmalloc buffer
  memcpy(vmalloc_ptr,initial_value,25);    

  mmap_vmalloc_proc_init();

  return 0;
}

// close and cleanup module
void __exit mmap_vmem_cleanup_module (void) {
  unsigned long virt_addr;
  printk("cleaning up module\n");

  remove_proc_entry("mmap_vmalloc",NULL);

  for (virt_addr=(unsigned long)vmalloc_ptr; virt_addr < (unsigned long)vmalloc_ptr + LEN;
       virt_addr+=PAGE_SIZE) {
    // clear all pages
    ClearPageReserved(vmalloc_to_page((unsigned long *)virt_addr));
  }
  vfree(vmalloc_ptr);
  /* Also all the required cleanup for character drivers */  
}


static int mmap_vmalloc_proc_show(struct seq_file *m, void *v)
{
  seq_printf(m, "%s",vmalloc_ptr);
  return 0;
}

static int mmap_vmalloc_proc_open(struct inode *inode, struct file *file)
{
  return single_open(file, mmap_vmalloc_proc_show, NULL);
}


static const struct file_operations proc_operations = {
  .open		= mmap_vmalloc_proc_open,
  .read		= seq_read,
  .llseek	= seq_lseek,
  .release	= single_release,
};

static int __init mmap_vmalloc_proc_init(void)
{
  proc_create("mmap_vmalloc", 0, NULL, &proc_operations);
  return 0;
}
