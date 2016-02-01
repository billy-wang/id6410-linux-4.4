#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sysctl.h>

/* the main switch for log enable */
int log_module_enable = 1;

/* log module table, it follows the log module index */
#define	MOD_UNIT(n, e, lvl)	{				\
	.name		= #n,					\
	.enable		= e,					\
	.level		= lvl					\
}

struct log_module_tables log_module_table[] = {
	MOD_UNIT(PM, 1, LOG_LEVEL_DEFAULT),
	MOD_UNIT(CLK, 1, LOG_LEVEL_DEFAULT),
	MOD_UNIT(WDT, 1, LOG_LEVEL_DEFAULT),
	MOD_UNIT(AXI, 1, LOG_LEVEL_DEFAULT),
	MOD_UNIT(V4L2, 1, LOG_LEVEL_DEFAULT),
	MOD_UNIT(TS, 1, LOG_LEVEL_DEFAULT),
	MOD_UNIT(OHCI, 1, LOG_LEVEL_DEFAULT),
	MOD_UNIT(DWC2, 1, LOG_LEVEL_DEFAULT),	
	MOD_UNIT(SOC, 1, LOG_LEVEL_DEFAULT),
};

/* log sysctl table */
#if 0 /* linux version 2.6.x */
#define	CTL_UNIT(n)	{					\
	.ctl_name       = LOG_MOD_##n + 1,			\
	.procname       = #n,					\
	.data           = &log_module_table[LOG_MOD_##n].enable,\
	.maxlen         = sizeof(int) * 2,			\
	.mode           = 0644,					\
	.proc_handler   = &proc_dointvec			\
}

struct ctl_table log_ctl_table[] = {
	CTL_UNIT(MAS),
	{
		.ctl_name       = LOG_MOD_MAX + 1,
		.procname       = "enable",
		.data           = &log_module_enable,
		.maxlen         = sizeof(int),
		.mode           = 0644,
		.proc_handler   = &proc_dointvec,
	},
	{ .ctl_name = 0 }
};
#else

#define	CTL_UNIT(n)	{					\
	.procname       = #n,			\
	.data           = &log_module_table[LOG_MOD_##n].enable,\
	.maxlen         = sizeof(int) * 2,			\
	.mode           = 0644,					\
	.proc_handler   = &proc_dointvec			\
}

struct ctl_table log_ctl_table[] = {
	CTL_UNIT(PM),
	CTL_UNIT(CLK),
	CTL_UNIT(WDT),	
	CTL_UNIT(AXI),
	CTL_UNIT(V4L2),
	CTL_UNIT(TS),
	CTL_UNIT(OHCI),
	CTL_UNIT(DWC2),
	CTL_UNIT(SOC),
	{
		.procname       = "enable",
		.data           = &log_module_enable,
		.maxlen         = sizeof(int),
		.mode           = 0644,
		.proc_handler   = &proc_dointvec,
	},
	{ }
};

#endif

EXPORT_SYMBOL(log_module_enable);
EXPORT_SYMBOL(log_module_table);
