#include "gpio_func.h"

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/string.h>
#include <asm/uaccess.h>
INT32 inline get_mask_bit(int gpio_number)
{
    return (1 << (gpio_number));
}
volatile void __iomem * g_gpio_vbase[3];
INT32 inline getGpioVbase(int num)
{
	return (INT32)g_gpio_vbase[num];
}
const static INT32 g_gpio0_start = AM33XX_GPIO0_BASE;
const static INT32 g_gpio1_start = AM33XX_GPIO1_BASE;
const static INT32 g_gpio2_start = AM33XX_GPIO2_BASE;
const static INT32 g_gpio1_len = GPIO_LEN;		// Length of GPIO1 addresses
void gpio_vbase_release(void)
{

	// -3. Unmap ioaddress
	iounmap((volatile void *)g_gpio_vbase[1]);

	// -2. Release memory region
	release_mem_region (g_gpio1_start, g_gpio1_len);

	// ` for gpio2
	iounmap((volatile void *)g_gpio_vbase[2]);
	release_mem_region(g_gpio2_start, g_gpio1_len);
	// for gpio0
	iounmap((volatile void *)g_gpio_vbase[0]);
	release_mem_region(g_gpio0_start,g_gpio1_len);

}
int gpio_vbase_init(int DRIVER_MAJOR)
{
	// 2. Check memory region and then request
	/*int result;
			result = check_mem_region(g_gpio0_start, g_gpio1_len);
			if (result < 0) {
				printk("Allocation for I/O memory range failed0\n");
				return(result);
			}*/
/*			if(!request_mem_region (g_gpio0_start, g_gpio1_len, "gpio0"))
			{
				printk("Allocation for I/O memory range failed0\n");
				return -1;
			}
*/
		// 3. Physical to virtual memory mapping

			g_gpio_vbase[0] = ioremap(g_gpio0_start, g_gpio1_len);
			if (!g_gpio_vbase[0]) {
				printk("Ioremap failed for GPIO0\n");
				// -2. Release memory region
				release_mem_region (g_gpio0_start, g_gpio1_len);
				// -1. Unregister gpio-led device
				//unregister_chrdev( DRIVER_MAJOR, "inkhead-driver" );
				return(-2);
			}


		// 2. Check memory region and then request
		/*result = check_mem_region(g_gpio1_start, g_gpio1_len);
		if (result < 0) {
			printk("Allocation for I/O memory range failed1\n");
			return(result);
		}*/
	/*	if(!request_mem_region (g_gpio1_start, g_gpio1_len, "gpio1"))
		{
			printk("Allocation for I/O memory range failed1\n");
			return -1;
		}
*/
		// 3. Physical to virtual memory mapping

		g_gpio_vbase[1] = ioremap(g_gpio1_start, g_gpio1_len);
		if (!g_gpio_vbase[1]) {
			printk("Ioremap failed for GPIO1\n");
			// -2. Release memory region
			release_mem_region (g_gpio1_start, g_gpio1_len);
			// -1. Unregister gpio-led device
			unregister_chrdev( DRIVER_MAJOR, "inkhead-driver" );
			return(-2);
		}
		// 2. Check memory region and then request
			/*result = check_mem_region(g_gpio2_start, g_gpio1_len);
			if (result < 0) {
				printk("Allocation for I/O memory range failed2\n");
				return(result);
			}*/
	/*		if(!request_mem_region (g_gpio2_start, g_gpio1_len, "gpio2"))
			{
				printk("Allocation for I/O memory range failed2\n");
				return -1;
			}
*/
		g_gpio_vbase[2] = ioremap(g_gpio2_start, g_gpio1_len);
		if(!g_gpio_vbase[2])
		{
			printk("ioreamp failed for GPIO2\n");
			release_mem_region(g_gpio2_start,g_gpio1_len);

			unregister_chrdev( DRIVER_MAJOR,"inkhead-driver");
			return (-2);
		}
		return 1;
}
