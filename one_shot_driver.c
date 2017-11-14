#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/mutex.h>
#include <linux/cdev.h>
#include <asm-generic/uaccess.h>
#include <asm/delay.h>
#include <linux/errno.h>
#include <linux/string.h>

#include <plat/dmtimer.h>
#include "one_shot_timer.h"
#include "am335x.h"
#include "gpio_func.h"

#define OSP_MAJOR	120

static DEFINE_MUTEX(osp_drv_mutex);

static int sg_osp_status;

#define OSP_IS_OPEN	0x00000001

static int sg_use_timer_num;
static int sg_pulse_width_count;
static struct cdev sg_cdev;
static struct class * osp_driver_class = NULL;
static dev_t g_osp_dev;
#define DEFAULT_TIMER_NUM	4
#define DEFAULT_WIDTH_COUNT	10

static int osp_open(struct inode *inode, struct file *file)
{
        mutex_lock(&osp_drv_mutex);
        if (sg_osp_status & OSP_IS_OPEN) {
                mutex_unlock(&osp_drv_mutex);
                return -EBUSY;
        }

        sg_osp_status |= OSP_IS_OPEN;
        mutex_unlock(&osp_drv_mutex);

	pr_info("%s done.\n",__FUNCTION__);
	return 0;
}

static int osp_release(struct inode *unused, struct file *file)
{
        mutex_lock(&osp_drv_mutex);
        sg_osp_status &= ~OSP_IS_OPEN;
        mutex_unlock(&osp_drv_mutex);

	pr_info("%s done.\n",__FUNCTION__);
	return 0;
}

static ssize_t set_osp_config(struct file *file, const char __user *buf,
                        size_t count, loff_t *ppos)
{
	struct omap_dm_timer *dmtimer_osp;
	char set_msg[32];
	int length,d_size;
	int tmp,cnt;

	if (count > 32) {
		d_size = 32;
	} else {
		d_size = count;
	}
	length = copy_from_user(set_msg, buf, d_size);
	length = d_size - length;

	if ((set_msg[0] == 'p') || (set_msg[0] == 'P')) {
		pr_info("%s: Set Timer number.\n",__FUNCTION__);
		tmp = set_msg[1] - '0';
		if ( (tmp < 4) || (7 < tmp) ) {
			pr_err("%s: Invalid Param - out of range!!\n",__FUNCTION__);
			return -EINVAL;
		} else {
			pr_info("%s: Timer number %d is selected.\n",__FUNCTION__,tmp);
			omap_cm_timer_module_enabled(sg_use_timer_num, 0);
			omap_cm_timer_module_enabled(tmp, 1);
			sg_use_timer_num = tmp;
		}
	}

	if ((set_msg[0] == 'c') || (set_msg[0] == 'C')) {
		pr_info("%s: Set Pulse width count.\n",__FUNCTION__);
		tmp = 0;
		for ( cnt = 1;cnt < length-1;cnt++) {
			tmp = tmp * 10 + (set_msg[cnt] - '0');
			pr_info("%s: tmp(%d).\n",__FUNCTION__,tmp);
		}
		pr_info("%s: Pulse width count is %d.\n",__FUNCTION__,tmp);
		sg_pulse_width_count = tmp;
	}
	
	if ((set_msg[0] == 's') || (set_msg[0] == 'S')) {
		pr_info("%s: Stop one shot pulse function.\n",__FUNCTION__);
		dmtimer_osp = omap_dm_timer_request_specific(sg_use_timer_num);
		omap_dm_timer_stop_oneshot(dmtimer_osp);
		omap_dm_timer_free(dmtimer_osp);
	}
	
	pr_info("%s done.\n",__FUNCTION__);
	return length;
}

static ssize_t issue_one_shot_pulse(struct file *file, char __user *buf,
                        size_t count, loff_t *ppos)
{
	struct omap_dm_timer *dmtimer_osp;

	pr_info("%s: create OneShotPulse to Timer-%d with width of 25ns x %d.\n",
		__FUNCTION__,sg_use_timer_num,sg_pulse_width_count);

	dmtimer_osp = omap_dm_timer_request_specific(sg_use_timer_num);
        if (!dmtimer_osp) {
                pr_err("%s: Error getting Timer resource for OneShotPulse.\n",__FUNCTION__);
                return -ENODEV;
        }
        GPIO_DATA gpio;
        gpio.GPIO_BASE_NUM = 1;
        gpio.GPIO_NUM = 13;
        gpio.GPIO_MODE = 7;
        INT32 oe1 = ioread32(g_gpio_vbase[1]+GPIO_OE);
        oe1 = oe1 & ~(1 << gpio.GPIO_NUM);
        iowrite32(oe1,g_gpio_vbase[1]+GPIO_OE);
        omap_dm_timer_prepare_oneshot(dmtimer_osp,10);
    	omap_dm_timer_cont_prepare(dmtimer_osp,10);
        gpio_iowrite32f(gpio,GPIO_SETDATAOUT);
       // gpio_iowrite32f(gpio,GPIO_CLEARDATAOUT);
	if ( omap_dm_timer_do_oneshot(dmtimer_osp) ) {
		copy_to_user(buf,"NG",2);
		pr_err("%s: Error fail to create OneShotPulse !!\n",__FUNCTION__);
	} else {
		//copy_to_user(buf,"OK",2);
		//pr_info("%s: Success Normally done.\n",__FUNCTION__);
	}
	udelay(1);
	gpio_iowrite32f(gpio,GPIO_CLEARDATAOUT);
	gpio_iowrite32f(gpio,GPIO_CLEARDATAOUT);
	 gpio_iowrite32f(gpio,GPIO_SETDATAOUT);
	        gpio_iowrite32f(gpio,GPIO_CLEARDATAOUT);
	        omap_dm_timer_cont_prepare(dmtimer_osp,10);
	        if ( omap_dm_timer_do_oneshot(dmtimer_osp) ) {
	        		copy_to_user(buf,"NG",2);
	        		pr_err("%s: Error fail to create OneShotPulse !!\n",__FUNCTION__);
	        	} else {
	        		copy_to_user(buf,"OK",2);
	        		pr_info("%s: Success Normally done.\n",__FUNCTION__);
	        	}
	omap_dm_timer_free(dmtimer_osp);
	return 2;
}

static const struct file_operations osp_device_fops = {
        .owner          = THIS_MODULE,
        .read           = issue_one_shot_pulse,
        .write          = set_osp_config,
        .open           = osp_open,
        .release        = osp_release,
};

static int __init one_shot_pulse_init(void)
{

	int result;
     /*if (register_chrdev(OSP_MAJOR, "one shot pulse", &osp_device_fops) ){
         pr_err("%s : register_chrdev failed.\n",__FUNCTION__);
         return -EBUSY;
     } 
	*/
     sg_osp_status = 0; 
     sg_use_timer_num = DEFAULT_TIMER_NUM;
     sg_pulse_width_count = DEFAULT_WIDTH_COUNT;

     omap_cm_timer_module_enabled(sg_use_timer_num, 1);

     cdev_init(&sg_cdev,&osp_device_fops);
     	sg_cdev.owner = THIS_MODULE;
     	sg_cdev.ops = &osp_device_fops;
     	result = cdev_add(&sg_cdev,MKDEV(OSP_MAJOR,0),1);
     	if(result)
     	{
     		cdev_del(&sg_cdev);
     		return -1;
     	}
     	osp_driver_class = class_create(THIS_MODULE, "osp");
     	g_osp_dev = MKDEV(OSP_MAJOR, 0);
     	device_create(
     			osp_driver_class,
     				NULL,
     				g_osp_dev,
     				NULL,
     				"osp",
     				0);


     pr_info("one_shot_pulse_init done normally.\n");
     gpio_vbase_init(OSP_MAJOR);
     return 0;
}

static void __exit one_shot_pulse_exit(void)
{
     omap_cm_timer_module_enabled(sg_use_timer_num, 0);

 	cdev_del(&sg_cdev);
 	device_destroy(osp_driver_class, g_osp_dev);
 	class_destroy(osp_driver_class);
 	 unregister_chrdev(OSP_MAJOR, "one shot pulse");
     gpio_vbase_release();
     pr_info("one_shot_pulse_exit -- oneshot_pulse_dev removed.\n");
}

module_init(one_shot_pulse_init);
module_exit(one_shot_pulse_exit);

MODULE_AUTHOR("Plecolo");
MODULE_DESCRIPTION("One-Shot Pulse Driver using dm_timer.");
MODULE_LICENSE("GPL");

