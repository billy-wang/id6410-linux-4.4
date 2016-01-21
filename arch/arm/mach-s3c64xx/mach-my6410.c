/* linux/arch/arm/mach-s3c64xx/mach-my6410.c
 *
 * Copyright 2010 Darius Augulis <augulis.darius@gmail.com>
 * Copyright 2008 Openmoko, Inc.
 * Copyright 2008 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *	http://armlinux.simtec.co.uk/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/fb.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/dm9000.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/serial_core.h>
#include <linux/serial_s3c.h>
#include <linux/types.h>
#include <linux/pwm.h>
#include <linux/pwm_backlight.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <mach/map.h>
#include <mach/regs-gpio.h>
#include <mach/gpio-samsung.h>
#include <mach/regs-clock.h>

#include <plat/adc.h>
#include <plat/cpu.h>
#include <plat/devs.h>
#include <plat/fb.h>
#include <plat/gpio-cfg.h>

#include <linux/platform_data/i2c-s3c2410.h>
#include <linux/platform_data/mtd-nand-s3c2410.h>
#include <linux/platform_data/mmc-sdhci-s3c.h>
#include <plat/sdhci.h>
#include <linux/platform_data/touchscreen-s3c2410.h>
#include <linux/platform_data/s3c-hsotg.h>
#include <linux/mmc/host.h>
#include <plat/keypad.h>

#include <video/platform_lcd.h>
#include <video/samsung_fimd.h>
#include <plat/samsung-time.h>

#include "common.h"
#include "regs-modem.h"
#include "regs-srom.h"

#define UCON S3C2410_UCON_DEFAULT
#define ULCON (S3C2410_LCON_CS8 | S3C2410_LCON_PNONE | S3C2410_LCON_STOPB)
#define UFCON (S3C2410_UFCON_RXTRIG8 | S3C2410_UFCON_FIFOMODE)

static struct dwc2_hsotg_plat my6410_hsotg_pdata;

static struct s3c2410_uartcfg my6410_uartcfgs[] __initdata = {
	[0] = {
		.hwport	= 0,
		.flags	= 0,
		.ucon	= UCON,
		.ulcon	= ULCON,
		.ufcon	= UFCON,
	},
	[1] = {
		.hwport	= 1,
		.flags	= 0,
		.ucon	= UCON,
		.ulcon	= ULCON,
		.ufcon	= UFCON,
	},
	[2] = {
		.hwport	= 2,
		.flags	= 0,
		.ucon	= UCON,
		.ulcon	= ULCON,
		.ufcon	= UFCON,
	},
	[3] = {
		.hwport	= 3,
		.flags	= 0,
		.ucon	= UCON,
		.ulcon	= ULCON,
		.ufcon	= UFCON,
	},
};

/* DM9000AEP 10/100 ethernet controller */

static struct resource my6410_dm9k_resource[] = {
	[0] = DEFINE_RES_MEM(S3C64XX_PA_XM0CSN1, 2),
	[1] = DEFINE_RES_MEM(S3C64XX_PA_XM0CSN1 + 4, 2),
	[2] = DEFINE_RES_NAMED(S3C_EINT(7), 1, NULL, IORESOURCE_IRQ \
					| IORESOURCE_IRQ_HIGHLEVEL),
};

static struct dm9000_plat_data my6410_dm9k_pdata = {
	.flags		= (DM9000_PLATF_16BITONLY | DM9000_PLATF_NO_EEPROM),		
	.dev_addr		= { 0x08, 0x90, 0x00, 0xa0, 0x90, 0x90 },
};

static struct platform_device my6410_device_eth = {
	.name		= "dm9000",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(my6410_dm9k_resource),
	.resource	= my6410_dm9k_resource,
	.dev		= {
		.platform_data	= &my6410_dm9k_pdata,
	},
};

static struct mtd_partition my6410_nand_part[] = {
	[0] = {
		.name	= "uboot",
		.size	= SZ_1M,
		.offset	= 0,
	},
	[1] = {
		.name	= "kernel",
		.size	= SZ_8M,
		.offset	= SZ_1M,
	},
	[2] = {
		.name	= "rootfs",
		.size	= SZ_128M,
		.offset	= SZ_1M + SZ_8M,
	},
	[3] = {
		.name	= "file system",
		.size	= MTDPART_SIZ_FULL,
		.offset	= SZ_1M + SZ_8M + SZ_128M,
	},
};

static struct s3c2410_nand_set my6410_nand_sets[] = {
	[0] = {
		.name		= "nand",
		.nr_chips	= 1,
		.nr_partitions	= ARRAY_SIZE(my6410_nand_part),
		.partitions	= my6410_nand_part,
		//.disable_ecc=1,
	},
};

static struct s3c2410_platform_nand my6410_nand_info = {
	.tacls		= 25,
	.twrph0		= 55,
	.twrph1		= 40,
	.nr_sets	= ARRAY_SIZE(my6410_nand_sets),
	.sets		= my6410_nand_sets,
};

static struct s3c_fb_pd_win my6410_lcd_type0_fb_win = {
	.max_bpp	= 32,
	.default_bpp	= 16,
	.xres		= 480,
	.yres		= 272,
};

static struct fb_videomode my6410_lcd_type0_timing = {
	/* 4.3" 480x272 */
	.left_margin	= 2,
	.right_margin	= 2,
	.upper_margin	= 2,
	.lower_margin	= 2,
	.hsync_len	= 41,
	.vsync_len	= 10,
	.xres		= 480,
	.yres		= 272,
	.refresh	= 60,
};

static struct s3c_fb_pd_win my6410_lcd_type1_fb_win = {
	.max_bpp	= 32,
	.default_bpp	= 16,
	.xres		= 800,
	.yres		= 480,
};

static struct fb_videomode my6410_lcd_type1_timing = {
	/* 7.0" 800x480 */
	.left_margin	= 8,
	.right_margin	= 13,
	.upper_margin	= 7,
	.lower_margin	= 5,
	.hsync_len	= 3,
	.vsync_len	= 1,
	.xres		= 800,
	.yres		= 480,
};

static struct s3c_fb_platdata my6410_lcd_pdata[] __initdata = {
	{
		.setup_gpio	= s3c64xx_fb_gpio_setup_24bpp,
		.vtiming	= &my6410_lcd_type0_timing,
		.win[0]		= &my6410_lcd_type0_fb_win,
		.vidcon0	= VIDCON0_VIDOUT_RGB | VIDCON0_PNRMODE_RGB,
		.vidcon1	= VIDCON1_INV_HSYNC | VIDCON1_INV_VSYNC,
		/* 0x60
		 * bit[7]= 0: RGB type LCD driver gets the video data at VCLK falling edge
		 * bit[6]= 1: HSYNC
		 * bit[5]= 1: VSYNC
		 * bit[4]= 0: VDEN
		*/
	}, {
		.setup_gpio	= s3c64xx_fb_gpio_setup_24bpp,
		.vtiming	= &my6410_lcd_type1_timing,
		.win[0]		= &my6410_lcd_type1_fb_win,
		.vidcon0	= VIDCON0_VIDOUT_RGB | VIDCON0_PNRMODE_RGB,
		.vidcon1	= VIDCON1_INV_HSYNC | VIDCON1_INV_VSYNC,
	},
	{ },
};

static struct pwm_lookup my6410_pwm_lookup[] = {
	PWM_LOOKUP("samsung-pwm", 0, "pwm-backlight", NULL, 100000,
		   PWM_POLARITY_NORMAL),
};

static struct platform_pwm_backlight_data my6410_backlight_data = {
	.max_brightness	= 1000,
	.dft_brightness	= 600,
	.enable_gpio	= -1,
};

static struct platform_device my6410_backlight_device = {
	.name		= "pwm-backlight",
	.id		= -1,
	.dev		= {
		.parent	= &samsung_device_pwm.dev,
		.platform_data = &my6410_backlight_data,
	},
};

static void my6410_lcd_power_set(struct plat_lcd_data *pd,
				   unsigned int power)
{
	pr_debug("%s: setting power %d\n", __func__, power);

	if (power){
		//gpio_set_value(S3C64XX_GPE(0), 1);
		s3c_gpio_cfgpin(S3C64XX_GPF(14), S3C_GPIO_SFN(2));
	}else{
		gpio_direction_output(S3C64XX_GPF(14), 0);
		//gpio_set_value(S3C64XX_GPE(0), 0);
	}
}

static struct plat_lcd_data my6410_lcd_power_data = {
	.set_power	= my6410_lcd_power_set,
};

static struct platform_device my6410_lcd_powerdev = {
	.name			= "platform-lcd",
	.dev.parent		= &s3c_device_fb.dev,
	.dev.platform_data	= &my6410_lcd_power_data,
};


static struct s3c_sdhci_platdata my6410_hsmmc0_pdata = {
	.max_width		= 4,
	//.host_caps 	= MMC_VDD_32_33|MMC_VDD_33_34,
	.host_caps 	= (MMC_CAP_4_BIT_DATA |
			   MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED),
	.cd_type		= S3C_SDHCI_CD_GPIO,
	.ext_cd_gpio		= S3C64XX_GPG(6),
	.ext_cd_gpio_invert	= true,
};

static struct s3c_sdhci_platdata my6410_hsmmc1_pdata = {
	.max_width		= 4,
	//.host_caps 	= MMC_VDD_32_33|MMC_VDD_33_34,
	.host_caps 	= (MMC_CAP_4_BIT_DATA |
			   MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED),
	.cd_type		= S3C_SDHCI_CD_GPIO,
	.ext_cd_gpio		= S3C64XX_GPN(10),
	.ext_cd_gpio_invert	= true,
};

static struct i2c_board_info i2c_devs0[] __initdata = {
	{ I2C_BOARD_INFO("24c128", 0x57), },
};

static uint32_t my6410_keymap[] __initdata = {
	/* KEY(row, col, keycode) */	
	KEY(0, 0, KEY_1), KEY(0, 1, KEY_2), KEY(0, 2, KEY_3),
	KEY(0, 3, KEY_4), KEY(0, 4, KEY_5), KEY(0, 5, KEY_6),
	KEY(0, 6, KEY_7), KEY(0, 7, KEY_8),
	KEY(1, 0, KEY_A), KEY(1, 0, KEY_B), KEY(1, 2, KEY_C),
	KEY(1, 3, KEY_D), KEY(1, 4, KEY_E), KEY(1, 5, KEY_F),
	KEY(1, 6, KEY_G), KEY(1, 7, KEY_H)
};

static struct matrix_keymap_data my6410_keymap_data __initdata = {
	.keymap		= my6410_keymap,
	.keymap_size	= ARRAY_SIZE(my6410_keymap),
};

static struct samsung_keypad_platdata my6410_keypad_data __initdata = {
	.keymap_data	= &my6410_keymap_data,
	.rows		= 8,
	.cols		= 8,
};

static struct platform_device *my6410_devices[] __initdata = {
	&my6410_device_eth,
	&s3c_device_hsmmc0,
	&s3c_device_hsmmc1,	
	&s3c_device_i2c0,
	&s3c_device_ohci,
	&s3c_device_nand,
	&s3c_device_fb,
	&samsung_device_pwm,	
	&s3c64xx_device_iisv4,
	&my6410_lcd_powerdev,
	&my6410_backlight_device,
#ifdef CONFIG_SAMSUNG_DEV_ADC
	&s3c_device_adc,
#endif
	&s3c_device_ts,
	&s3c_device_usb_hsotg,
	&s3c_device_rtc,	
	&s3c_device_wdt,
};

static void __init my6410_map_io(void)
{
	u32 tmp;

	s3c64xx_init_io(NULL, 0);
	s3c64xx_set_xtal_freq(12000000);
	s3c24xx_init_uarts(my6410_uartcfgs, ARRAY_SIZE(my6410_uartcfgs));
	samsung_set_timer_source(SAMSUNG_PWM3, SAMSUNG_PWM4);

	/* set the LCD type */
	tmp = __raw_readl(S3C64XX_SPCON);
	tmp &= ~S3C64XX_SPCON_LCD_SEL_MASK;
	tmp |= S3C64XX_SPCON_LCD_SEL_RGB;
	__raw_writel(tmp, S3C64XX_SPCON);

	/* remove the LCD bypass */
	tmp = __raw_readl(S3C64XX_MODEM_MIFPCON);
	tmp &= ~MIFPCON_LCD_BYPASS;
	__raw_writel(tmp, S3C64XX_MODEM_MIFPCON);
}

/*
 * my6410_features string
 *
 * 0-9 LCD configuration
 *
 */
static char my6410_features_str[12] __initdata = "0";

static int __init my6410_features_setup(char *str)
{
	if (str)
		strlcpy(my6410_features_str, str,
			sizeof(my6410_features_str));
	return 1;
}

__setup("my6410=", my6410_features_setup);

#define FEATURE_SCREEN (1 << 0)

struct my6410_features_t {
	int done;
	int lcd_index;
};

static void my6410_parse_features(
		struct my6410_features_t *features,
		const char *features_str)
{
	const char *fp = features_str;

	features->done = 0;
	features->lcd_index = 0;

	while (*fp) {
		char f = *fp++;

		switch (f) {
		case '0'...'9':	/* tft screen */
			if (features->done & FEATURE_SCREEN) {
				printk(KERN_INFO "MY6410: '%c' ignored, "
					"screen type already set\n", f);
			} else {
				int li = f - '0';
				if (li >= ARRAY_SIZE(my6410_lcd_pdata))
					printk(KERN_INFO "MY6410: '%c' out "
						"of range LCD mode\n", f);
				else {
					features->lcd_index = li;
				}
			}
			features->done |= FEATURE_SCREEN;
			break;
		}
	}
}

static void __init my6410_machine_init(void)
{
	u32 cs1;
	struct my6410_features_t features = { 0 };

	printk(KERN_INFO "MY6410: Option string my6410=%s\n",
			my6410_features_str);
	/* Parse the feature string */
	my6410_parse_features(&features, my6410_features_str);

	printk(KERN_INFO "MY6410: selected LCD display is %dx%d\n",
		my6410_lcd_pdata[features.lcd_index].win[0]->xres,
		my6410_lcd_pdata[features.lcd_index].win[0]->yres);

	s3c_i2c0_set_platdata(NULL);
	s3c_nand_set_platdata(&my6410_nand_info);
	s3c_fb_set_platdata(&my6410_lcd_pdata[features.lcd_index]);
	s3c_sdhci0_set_platdata(&my6410_hsmmc0_pdata);
	s3c_sdhci1_set_platdata(&my6410_hsmmc1_pdata);
	s3c24xx_ts_set_platdata(NULL);
	dwc2_hsotg_set_platdata(&my6410_hsotg_pdata);

	samsung_keypad_set_platdata(&my6410_keypad_data);

	/* configure nCS1 width to 16 bits */

	cs1 = __raw_readl(S3C64XX_SROM_BW) &
		~(S3C64XX_SROM_BW__CS_MASK << S3C64XX_SROM_BW__NCS1__SHIFT);
	cs1 |= ((1 << S3C64XX_SROM_BW__DATAWIDTH__SHIFT) |
		(1 << S3C64XX_SROM_BW__WAITENABLE__SHIFT) |
		(1 << S3C64XX_SROM_BW__BYTEENABLE__SHIFT)) <<
			S3C64XX_SROM_BW__NCS1__SHIFT;
	__raw_writel(cs1, S3C64XX_SROM_BW);

	/* set timing for nCS1 suitable for ethernet chip */

	__raw_writel((0 << S3C64XX_SROM_BCX__PMC__SHIFT) |
		(6 << S3C64XX_SROM_BCX__TACP__SHIFT) |
		(4 << S3C64XX_SROM_BCX__TCAH__SHIFT) |
		(1 << S3C64XX_SROM_BCX__TCOH__SHIFT) |
		(13 << S3C64XX_SROM_BCX__TACC__SHIFT) |
		(4 << S3C64XX_SROM_BCX__TCOS__SHIFT) |
		(0 << S3C64XX_SROM_BCX__TACS__SHIFT), S3C64XX_SROM_BC1);

	gpio_request(S3C64XX_GPE(0), "LCD BL ");
	gpio_direction_output(S3C64XX_GPF(14), 0);  /* turn off not user */

	gpio_request(S3C64XX_GPF(14), "LCD PWM");
	gpio_direction_output(S3C64XX_GPF(14), 0);  /* turn off not user */

	i2c_register_board_info(0, i2c_devs0, ARRAY_SIZE(i2c_devs0));

	pwm_add_table(my6410_pwm_lookup, ARRAY_SIZE(my6410_pwm_lookup));

	platform_add_devices(my6410_devices, ARRAY_SIZE(my6410_devices));
}

MACHINE_START(MY6410, "MY6410")
	/* Maintainer: Darius Augulis <augulis.darius@gmail.com> */
	.atag_offset	= 0x100,
	.init_irq	= s3c6410_init_irq,
	.map_io		= my6410_map_io,
	.init_machine	= my6410_machine_init,
	.init_time	= samsung_timer_init,
	.restart	= s3c64xx_restart,
MACHINE_END
