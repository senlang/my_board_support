#include <linux/kernel.h>                                                                     
#include <linux/module.h>                                                                  
#include <linux/fs.h>                                                                      
#include <linux/slab.h>                                                                    
#include <linux/device.h>                                                                  
                                                                                           
#include <asm/io.h>                                                                        
#include <asm/uaccess.h>                                                                   
                                                                                           
                                                                                           
#include "led_demo.h"                                                                      
                                                                                           
                                                                                           
MODULE_LICENSE("GPL");                                                                     
                                                                                           
                                                                                           
static int led_demo_major;                                                                 
static int led_demo_minor;                                                                 
static int number_of_dev = 1;                                                              
                                                                                           
static struct led_demo_dev  *led_dev = NULL;                                               
                                                                                           
static unsigned int *GPM4CON = NULL;                                                       
static unsigned int *GPM4DAT = NULL;                                                       
                                                                                           
static struct class *led_demo_class = NULL;                                                
                                                                                           
                                                                                           
static int led_open (struct inode *node, struct file *fops)                                
{                                                                                          
    struct led_demo_dev *dev;                                                              
                                                                                           
    dev = container_of(node->i_cdev, struct led_demo_dev, dev);                            
                                                                                           
    fops->private_data = dev;                                                              
                                                                                           
    return 0;                                                                              
}                                                                                          
static int led_close (struct inode *node, struct file *fops)                               
{                                                                                          
    return 0;                                                                              
}                                                                                          
                                                                                           
static long led_ioctl (struct file *fops, unsigned int cmd, unsigned long data)            
{                                                                                          
    //struct led_demo_dev * led_dev = (struct led_demo_dev *)fops->private_data;           
                                                                                           
    if((data < 1) || (data > 4))                                                           
    {                                                                                      
        printk(KERN_ALERT"parameter is no valid.\n");                                      
        return -EINVAL;                                                                    
    }                                                                                      
                                                                                           
    switch (cmd)                                                                           
    {                                                                                      
        case LED_OFF:                                                                      
            writel(readl(GPM4DAT) | (0x1<<(data-1)), GPM4DAT);                             
            break;                                                                         
        case LED_ON:                                                                       
            writel(readl(GPM4DAT) & ~(0x1<<(data-1)), GPM4DAT);                            
            break;                                                                         
        default:                                                                           
            return -EINVAL;                                                                
            break;                                                                         
    }                                                                                      
                                                                                           
                                                                                           
    return 0;                                                                              
}                                                                                          
                                                                                           
struct file_operations led_fops =                                                          
{                                                                                          
    .owner = THIS_MODULE,                                                                  
    .open = led_open,                                                                      
    .unlocked_ioctl = led_ioctl,                                                           
    .compat_ioctl = led_ioctl,                                                             
    .release = led_close,                                                                  
};                                                                                         
                                                                                           
static int __led_setup_dev(struct led_demo_dev * dev)                                      
{                                                                                          
    int err = -1;                                                                          
                                                                                           
    dev_t devno = MKDEV(led_demo_major, led_demo_minor);                                   
                                                                                           
    memset(dev, 0, sizeof(struct led_demo_dev));                                           
                                                                                           
    cdev_init(&(dev->dev), &led_fops);                                                     
                                                                                           
    dev->dev.owner = THIS_MODULE;                                                          
                                                                                           
    err = cdev_add(&(dev->dev), devno, number_of_dev);                                     
    if(err < 0)                                                                            
    {                                                                                      
        return err;                                                                        
    }                                                                                      
                                                                                           
    return 0;                                                                              
}                                                                                          
                                                                                           
static int led_demo_init(void)                                                             
{                                                                                          
    int err = -1;                                                                          
    dev_t dev;                                                                             
    struct device *temp = NULL;                                                            
                                                                                           
    printk(KERN_ALERT"Initializing led demo device.\n");                                   
                                                                                           
    err = alloc_chrdev_region(&dev, 0, number_of_dev, LED_DEMO_DEVICE_NODE_NAME);          
    if(err < 0)                                                                            
    {                                                                                      
        printk(KERN_ALERT"fail to alloc char dev region.\n");                              
        goto fail;                                                                         
    }                                                                                      
                                                                                           
    led_demo_major = MAJOR(dev);                                                           
    led_demo_minor = MINOR(dev);                                                           
                                                                                           
    led_dev = kmalloc(sizeof(struct led_demo_dev), GFP_KERNEL);                            
    if(!led_dev)                                                                           
    {                                                                                      
        err = -ENOMEM;                                                                     
        printk(KERN_ALERT"Failed to alloc led device.\n");                                 
        goto unregister;                                                                   
    }                                                                                      
                                                                                           
    err = __led_setup_dev(led_dev);                                                        
    if (err < 0)                                                                           
    {                                                                                      
        printk(KERN_ALERT"Failed to setup led device.\n");                                 
        goto clean_up;                                                                     
    }                                                                                      
                                                                                           
    GPM4CON = (unsigned int *)ioremap(EXYNOS4412_GPM4CON, 4);                              
    if(!GPM4CON)                                                                           
    {                                                                                      
        err = -ENOMEM;                                                                     
        goto destroy_cdev;                                                                 
    }                                                                                      
                                                                                           
    GPM4DAT = (unsigned int *)ioremap(EXYNOS4412_GPM4DAT, 4);                              
    if(!GPM4DAT)                                                                           
    {                                                                                      
        err = -ENOMEM;                                                                     
        goto unmap1;                                                                       
    }                                                                                      
                                                                                           
    writel((readl(GPM4CON) & ~0xffff) | 0x1111, GPM4CON);                                  
    writel(readl(GPM4DAT)| 0xff, GPM4DAT);   
    writel(readl(GPM4DAT) & 0xfa, GPM4DAT);  
	
	printk("readl(GPM4DAT) = 0x%x\n",readl(GPM4DAT));
	
    writel(readl(GPM4DAT)| 0xff, GPM4DAT);   
    writel(readl(GPM4DAT) & 0xfa, GPM4DAT);   
	
	printk("readl(GPM4DAT) = 0x%x\n",readl(GPM4DAT));

	
                                                                                           
    led_demo_class = class_create(THIS_MODULE, LED_DEMO_DEVICE_CLASS_NAME);                
    if(IS_ERR(led_demo_class))                                                             
    {                                                                                      
        err = PTR_ERR(led_demo_class);                                                     
        printk(KERN_ALERT"Failed to create led demo class.\n");                            
        goto unmap2;                                                                       
    }                                                                                      
                                                                                           
    temp = device_create(led_demo_class, NULL, dev, NULL, "%s", LED_DEMO_DEVICE_FILE_NAME);
    if(IS_ERR(temp))                                                                       
    {                                                                                      
        err = PTR_ERR(temp);                                                               
        printk(KERN_ALERT"Failed to create led demo device.\n");                           
        goto destroy_class;                                                                
    }                                                                                      
                                                                                           
    dev_set_drvdata(temp, (void *)led_dev);                                                
                                                                                           
    printk(KERN_ALERT"Succeed to initialize led demo device.\n");                          
                                                                                           
    return 0;                                                                              
                                                                                           
destroy_class:                                                                             
    class_destroy(led_demo_class);                                                         
                                                                                           
unmap2:                                                                                    
    iounmap(GPM4DAT);                                                                      
                                                                                           
unmap1:                                                                                    
    iounmap(GPM4CON);                                                                      
                                                                                           
destroy_cdev:                                                                              
    cdev_del(&(led_dev->dev));                                                             
                                                                                           
clean_up:                                                                                  
    kfree(led_dev);                                                                        
                                                                                           
unregister:                                                                                
    unregister_chrdev_region(MKDEV(led_demo_major, led_demo_minor), number_of_dev);        
                                                                                           
fail:                                                                                      
                                                                                           
    return err;                                                                            
}                                                                                          
                                                                                           
static void led_demo_exit(void)                                                            
{                                                                                          
    if(led_demo_class)                                                                     
    {                                                                                      
        device_destroy(led_demo_class, MKDEV(led_demo_major, led_demo_minor));             
        class_destroy(led_demo_class);                                                     
    }                                                                                      
                                                                                           
    iounmap(GPM4DAT);                                                                      
    iounmap(GPM4CON);                                                                      
                                                                                           
    if(led_dev)                                                                            
    {                                                                                      
        cdev_del(&(led_dev->dev));                                                         
        kfree(led_dev);                                                                    
    }                                                                                      
                                                                                           
    unregister_chrdev_region(MKDEV(led_demo_major, led_demo_minor), number_of_dev);        
}                                                                                          
                                                                                           
                                                                                           
                                                                                           
module_init(led_demo_init);                                                                
module_exit(led_demo_exit);                                                                
