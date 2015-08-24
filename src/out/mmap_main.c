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

#include "mmap_kmalloc.h"
#include "mmap_vmalloc.h"

MODULE_DESCRIPTION("Allocating mmapable memory buffer.");
MODULE_AUTHOR("Aakarsh Nair"); 
MODULE_LICENSE("GPL");

static int mmap_major;

static dev_t  dev;
static struct cdev   *my_cdev;
static struct class *mmap_class;

static char* kmalloc_dev_name = "mmap_kmalloc";
static char* vmalloc_dev_name = "mmap_vmalloc";

static int dev_open(struct inode *inode, struct file *filp);
static int dev_release(struct inode *inode, struct file *filp);
static int dev_mmap(struct file *filp, struct vm_area_struct *vma);

static struct file_operations fops = {
  owner:   THIS_MODULE,
  open:    dev_open,
  release: dev_release,
  mmap: dev_mmap
};

/**
 * Stores the inode's minor number in the file's private data field
 * allowing the mmap routine to distinguish which kind of memory has
 * been requested for mapping.
 */
static int dev_open(struct inode *inode, struct file *filp)
{
  int minor= iminor(inode);
  printk(KERN_INFO "mmap_main: a device opened  minor:%d\n",minor);
  filp->private_data = (void*) minor;
  return 0;
}

static int dev_release(struct inode *inode, struct file *filp)
{
  printk(KERN_INFO "mmap_mmap: a device closed\n");  

  return 0;
}

/**
 * Based on device minor number dispatch to either the kmalloc or
 * vmallac buffer handler.
 */
static int dev_mmap(struct file *filp, struct vm_area_struct *vma)
{
  int minor = (int) filp->private_data;
  printk(KERN_INFO "mmap_main: mmap called with minor %d \n",minor);    
  switch(minor){
  case 0:
    mmap_kmalloc(filp,vma);
    break;
  case 1:
    mmap_vmem(filp,vma);
    break;
  default:
    return -ENOTTY;
  }

  return 0;
}

static int __init mmap_main_init_module (void) 
{
  int ret, baseminor = 0;

  mmap_kmalloc_init_module();
  mmap_vmem_init_module();

  // Dynamically allocate character region.

  ret = alloc_chrdev_region(&dev, 
                            0,  //baseminor
                            2, // count
                            "mmap_main" //name
                            ); 

  if(ret < 0 ) { return ret; }

  // Device major number
  mmap_major = MAJOR(dev);    
  
  // Allocate cdev struction to handle device fops
  my_cdev = cdev_alloc();
  cdev_init(my_cdev,&fops);
  my_cdev->owner = THIS_MODULE;

  // Connect device to driver
  ret = cdev_add(my_cdev,dev,2);
  if(ret < 0) { 
    unregister_chrdev_region(dev,2);
    return ret;
  }
  
  mmap_class = class_create(THIS_MODULE,"mmap_main");
  if(IS_ERR(mmap_class)) {
    return PTR_ERR(mmap_class);
  }

  // Create two devices in /dev which will handle mmap differently
  device_create(mmap_class,
                NULL, //parent
                MKDEV(mmap_major,baseminor),
                NULL, //drvdata
                kmalloc_dev_name);
  
  device_create(mmap_class,
                NULL, //parent
                MKDEV(mmap_major,baseminor+1),
                NULL, //drvdata
                vmalloc_dev_name);
	  
  return 0;
}


static void __exit mmap_main_cleanup_module (void) {
  printk("cleaning up module\n");

  mmap_kmalloc_cleanup_module();
  mmap_vmem_cleanup_module();

  cdev_del(my_cdev);
  unregister_chrdev_region(dev, 2);

  // Remove device node
  device_destroy(mmap_class,dev);
  device_destroy(mmap_class,MKDEV(mmap_major,1));

  class_destroy(mmap_class);
}


module_init(mmap_main_init_module);
module_exit(mmap_main_cleanup_module);
