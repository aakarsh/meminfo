#include <linux/module.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
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

static struct proc_dir_entry * meminfo_proc_entry;

static int meminfo_proc_open(struct inode *inode, struct file *file);
static int meminfo_proc_show(struct seq_file *m, void *v);


static const struct file_operations meminfo_proc_fops = {
	.open		= meminfo_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int meminfo_proc_show(struct seq_file *m, void *v)
{
  seq_printf(m,
             "Total RAM :             %8ld\n"
             "Total Pages:            %8ld\n"
             "Pages Size:             %8ld\n"
             "Page Shift:             %8d\n"

             ,NPAGES_KB(totalram_pages)
             ,totalram_pages
             ,PAGE_SIZE
             ,PAGE_SHIFT);


  seq_printf(m,"Page State Information \n");

  seq_printf(m,
             "Anon Active:              %8lu\n"
             "Anon InActive:            %8lu\n"
             "Isolated Anonymous:       %8lu\n"
             "Active File:              %8lu\n"
             "Inactive File:            %8lu\n"
             "Isolated File:            %8lu\n"
             "Unevictable:              %8lu\n"
             "Dirty:                    %8lu\n"
             "Writeback:                %8lu\n"
             "Unstable:                 %8lu\n"
             "Free :                    %8lu\n"
             "Slab Reclaimable :        %8lu\n"
             "Slab UnReclaimable :      %8lu\n"
             "Mapped Pages :            %8lu\n"
             "Shmem Pages :             %8lu\n"
             "Page Tables  :            %8lu\n"
             "Bounce  :                 %8lu\n"
             "Free CMA  :               %8lu\n",
             global_page_state(NR_ACTIVE_ANON),
             global_page_state(NR_INACTIVE_ANON),
             global_page_state(NR_ISOLATED_ANON),
             global_page_state(NR_ACTIVE_FILE),
             global_page_state(NR_INACTIVE_FILE),
             global_page_state(NR_ISOLATED_FILE),
             global_page_state(NR_UNEVICTABLE),
             global_page_state(NR_FILE_DIRTY),
             global_page_state(NR_WRITEBACK),
             global_page_state(NR_UNSTABLE_NFS),
             global_page_state(NR_FREE_PAGES),
             global_page_state(NR_SLAB_RECLAIMABLE),
             global_page_state(NR_SLAB_UNRECLAIMABLE),
             global_page_state(NR_FILE_MAPPED),
             global_page_state(NR_SHMEM),
             global_page_state(NR_PAGETABLE),
             global_page_state(NR_BOUNCE),
             global_page_state(NR_FREE_CMA_PAGES));


  return 0;
}



static int __init meminfo_main (void) 
{
  printk("meminfo_main: Entry \n");

  meminfo_proc_entry = proc_create("meminfo_full", 0, NULL, &meminfo_proc_fops);    

  printk("meminfo_main: Exit \n");	  
  return 0;
}


static void __exit meminfo_cleanup_module (void) {
  printk("meminfo_cleanup_module: Exit \n");
  proc_remove(meminfo_proc_entry);
}

/**
 * Delegate showing proc entry to meminfo_proc_show
 */
static int meminfo_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, meminfo_proc_show, NULL);
}




module_init(meminfo_main);
module_exit(meminfo_cleanup_module);
