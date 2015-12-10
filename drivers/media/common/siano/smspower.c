/*
 *  Siano core API module
 *
 *  This file contains implementation for the interface to sms core component
 *
 *  author: wood
 *
 *  Copyright (c), 2005-2008 Siano Mobile Silicon, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation;
 *
 *  Software distributed under the License is distributed on an "AS IS"
 *  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.
 *
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include "smscoreapi.h"
#include <linux/delay.h>
#include "smspower.h"



#include <linux/gpio.h>

#include "kd_camera_hw.h"

#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_camera_feature.h"


extern bool _hwPowerOn(MT65XX_POWER powerId, int powerVolt, char *mode_name);
extern bool _hwPowerDown(MT65XX_POWER powerId, char *mode_name);

#ifdef CONFIG_MEDIATEK_SOLUTION
#include <mach/mt_gpio.h>
#include <cust_gpio_boot.h>

#define  HOST_SMS_RSET_PIN GPIO_CMMB_RST_PIN
//#define  HOST_SMS_POWER_IO_PIN (GPIO69 | 0x80000000)
//#define  HOST_SMS_POWER_EMI_PIN (GPIO21 | 0x80000000)
//#define  HOST_SMS_POWER_CORE_PIN (GPIO84 | 0x80000000)

#define GPIO_HIGH	GPIO_OUT_ONE
#define GPIO_LOW	GPIO_OUT_ZERO
#define gpio_request(pin,name) 0
#define gpio_free(pin)			0
#define gpio_direction_output(pin,level) \
	do { \
		mt_set_gpio_mode(pin,GPIO_MODE_00); \
		mt_set_gpio_dir(pin, GPIO_DIR_OUT); \
		mt_set_gpio_out(pin, level); \
	} while (0)
#undef gpio_set_value
#define gpio_set_value(pin,lever) mt_set_gpio_out(pin,lever)

#if defined(SMS_SPI_DRV)
static void mt_spi_set_mode(void)
{
	mt_set_gpio_mode(GPIO_SPI_CS_PIN, GPIO_SPI_CS_PIN_M_SPI_CSA);
	//mt_set_gpio_dir(GPIO_SPI_CS_PIN, GPIO_DIR_OUT);
	//mt_set_gpio_pull_select(GPIO_SPI_CS_PIN, GPIO_PULL_UP);
	//mt_set_gpio_pull_enable(GPIO_SPI_CS_PIN, GPIO_PULL_ENABLE);

	mt_set_gpio_mode(GPIO_SPI_SCK_PIN, GPIO_SPI_SCK_PIN_M_SPI_CKA);
	//mt_set_gpio_dir(GPIO_SPI_SCK_PIN, GPIO_DIR_OUT);
	//mt_set_gpio_pull_select(GPIO_SPI_SCK_PIN, GPIO_PULL_UP);
	//mt_set_gpio_pull_enable(GPIO_SPI_SCK_PIN, GPIO_PULL_ENABLE);
	
	mt_set_gpio_mode(GPIO_SPI_MISO_PIN, GPIO_SPI_MISO_PIN_M_SPI_MIA);
	//mt_set_gpio_dir(GPIO_SPI_MISO_PIN, GPIO_DIR_IN);
	//mt_set_gpio_pull_select(GPIO_SPI_MISO_PIN, GPIO_PULL_UP);
	//mt_set_gpio_pull_enable(GPIO_SPI_MISO_PIN, GPIO_PULL_ENABLE);

	mt_set_gpio_mode(GPIO_SPI_MOSI_PIN, GPIO_SPI_MOSI_PIN_M_SPI_MOA);
	//mt_set_gpio_dir(GPIO_SPI_MOSI_PIN, GPIO_DIR_OUT);
	//mt_set_gpio_pull_select(GPIO_SPI_MOSI_PIN, GPIO_PULL_UP);
	//mt_set_gpio_pull_enable(GPIO_SPI_MOSI_PIN, GPIO_PULL_ENABLE);
}

static void mt_spi_set_gpio(void)
{
	//set dir pull to save power
	mt_set_gpio_mode(GPIO_SPI_CS_PIN, GPIO_SPI_CS_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_SPI_CS_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO_SPI_CS_PIN, GPIO_PULL_DISABLE);
	
	mt_set_gpio_mode(GPIO_SPI_SCK_PIN, GPIO_SPI_SCK_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_SPI_SCK_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO_SPI_SCK_PIN, GPIO_PULL_DISABLE);

	mt_set_gpio_mode(GPIO_SPI_MISO_PIN, GPIO_SPI_MISO_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_SPI_MISO_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO_SPI_MISO_PIN, GPIO_PULL_DISABLE);

	mt_set_gpio_mode(GPIO_SPI_MOSI_PIN, GPIO_SPI_MOSI_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_SPI_MOSI_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO_SPI_MOSI_PIN, GPIO_PULL_DISABLE);

}


#endif

#elif defined(CONFIG_PLAT_RK)
#include <linux/interrupt.h>
#include "../../../spi/rk29_spim.h"
#define  HOST_SMS_RSET_PIN RK30_PIN0_PB1
#define  HOST_SMS_POWER_IO_PIN RK30_PIN0_PC7
#define  HOST_SMS_POWER_EMI_PIN RK30_PIN0_PC0
#define  HOST_SMS_POWER_CORE_PIN RK30_PIN0_PC1

#endif //CONFIG_ARCH_ ...
#if 1
void sms_chip_poweron(int id)
{
	sms_info("");
	printk("yzb---power--on\n");
#if defined(SMS_SPI_DRV) && defined(CONFIG_MEDIATEK_SOLUTION)
	mt_spi_set_mode();
#endif
_hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1200,"cmmb-core");
	mdelay(50);
_hwPowerOn(CAMERA_POWER_VCAM_IO, VOL_1800, "cmmb-io");

	
	mt_set_gpio_mode(GPIO127, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO127,GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO127,GPIO_OUT_ONE);

	mt_set_gpio_mode(GPIO126, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO126,GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO126,GPIO_OUT_ONE);

	mdelay(100);
	mt_set_gpio_mode(GPIO_CMMB_RST_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_CMMB_RST_PIN,GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CMMB_RST_PIN,GPIO_OUT_ONE);
	mdelay(100);
	printk("yzb---power--on_11_end\n");
}


void sms_chip_poweroff(int id)
{
	sms_info("");

	mt_set_gpio_mode(GPIO127, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO127,GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO127,GPIO_OUT_ONE);

	mt_set_gpio_mode(GPIO126, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO126,GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO126,GPIO_OUT_ZERO);

	

	mt_set_gpio_mode(GPIO_CMMB_RST_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_CMMB_RST_PIN,GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CMMB_RST_PIN,GPIO_OUT_ZERO);
	mdelay(100);
	printk("yzb---power--off_11_end\n");
	

	_hwPowerDown(CAMERA_POWER_VCAM_IO, "cmmb-io");
	mdelay(200);
	_hwPowerDown(CAMERA_POWER_VCAM_D, "cmmb-core");

	printk("yzb---power--off\n");
	
	mdelay(50);
	
#if defined(SMS_SPI_DRV) && defined(CONFIG_MEDIATEK_SOLUTION)
	mt_spi_set_gpio();
#endif


}
#else


#endif
