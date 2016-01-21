/*
 * fs/kernfs/mount.c - kernfs mount implementation
 *
 * Copyright (c) 2016 Billy Wang <billy.wang@gigasetdigital.com>
 *
 * This file is released under the GPLv2.
 */

#ifndef __LINUX_LOG_H
#define __LINUX_LOG_H

#define	LOG_LEVEL_EMERG		0	/* system is unusable			*/
#define	LOG_LEVEL_ALERT		1	/* action must be taken immediately	*/
#define	LOG_LEVEL_CRIT		2	/* critical conditions			*/
#define	LOG_LEVEL_ERR			3	/* error conditions			*/
#define	LOG_LEVEL_WARNING	4	/* warning conditions			*/
#define	LOG_LEVEL_NOTICE	5	/* normal but significant condition	*/
#define	LOG_LEVEL_INFO		6	/* informational			*/
#define	LOG_LEVEL_DEBUG		7	/* debug-level messages			*/
#define	LOG_LEVEL_RWREG		8	/* debug-reg messages			*/

#define	LVL_TO_STR(lvl)	"<" #lvl ">"

/* set default log level for all modules */
#ifdef	CONFIG_LOG_LEVEL_DEFAULT
#define	LOG_LEVEL_DEFAULT	CONFIG_LOG_LEVEL_DEFAULT
#else
#define	LOG_LEVEL_DEFAULT	LOG_LEVEL_INFO
#endif

/* this macro can eliminate the debug or more log in compling stage */
#ifdef	CONFIG_LOG_LEVEL_MAX
#define	LOG_LEVEL_MAX		CONFIG_LOG_LEVEL_MAX
#else
#define	LOG_LEVEL_MAX		LOG_LEVEL_RWREG
#endif

/* this is the module index used for log table */
enum log_module_index {
	LOG_MOD_PM =0,		/* power manager */
	LOG_MOD_CLK,
	LOG_MOD_WDT,
	LOG_MOD_AXI,			/* bus */
	LOG_MOD_V4L2,			/* Video for linux 2ed */
	LOG_MOD_TS,				/* touchscreen */
	/* ... */
	LOG_MOD_MAX
};

struct log_module_tables {
	const char	*name;
	int		enable;
	int		level;
};

extern int log_module_enable;
extern struct log_module_tables log_module_table[];

/* the arguments mod, lvl and fmt must be constant:
 * mod: LOG_MOD_xxx
 * lvl: LOG_LEVEL_xxx
 * fmt: "..."
 */

#define	log_printk_normal(mod, lvl, fmt, args...)	do {			\
	if (lvl <= LOG_LEVEL_MAX) {						\
		if (log_module_enable && log_module_table[mod].enable &&	\
		    lvl <= log_module_table[mod].level) {			\
			printk(KERN_ERR LVL_TO_STR(lvl) "{%s} " fmt,			\
			    log_module_table[mod].name, ##args);		\
		}								\
	}									\
} while (0)

#define	log_printk_err(mod, lvl, fmt, args...)	do {			\
		if (lvl <= LOG_LEVEL_MAX) { 					\
			if (log_module_enable && log_module_table[mod].enable &&	\
					lvl <= log_module_table[mod].level) { 		\
				printk(KERN_ERR LVL_TO_STR(lvl) "{%s} " fmt,			\
						log_module_table[mod].name, ##args);		\
			} 							\
		} 								\
	} while (0)

#define	log_printk_warning(mod, lvl, fmt, args...)	do {			\
		if (lvl <= LOG_LEVEL_MAX) { 					\
			if (log_module_enable && log_module_table[mod].enable &&	\
					lvl <= log_module_table[mod].level) { 		\
				printk(KERN_WARNING LVL_TO_STR(lvl) "{%s} " fmt,			\
						log_module_table[mod].name, ##args);		\
			} 							\
		} 								\
	} while (0)

#define	log_printk_notice(mod, lvl, fmt, args...)	do {			\
		if (lvl <= LOG_LEVEL_MAX) { 					\
			if (log_module_enable && log_module_table[mod].enable &&	\
					lvl <= log_module_table[mod].level) { 		\
				printk(KERN_NOTICE LVL_TO_STR(lvl) "{%s} " fmt, 		\
						log_module_table[mod].name, ##args);		\
			} 							\
		} 								\
	} while (0)

#define	log_printk_info(mod, lvl, fmt, args...)	do {			\
		if (lvl <= LOG_LEVEL_MAX) { 					\
			if (log_module_enable && log_module_table[mod].enable &&	\
					lvl <= log_module_table[mod].level) { 		\
				printk(KERN_INFO LVL_TO_STR(lvl) "{%s} " fmt,			\
						log_module_table[mod].name, ##args);		\
			} 							\
		} 								\
	} while (0)

#define	log_printk_debug(mod, lvl, fmt, args...)	do {			\
	if (lvl <= LOG_LEVEL_MAX) {						\
		if (log_module_enable && log_module_table[mod].enable &&	\
		    lvl <= log_module_table[mod].level) {			\
			printk(KERN_ERR LVL_TO_STR(lvl) "{%s} %s(%d) " fmt,		\
			    log_module_table[mod].name,				\
			    __func__, __LINE__, ##args);			\
		}								\
	}									\
} while (0)

#define	log_printk_rwreg(mod, lvl, fmt, args...)	do {			\
			if (lvl <= LOG_LEVEL_MAX) { 					\
				if (log_module_enable && log_module_table[mod].enable &&	\
						lvl <= log_module_table[mod].level) { 		\
					printk(KERN_EMERG LVL_TO_STR(lvl) "{%s} " fmt, \
						log_module_table[mod].name,##args);			\
				} 							\
			} 								\
		} while (0)

#define	log_printk(mod, lvl, fmt, args...)		do {			\
	if (lvl == LOG_LEVEL_RWREG)						\
		log_printk_rwreg(mod, lvl, fmt, ##args);			\
	else if (lvl == LOG_LEVEL_DEBUG)						\
		log_printk_debug(mod, lvl, fmt, ##args);			\
	else if	(lvl == LOG_LEVEL_INFO)						\
		log_printk_info(mod, lvl, fmt, ##args);				\
	else if (lvl == LOG_LEVEL_NOTICE) 					\
		log_printk_notice(mod, lvl, fmt, ##args); 			\
	else if (lvl == LOG_LEVEL_WARNING)	\
		log_printk_warning(mod, lvl, fmt, ##args); 		\
	else if (lvl == LOG_LEVEL_ERR)					\
		log_printk_err(mod, lvl, fmt, ##args);		\
	else									\
		log_printk_normal(mod, lvl, fmt, ##args);			\
} while (0)

#endif
