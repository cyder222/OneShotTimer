
#include "one_shot_timer.h"
#include <plat/dmtimer.h>
#include "am335x.h"
#include <linux/delay.h>

static const int timer_clkctrl_offset[8] = {
	0x00,   /* none   */
	0x00,	/* none   */
	0x80,	/* timer2 */
	0x84,	/* timer3 */
	0x88,	/* timer4 */
	0xec,	/* timer5 */
	0xf0,	/* timer6 */
	0x7c,	/* timer7 */
};

/**
 * @param count // 何カウント分パルスを発生させるか
**/
int omap_dm_timer_create_oneshot(struct omap_dm_timer * timer_ptr,unsigned int count)
{
	int ret;
	unsigned int reg_data;
	void __iomem* cm_per_base;
	int loopcnt;

	if (count < 1 || 0xfffffffe < count) {
		pr_err("%s: The range of count is invalid.\n",__FUNCTION__);
		return -1;
	}

	cm_per_base = ioremap(AM33XX_PRCM_BASE, 0x100);

//	pr_info("%s called.\n",__FUNCTION__);
	omap_dm_timer_stop(timer_ptr);


/*      start               load                match
                              ---------------------
        ______________________|                   |__________________
        0xffffffff - 100      0                 count -1 
*/

	ret = omap_dm_timer_set_pwm(timer_ptr,
				0,	/* default level=low 	*/
				1,	/* toggle mode		*/
				0 );
	if ( ret ) {
		pr_err("%s: Failed on omap_dm_timer_set_pwm().\n",__FUNCTION__);
		omap_dm_timer_disable(timer_ptr);
		iounmap(cm_per_base);
		return -1;
	} 
	
	ret = omap_dm_timer_set_load(timer_ptr, 1 /* one shot */, (0xffffffff - 2));
	omap_dm_timer_enable(timer_ptr);
	omap_dm_timer_trigger(timer_ptr);
	for (loopcnt=0;loopcnt<10;loopcnt++) {
		reg_data = __raw_readl(timer_ptr->func_base + (OMAP_TIMER_WRITE_PEND_REG & 0xff));
		if (reg_data == 0)
			break;
		udelay(10);
	}
	ret = omap_dm_timer_set_pwm(timer_ptr,
				0,	/* default level=low 	*/
				1,	/* toggle mode		*/
				OMAP_TIMER_TRIGGER_OVERFLOW_AND_COMPARE );
	if ( ret ) {
		pr_err("%s: Failed on omap_dm_timer_set_pwm().\n",__FUNCTION__);
		omap_dm_timer_disable(timer_ptr);
		iounmap(cm_per_base);
		return -1;
	} 
	
	ret = omap_dm_timer_set_match(timer_ptr, 1, count - 1);
	if ( ret ) {
		pr_err("%s: Failed on omap_dm_timer_set_match().\n",__FUNCTION__);
		omap_dm_timer_disable(timer_ptr);
		iounmap(cm_per_base);
		return -1;
	} 
#if 0
	ret = omap_dm_timer_set_load(timer_ptr, 1 /* one shot */, 0);
	if ( ret ) {
		pr_err("%s: Failed on omap_dm_timer_set_load().\n",__FUNCTION__);
		omap_dm_timer_disable(timer_ptr);
		iounmap(cm_per_base);
		return -1;
	} 
#else	/* omap_dm_timer_set_load() 関数はtriggerを掛けてしまうのでレジスタを設定 */
	omap_dm_timer_enable(timer_ptr);
	__raw_writel(0,timer_ptr->func_base + (OMAP_TIMER_LOAD_REG & 0xff)); 
	omap_dm_timer_disable(timer_ptr);
#endif

	ret = omap_dm_timer_start(timer_ptr);
	if ( ret ) {
		pr_err("%s: Failed on omap_dm_timer_start().\n",__FUNCTION__);
		omap_dm_timer_disable(timer_ptr);
		iounmap(cm_per_base);
		return -1;
	} 

	omap_dm_timer_enable(timer_ptr);
	reg_data = __raw_readl(timer_ptr->func_base + (OMAP_TIMER_CTRL_REG & 0xff));
	reg_data &= ~OMAP_TIMER_CTRL_AR;
	__raw_writel(reg_data,timer_ptr->func_base + (OMAP_TIMER_CTRL_REG & 0xff));
	omap_dm_timer_disable(timer_ptr);

	pr_info("%s: Successfully done.\n",__FUNCTION__);
	iounmap(cm_per_base);
	return 0;
}

int omap_dm_timer_prepare_oneshot(struct omap_dm_timer * timer_ptr,unsigned int count)
{
	int ret;
	unsigned int reg_data;
	int loopcnt=0;
	if (count < 1 || 0xfffffffe < count) {
		pr_err("%s: The range of count is invalid.\n",__FUNCTION__);
		return -1;
	}

/*      start               load                match
                              ---------------------
        ______________________|                   |__________________
        0xffffffff - 2        0                 count -1 
*/

	omap_dm_timer_stop(timer_ptr);
	ret = omap_dm_timer_set_pwm(timer_ptr,
				0,	/* default level=low 	*/
				1,	/* toggle mode		*/
				0 );
	if ( ret ) {
		pr_err("%s: Failed on omap_dm_timer_set_pwm().\n",__FUNCTION__);
		omap_dm_timer_disable(timer_ptr);
//		iounmap(cm_per_base);
		return -1;
	}

	ret = omap_dm_timer_set_load(timer_ptr, 1 /* one shot */, (0xffffffff - 2));
	omap_dm_timer_enable(timer_ptr);
	omap_dm_timer_trigger(timer_ptr);
	for (loopcnt=0;loopcnt<10;loopcnt++) {
		reg_data = __raw_readl(timer_ptr->func_base + (OMAP_TIMER_WRITE_PEND_REG & 0xff));
		if (reg_data == 0)
			break;
		udelay(10);
	}

	ret = omap_dm_timer_set_pwm(timer_ptr,
				0,	/* default level=low 	*/
				1,	/* toggle mode		*/
				OMAP_TIMER_TRIGGER_OVERFLOW_AND_COMPARE );
	if ( ret ) {
		pr_err("%s: Failed on omap_dm_timer_set_pwm().\n",__FUNCTION__);
		omap_dm_timer_disable(timer_ptr);
		return -1;
	} 
	/* set the MATCH register <count - 1> */	
	ret = omap_dm_timer_set_match(timer_ptr, 1, count - 1);
	if ( ret ) {
		pr_err("%s: Failed on omap_dm_timer_set_match().\n",__FUNCTION__);
		omap_dm_timer_disable(timer_ptr);
		return -1;
	} 
	/* set the LOAD register 0x0 */ 
	omap_dm_timer_enable(timer_ptr);
	__raw_writel(0,timer_ptr->func_base + (OMAP_TIMER_LOAD_REG & 0xff)); 
	reg_data = __raw_readl(timer_ptr->func_base + (OMAP_TIMER_IF_CTRL_REG & 0xff));
	reg_data &= ~0x04;
	__raw_writel(reg_data,timer_ptr->func_base + (OMAP_TIMER_IF_CTRL_REG & 0xff));

	return 0;
}

int omap_dm_timer_cont_prepare(struct omap_dm_timer * timer_ptr,unsigned int count)
{
	unsigned int reg_data;

	/*if (count < 1 || 0xfffffffd < count) {
		pr_err("%s: The range of count is invalid.\n",__FUNCTION__);
		return -1;
	}*/

	reg_data = __raw_readl(timer_ptr->func_base + (OMAP_TIMER_CTRL_REG & 0xff));
	reg_data &= ~OMAP_TIMER_CTRL_ST;
	__raw_writel(reg_data,timer_ptr->func_base + (OMAP_TIMER_CTRL_REG & 0xff));

	__raw_writel(0xffffffff -2,timer_ptr->func_base + (OMAP_TIMER_COUNTER_REG & 0xff));

#if 0 /* if it is not necessary to change pulse width, this can be commented-out. */
	/* set the MATCH register <count - 1> */	
	if ( omap_dm_timer_set_match(timer_ptr, 1, count - 1) ) {
		pr_err("%s: Failed on omap_dm_timer_set_match().\n",__FUNCTION__);
		omap_dm_timer_disable(timer_ptr);
		return -1;
	} 
#endif

	return 0;
}

int omap_dm_timer_do_oneshot(struct omap_dm_timer * timer_ptr)
{
	unsigned int reg_data;

//	omap_dm_timer_enable(timer_ptr);
	reg_data = __raw_readl(timer_ptr->func_base + (OMAP_TIMER_CTRL_REG & 0xff));
	reg_data |= OMAP_TIMER_CTRL_ST;
	__raw_writel(reg_data,timer_ptr->func_base + (OMAP_TIMER_CTRL_REG & 0xff));

//	omap_dm_timer_disable(timer_ptr);
	return 0;
}

int omap_cm_timer_module_enabled(int timer_num, int enabled)
{
	void __iomem* cm_per_base;
	unsigned int reg_data;
	cm_per_base = ioremap(AM33XX_PRCM_BASE, 0x100);
	reg_data = __raw_readl(cm_per_base + timer_clkctrl_offset[timer_num]);
	if (enabled) {
		reg_data |= 2;
	} else {
		reg_data &= ~2;
	}
	__raw_writel(reg_data,cm_per_base + timer_clkctrl_offset[timer_num]);

	iounmap(cm_per_base);
	return 0;
}	

int omap_dm_timer_stop_oneshot(struct omap_dm_timer * timer_ptr)
{
	int ret;

	ret = omap_dm_timer_stop(timer_ptr);
        if ( ret ) {
                pr_err("%s: Failed on omap_dm_timer_stop().\n",__FUNCTION__);
                return -1;
        }
	
	return 0;
}


