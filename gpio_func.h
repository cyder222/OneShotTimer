#ifndef __GPIO_FUNC
#define __GPIO_FUNC

#include <asm/io.h>
//#include <plat/am33xx.h>
#include "am335x.h"
//#include <plat/gpio.h>
#include <asm/gpio.h>
#include "type.h"
#define MAX_BUFLEN 64
#define GPIO_LEN 0x100

extern volatile void __iomem * g_gpio_vbase[3];
int gpio_vbase_init(int major_number);
void gpio_vbase_release(void);
INT32 getGpioVbase(int num);// 0 1 2

static const unsigned long GPIO_BASE[] = {
	GPIO0,
	GPIO1,
	GPIO2,
	GPIO3,
};
/* 一部のレジスタオフセット */
#define OUTPUT_ENABLE  GPIO_OE
#define DATA_IN        GPIO_DATAIN
#define DATA_OUT       GPIO_DATAOUT
#define SET_DATA_OUT   GPIO_SETDATAOUT
#define CLEAR_DATA_OUT GPIO_CLEARDATAOUT

/* operation */
#define IN          (0)
#define OUT         (1)
#define CLEAR       (2)
#define SET         (3)
#define OUT_ENABLE  (4)
typedef struct GPIO_DATA{
	unsigned int GPIO_BASE_NUM;
	unsigned int GPIO_NUM;
	unsigned int GPIO_MODE;
} GPIO_DATA;

unsigned long inline get_mask_bit(int gpio_number);

#define gpio_iowrite32(GPIO_DATA,FLUG) iowrite32(get_mask_bit(GPIO_DATA.GPIO_NUM),g_gpio_vbase[GPIO_DATA.GPIO_BASE_NUM]+FLUG)
#define gpio_ioread32(GPIO_DATA,FLUG) ioread32(g_gpio_vbase[GPIO_DATA.GPIO_BASE_NUM]+FLUG)
#define gpio_iowrite32f(GPIO_DATA,FLUG) __raw_writel(get_mask_bit(GPIO_DATA.GPIO_NUM),g_gpio_vbase[GPIO_DATA.GPIO_BASE_NUM]+FLUG) //fast io_write not detect for mmio or ioport
#define gpio_ioread32f( GPIO_DATA,FLUG) __raw_readl(g_gpio_vbase[GPIO_DATA.GPIO_BASE_NUM]+FLUG) // fast io_read not detect mmio or ioport
/*
static unsigned long *get_reg_address(int gpio_number,int operation)
{
    unsigned long p;

    switch (operation) {
    case IN:
        p = GPIO_BASE[gpio_number >> 5] + DATA_IN;
        break;
    case OUT:
        p = GPIO_BASE[gpio_number >> 5] + DATA_OUT;
        break;
    case SET:
        p = GPIO_BASE[gpio_number >> 5] + SET_DATA_OUT;
        break;
    case CLEAR:
        p = GPIO_BASE[gpio_number >> 5] + CLEAR_DATA_OUT;
        break;
    case OUT_ENABLE:
        p = GPIO_BASE[gpio_number >> 5] + OUTPUT_ENABLE;
        break;
    default:
        p = GPIO_BASE[gpio_number >> 5] + DATA_IN;
        break;
    }
    return ioremap(p,GPIO_LEN);
}
*/



#endif
