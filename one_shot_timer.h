#include <plat/dmtimer.h>

/**
 * @param count // 何カウント分パルスを発生させるか
**/
extern int omap_dm_timer_create_oneshot(struct omap_dm_timer * timer_ptr,unsigned int count);
extern int omap_cm_timer_module_enabled(int timer_num, int enabled);
extern int omap_dm_timer_stop_oneshot(struct omap_dm_timer * timer_ptr);
extern int omap_dm_timer_prepare_oneshot(struct omap_dm_timer * timer_ptr,unsigned int count);
extern int omap_dm_timer_cont_prepare(struct omap_dm_timer * timer_ptr,unsigned int count);
extern int omap_dm_timer_do_oneshot(struct omap_dm_timer * timer_ptr);

