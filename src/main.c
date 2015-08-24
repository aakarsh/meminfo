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


static int __init mmap_main_init_module (void) 
{
  printk("mmap_main_init_module: Entry \n");
  printk("mmap_main_init_module: Exit \n");	  
  return 0;
}


static void __exit mmap_main_cleanup_module (void) {
  printk("mmap_main_cleanup_module: Exit \n");
}


module_init(mmap_main_init_module);
module_exit(mmap_main_cleanup_module);
