#ifndef __LED_DEMO_H__
#define __LED_DEMO_H__
 
#include <linux/cdev.h>
 
#define LED_ON    _IOW('L', 0, int)
#define LED_OFF    _IOW('L', 1, int)
 
#define LED_DEMO_DEVICE_NODE_NAME  "led_demo"
#define LED_DEMO_DEVICE_CLASS_NAME "led_demo"
#define LED_DEMO_DEVICE_FILE_NAME  "led_demo"
 
#define EXYNOS4412_GPM4CON    0x110002E0
#define EXYNOS4412_GPM4DAT    0x110002E4
 
 
struct led_demo_dev
{
    struct cdev dev;
};
 
#endif