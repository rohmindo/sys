#include <linux/init.h> 
#include <linux/kernel.h> 
#include <linux/module.h> 
#include <linux/fs.h> 
#include <linux/uaccess.h> 
#include <linux/slab.h> 
#include <linux/delay.h>

#include <asm/mach/map.h> 
#include <asm/uaccess.h> 

typedef struct{
   unsigned int hydro_integer;
   unsigned int hydro_float;
   unsigned int temp_integer;
   unsigned int temp_float;
}Data;

#define HIGH                    1   
#define LOW                     0

#define HYDRO_MAJOR_NUMBER   505
#define HYDRO_DEV_NAME         "hydro_dev" 

#define IOCTL_HYDRO_MAGIC_NUMBER    'h'
#define IOCTL_CMD_HYDRO      _IOR(IOCTL_HYDRO_MAGIC_NUMBER, 0, Data)

#define GPIO_BASE_ADDRESS       0x3F200000

#define GPFSEL0                  0x00
#define GPFSEL1			0x04
#define GPSET1                  0x1C
#define GPCLR1                  0x28
#define GPLEV1                  0x34

static void __iomem* gpio_base;

volatile unsigned int* gpsel0;
volatile unsigned int* gpsel1;
volatile unsigned int* gpset1;
volatile unsigned int* gpclr1;
volatile unsigned int* gplev1;

int hydro_open(struct inode * inode, struct file * filp){
    printk(KERN_ALERT "\nhumidity driver open\n"); 
    gpio_base = ioremap(GPIO_BASE_ADDRESS, 0xFF);
    
    gpsel0 = (volatile unsigned int*)(gpio_base + GPFSEL0);
    gpsel1 = (volatile unsigned int*)(gpio_base + GPFSEL1);
    gpset1 = (volatile unsigned int*)(gpio_base + GPSET1);
    gpclr1 = (volatile unsigned int*)(gpio_base + GPCLR1);
    gplev1 = (volatile unsigned int*)(gpio_base + GPLEV1);
/*
    *gpsel0 |= (1<<26);
    *gpsel0 |= (1<<29);
    *gpsel1 |= (1<<2);
    *gpsel1 |= (1<<5);*/

    return 0; 
}

int hydro_release(struct inode * inode, struct file * filp) { 
    printk(KERN_ALERT "humidity driver closed\n\n"); 
    iounmap((void *)gpio_base); 
    return 0; 
}


long hydro_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{ 
    Data data;
    int result[5] = {0};
    int index = 0;
   int pre_data = HIGH;
   int counter = 0;
    int i = 0;
    switch (cmd){ 
        case IOCTL_CMD_HYDRO: 

            *gpsel0 |= (1 << 12);       //GPIO 4 SET OUTPUT MODE

            *gpset1 |= (1 << 4);        //GPIO 4 SET HIGH
            mdelay(800);             //SLEEP FOR 800 miliseconds to synchronize bits from loop

            *gpclr1 |= (1 << 4);        //GPIO 4 SET LOW
            mdelay(18);              //SLEEP FOR 18 miliseconds

            *gpset1 |= (1 << 4);        //GPIO 4 SET HIGH
            udelay(30);             //SLEEP FOR 30 microseconds

           *gpsel0 &=  ~(1 << 12);        //GPIO 4 SET INPUT MODE

            //except first 3(wait, response, ready) signals
            for(i = 0; i < 3; i++){
                if(((*gplev1 >> 4) & 0x01) == LOW){
                    while(((*gplev1 >> 4) & 0x01) == LOW){} continue;
                }
                if(((*gplev1 >> 4) & 0x01) == HIGH){
                    while(((*gplev1 >> 4) & 0x01) == HIGH){} continue;
                }
            }
            //interpret '0' and '1' bits
           while(index < 40){
                if(((*gplev1 >> 4) & 0x01) == LOW) continue;
              counter = 0;
              while(((*gplev1 >> 4) & 0x01) == HIGH){
                 counter++;
                    udelay(1);
              }
              if(counter > 80) break;
             result[index / 8] <<= 1;
             if(counter > 28) result[index / 8] |= 1;
                index++;
           }
            //printk(KERN_INFO "data[0]: %x data[1]: %x data[2]: %x data[3]: %x data[4]: %x\n", data[0], data[1], data[2], data[3], data[4]); 
            if(result[4] != ((result[0] + result[1] + result[2] + result[3]) & 0xFF))
                break;
            data.hydro_integer = result[0];
            data.hydro_float = result[1];
            data.temp_integer = result[2];
            data.temp_float = result[3];
            //printk(KERN_INFO "humidity: %d.%d%% temperature: %d.%d%%\n", info.humidity_integer, info.humidity_float, info.temperature_integer, info.temperature_float);

            copy_to_user((void*)arg, (void*)&data, sizeof(data));
	   printk(KERN_ALERT "hydro done\n"); 
          break; 

        default : 
            printk(KERN_ALERT "ioctl : command error\n");
    }
    
    return 0; 
}

static struct file_operations hydro_fops = { 
    .owner = THIS_MODULE,  
    .open = hydro_open, 
    .release = hydro_release,
    .unlocked_ioctl = hydro_ioctl
}; 

int __init hydro_init (void) { 
    if(register_chrdev(HYDRO_MAJOR_NUMBER, HYDRO_DEV_NAME, &hydro_fops) < 0)
        printk(KERN_ALERT "hydro driver initalization failed\n"); 
    else 
        printk(KERN_ALERT "hydro driver initalization succeed\n");
    return 0; 
}

void __exit hydro_exit(void){ 
    unregister_chrdev(HYDRO_MAJOR_NUMBER, HYDRO_DEV_NAME); 
    printk(KERN_ALERT "hydro driver exit"); 
}

module_init(hydro_init); 
module_exit(hydro_exit);  

MODULE_LICENSE("GPL");
MODULE_AUTHOR("joreka"); 
MODULE_DESCRIPTION("hydro_dev");

