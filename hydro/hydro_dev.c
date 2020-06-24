#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/delay.h>

#include <asm/mach/map.h>
#include <asm/io.h>
#include <asm/uaccess.h>

typedef struct {
   unsigned int tempi;
   unsigned int tempf;
   unsigned int hydroi;
   unsigned int hydrof;
}Data;

#define HYDRO_MAJOR_NUMBER 505
#define HYDRO_DEV_NAME "hydro_dev"

#define GPIO_BASE_ADDR 0x3F200000
#define GPFSEL0 0x00
#define GPFSEL1 0x04
#define GPSET0    0x1C
#define GPCLR0    0x28
#define GPLEV0   0x34

#define HIGH    1
#define LOW   0

static void __iomem *gpio_base;
volatile unsigned int *gpsel0;
volatile unsigned int *gpsel1;
volatile unsigned int *gpset1;
volatile unsigned int *gpclr1;
volatile unsigned int *gplev1;

int hydro_open(struct inode *inode,struct file *flip){
   printk(KERN_ALERT "HYDRO driver open!!\n");
   
   gpio_base = ioremap(GPIO_BASE_ADDR, 0x60);
   gpsel0 = (volatile unsigned int *)(gpio_base + GPFSEL0);
   gpsel1 = (volatile unsigned int *)(gpio_base + GPFSEL1);
   gpset1 = (volatile unsigned int *)(gpio_base + GPSET0);
   gpclr1 = (volatile unsigned int *)(gpio_base + GPCLR0);
   gplev1 = (volatile unsigned int *)(gpio_base + GPLEV0);

   *gpsel0 |= (1<<26);
   *gpsel0 |= (1<<29);
   *gpsel1 |= (1<<2);
   *gpsel1 |= (1<<5);
   
   return 0;
}

int hydro_release(struct inode *inode,struct file *flip){
   printk(KERN_ALERT "HYDRO driver closed!!\n");
   
   iounmap((void *)gpio_base);
   
   return 0;
}

ssize_t hydro_read(struct file *filp, char* buf, size_t counts, loff_t *f_pos) {
   printk(KERN_ALERT "read function called!!\n");
   Data data;
   int result[5] = {0, };
   int time = 0;
   int prev = HIGH;
   int index = 0;
   int i = 0;

   *gpsel0 |= (1<<12);

   *gpset1 |= (1<<4);
   mdelay(800);
   *gpclr1 |= (1<<4);
   mdelay(18);
   *gpset1 |= (1<<4);
   udelay(30);

   *gpsel0 = (0<<24);

   for(i = 0; i < 3; i++) {
      if(((*gplev1 >> 4) & HIGH) == LOW) {
         while(((*gplev1 >> 4) & HIGH) == LOW){}
         continue;
      }
      if(((*gplev1 >> 4) & HIGH) == HIGH) {
         while(((*gplev1 >> 4) & HIGH) ==HIGH){}
         continue;
      }
   }
   //int status = LOW;
   while(index < 40) {
      if(((*gplev1 >> 4) & 0x01) == LOW)
         continue;
      time = 0;
      while(((*gplev1 >> 4) & 0x01) == HIGH){
         time++;
         udelay(1);
      }
      if(time > 70)
         break;
      result[index / 8] <<= 1;
      if(time > 28)
         result[index/8] |= 1;
      time++;
   }
   int wrong = -1;
   printk(KERN_ALERT "%d %d %d %d\n", result[0], result[1], result[2], result[3]);

   if((result[4] == (result[0] + result[1] + result[2] + result[3])& 0xFF) && result[0] != 0) {
      data.hydroi = result[0];
      data.hydrof = result[1];
      data.tempi = result[2];
      data.tempf = result[3];
      copy_to_user(buf, (const void*)&data, sizeof(data));
   }
   else
      copy_to_user(buf, &wrong, sizeof(int));

   return counts;
}
         
static struct file_operations hydro_fops={
   .owner = THIS_MODULE,
   .open = hydro_open,
   .release = hydro_release,
   .read = hydro_read
};

int __init hydro_init(void){
   if(register_chrdev(HYDRO_MAJOR_NUMBER, HYDRO_DEV_NAME, &hydro_fops)<0)
      printk(KERN_ALERT "HYDRO driver initialization fail\n");
   else
      printk(KERN_ALERT "HYDRO driver intialization success\n");
   
   return 0;
}

void __exit hydro_exit(void){
   unregister_chrdev(HYDRO_MAJOR_NUMBER, HYDRO_DEV_NAME);
   printk(KERN_ALERT "HYDRO driver exit doen\n");
}

module_init(hydro_init);
module_exit(hydro_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("joreka");
MODULE_DESCRIPTION("HYDRO_DEV");
   
   

