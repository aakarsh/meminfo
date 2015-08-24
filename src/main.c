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

MODULE_DESCRIPTION("Try to collect information about memory usage");
MODULE_AUTHOR("Aakarsh Nair"); 
MODULE_LICENSE("GPL");

// Display in kilobyte
#define NPAGES_KB(npages) (npages << (PAGE_SHIFT -10))

static int __init meminfo_main (void) 
{
  printk("meminfo_main: Entry \n");
  
  
  printk("Total RAM : %ld\n",NPAGES_KB(totalram_pages)); 
  printk("Total Pages: %ld\n",totalram_pages);
  printk("Pages Size: %ld\n",PAGE_SIZE);

  printk("Page Shift: %d\n",PAGE_SHIFT);

  printk("meminfo_main: Exit \n");	  
  return 0;
}


static void __exit meminfo_cleanup_module (void) {
  printk("meminfo_cleanup_module: Exit \n");
}

module_init(meminfo_main);
module_exit(meminfo_cleanup_module);
