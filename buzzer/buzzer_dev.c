#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/ktime.h>
#include <linux/jiffies.h>


#include <asm/mach/map.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#define BUZZER_MAJOR_NUMBER   501
#define BUZZER_DEV_NAME      "buzzer_dev"

#define GPIO_BASE_ADDR 0x3F200000
#define GPFSEL1 	0x04
#define GPSET1   	0x1C
#define GPCLR1   	0x28
#define GPLEV1   	0x34

static void __iomem *gpio_base;
volatile unsigned int* gpsel1;
volatile unsigned int* gpset1;
volatile unsigned int* gpclr1;

int buzzer_open(struct inode* inode, struct file* filp) {
   printk(KERN_ALERT "buzzer open function called\n");
   
   gpio_base = ioremap(GPIO_BASE_ADDR, 0x60);
   gpsel1 = (volatile unsigned int*)(gpio_base + GPFSEL1);
   gpset1 = (volatile unsigned int*)(gpio_base + GPSET1);
   gpclr1 = (volatile unsigned int*)(gpio_base + GPCLR1);
   
   return 0;
}
   
ssize_t buzzer_write(struct file *filp, const char* buf, size_t count, loff_t *f_pos) {
   printk(KERN_ALERT "buzzer write function called\n");
   
   *gpsel1 |= (1<<24);

   int data = 0;
   copy_from_user(&data, buf, 4);   
   printk(KERN_INFO "%d\n", data);

   unsigned int duration = 500;

   long bdelay = (long)(1000000/data);
   long time = (long)(duration*1000)/(bdelay*2);

   printk(KERN_ALERT "%ld %ld\n", bdelay, time);

   int i;
   for(i = 0; i < time*2; i++) {
      *gpset1 |= (1<<18);
      udelay(bdelay);
      *gpclr1 |= (1<<18);
      udelay(bdelay);
   }
   
   //*gpsel1 |=(0<<24);
   
   
   return count;
}

int buzzer_release(struct inode* inode, struct file* filp) {
   printk(KERN_ALERT "buzzer device driver closed!!\n");
   iounmap((void*)gpio_base);
   return 0;
}

static struct file_operations buzzer_fops = {
   .owner = THIS_MODULE,
   .open = buzzer_open,
   .release = buzzer_release,
   .write = buzzer_write
};

int __init buzzer_init(void) {
   if(register_chrdev(BUZZER_MAJOR_NUMBER, BUZZER_DEV_NAME, &buzzer_fops) < 0 )
      printk(KERN_ALERT "[buzzer] driver init failed\n");
   else
      printk(KERN_ALERT "[buzzer] driver init successful\n");

   return 0;
}

void __exit buzzer_exit(void) {
   unregister_chrdev(BUZZER_MAJOR_NUMBER, BUZZER_DEV_NAME);
   printk(KERN_ALERT "[buzzer] driver cleanup");
}

module_init(buzzer_init);
module_exit(buzzer_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("hyunjun cho");
MODULE_DESCRIPTION("BUZZER_DEVICE_DRIVER");

