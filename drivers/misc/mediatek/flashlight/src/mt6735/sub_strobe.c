
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/time.h>
#include "kd_flashlight.h"
#include <asm/io.h>
#include <asm/uaccess.h>
#include "kd_camera_hw.h"
#include <cust_gpio_usage.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/xlog.h>
#include <linux/version.h>


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
#include <linux/mutex.h>
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#include <linux/semaphore.h>
#else
#include <asm/semaphore.h>
#endif
#endif
// add for pwm
#include<mach/mt_pwm.h>
#include<mach/mt_clkmgr.h>
#include<mach/mt_gpio.h>
#include<mach/mt_typedefs.h>
#include<mach/upmu_common.h>
//end


/******************************************************************************
 * Debug configuration
******************************************************************************/
// availible parameter
// ANDROID_LOG_ASSERT
// ANDROID_LOG_ERROR
// ANDROID_LOG_WARNING
// ANDROID_LOG_INFO
// ANDROID_LOG_DEBUG
// ANDROID_LOG_VERBOSE
#define TAG_NAME "sub_strobe.c"
//#define PK_DBG_NONE(fmt, arg...)    do {} while (0)
//#define PK_DBG_FUNC(fmt, arg...)    xlog_printk(ANDROID_LOG_DEBUG  , TAG_NAME, KERN_INFO  "%s: " fmt, __FUNCTION__ ,##arg)
//#define PK_WARN(fmt, arg...)        xlog_printk(ANDROID_LOG_WARNING, TAG_NAME, KERN_WARNING  "%s: " fmt, __FUNCTION__ ,##arg)
//#define PK_NOTICE(fmt, arg...)      xlog_printk(ANDROID_LOG_DEBUG  , TAG_NAME, KERN_NOTICE  "%s: " fmt, __FUNCTION__ ,##arg)
//#define PK_INFO(fmt, arg...)        xlog_printk(ANDROID_LOG_INFO   , TAG_NAME, KERN_INFO  "%s: " fmt, __FUNCTION__ ,##arg)
//#define PK_TRC_FUNC(f)              xlog_printk(ANDROID_LOG_DEBUG  , TAG_NAME,  "<%s>\n", __FUNCTION__);
//#define PK_TRC_VERBOSE(fmt, arg...) xlog_printk(ANDROID_LOG_VERBOSE, TAG_NAME,  fmt, ##arg)
//#define PK_ERROR(fmt, arg...)       xlog_printk(ANDROID_LOG_ERROR  , TAG_NAME, KERN_ERR "%s: " fmt, __FUNCTION__ ,##arg)


#define DEBUG_LEDS_STROBE
#ifdef  DEBUG_LEDS_STROBE
//	#define PK_DBG PK_DBG_FUNC
//	#define PK_VER PK_TRC_VERBOSE
//	#define PK_ERR PK_ERROR

	#define PK_DBG printk
	#define PK_VER printk
	#define PK_ERR printk


#else
	#define PK_DBG(a,...)
	#define PK_VER(a,...)
	#define PK_ERR(a,...)
#endif

/******************************************************************************
 * local variables
******************************************************************************/

static DEFINE_SPINLOCK(g_strobeSMPLock); /* cotta-- SMP proection */


static u32 strobe_Res = 0;
static u32 strobe_Timeus = 0;
static BOOL g_strobe_On = 0;

static int g_duty=-1;
static int g_timeOutTimeMs=0;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
static DEFINE_MUTEX(g_strobeSem);
#else
static DECLARE_MUTEX(g_strobeSem);
#endif

int duty_table[12] = {0, 0, 0, 20, 30, 40, 50, 60, 70, 80, 90, 100};

int Flashlight2_Switch=0;//aeon add for factory mode  flashlight test
static int flag1 = 1;

static struct work_struct workTimeOut;

#define FLASH_GPIO_ENF GPIO_CAMERA_FLASH_MODE_PIN
//#define FLASH_GPIO_ENF GPIO_PWM_FRONT_FLASH_EN_PIN
//#define FLASH_GPIO_ENT GPIO_CAMERA_FLASH_MODE_PIN

/*****************************************************************************
Functions
*****************************************************************************/
static void work_timeOutFunc(struct work_struct *data);

static int gpio_pwm_flash_15(void)
{
	struct pwm_spec_config   pwm_setting ;
	printk("sub func:%s Enter\n",__func__);
	mt_set_gpio_mode(FLASH_GPIO_ENF,GPIO_MODE_01);
	pwm_setting.pwm_no  = PWM3;
#if 1
	pwm_setting.mode    = PWM_MODE_FIFO;
	pwm_setting.clk_div = CLK_DIV8;
	pwm_setting.clk_src = PWM_CLK_NEW_MODE_BLOCK;
	pwm_setting.PWM_MODE_FIFO_REGS.IDLE_VALUE = true;
	pwm_setting.PWM_MODE_FIFO_REGS.GUARD_VALUE = false;
	pwm_setting.PWM_MODE_FIFO_REGS.STOP_BITPOS_VALUE = 63;
	pwm_setting.PWM_MODE_FIFO_REGS.HDURATION = 1;
	pwm_setting.PWM_MODE_FIFO_REGS.LDURATION = 1;
	pwm_setting.PWM_MODE_FIFO_REGS.GDURATION = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.WAVE_NUM  = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA0 = 0xfff00000;
	pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA1 = 0;
#else
	pwm_setting.pmic_pad = false;
	pwm_setting.mode = PWM_MODE_OLD;
	pwm_setting.clk_src = PWM_CLK_OLD_MODE_BLOCK;
	pwm_setting.clk_div = CLK_DIV4;
	pwm_setting.PWM_MODE_OLD_REGS.IDLE_VALUE = 0;
	pwm_setting.PWM_MODE_OLD_REGS.GUARD_VALUE = 0;
	pwm_setting.PWM_MODE_OLD_REGS.GDURATION = 0;
	pwm_setting.PWM_MODE_OLD_REGS.WAVE_NUM = 0;
	pwm_setting.PWM_MODE_OLD_REGS.DATA_WIDTH = 100; // 100 level
	pwm_setting.PWM_MODE_OLD_REGS.THRESH = 15;
#endif
	pwm_set_spec_config(&pwm_setting);
}
static int gpio_pwm_flash_50(void)

{
	struct pwm_spec_config   pwm_setting ;
	printk("sub func:%s Enter\n",__func__);
	mt_set_gpio_mode(FLASH_GPIO_ENF,GPIO_MODE_01);
	pwm_setting.pwm_no  = PWM3;
#if 1
	pwm_setting.mode    = PWM_MODE_FIFO;
	pwm_setting.clk_div = CLK_DIV8;
	pwm_setting.clk_src = PWM_CLK_NEW_MODE_BLOCK;
	pwm_setting.PWM_MODE_FIFO_REGS.IDLE_VALUE = true;
	pwm_setting.PWM_MODE_FIFO_REGS.GUARD_VALUE = false;
	pwm_setting.PWM_MODE_FIFO_REGS.STOP_BITPOS_VALUE = 63;
	pwm_setting.PWM_MODE_FIFO_REGS.HDURATION = 1;
	pwm_setting.PWM_MODE_FIFO_REGS.LDURATION = 1;
	pwm_setting.PWM_MODE_FIFO_REGS.GDURATION = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.WAVE_NUM  = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA0 = 0xffffffff;
	pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA1 = 0x00000000;
#else
	pwm_setting.pmic_pad = false;
	pwm_setting.mode = PWM_MODE_OLD;
	pwm_setting.clk_src = PWM_CLK_OLD_MODE_BLOCK;
	pwm_setting.clk_div = CLK_DIV4;
	pwm_setting.PWM_MODE_OLD_REGS.IDLE_VALUE = 0;
	pwm_setting.PWM_MODE_OLD_REGS.GUARD_VALUE = 0;
	pwm_setting.PWM_MODE_OLD_REGS.GDURATION = 0;
	pwm_setting.PWM_MODE_OLD_REGS.WAVE_NUM = 0;
	pwm_setting.PWM_MODE_OLD_REGS.DATA_WIDTH = 100; // 100 level
	pwm_setting.PWM_MODE_OLD_REGS.THRESH = 50;
#endif

	pwm_set_spec_config(&pwm_setting);
}

static int gpio_pwm_flash_75(void)

{
	struct pwm_spec_config   pwm_setting ;
	printk("sub func:%s Enter\n",__func__);
	mt_set_gpio_mode(FLASH_GPIO_ENF,GPIO_MODE_01);
	pwm_setting.pwm_no  = PWM3;
#if 1
	pwm_setting.mode    = PWM_MODE_FIFO;
	pwm_setting.clk_div = CLK_DIV8;
	pwm_setting.clk_src = PWM_CLK_NEW_MODE_BLOCK;
	pwm_setting.PWM_MODE_FIFO_REGS.IDLE_VALUE = true;
	pwm_setting.PWM_MODE_FIFO_REGS.GUARD_VALUE = false;
	pwm_setting.PWM_MODE_FIFO_REGS.STOP_BITPOS_VALUE = 63;
	pwm_setting.PWM_MODE_FIFO_REGS.HDURATION = 1;
	pwm_setting.PWM_MODE_FIFO_REGS.LDURATION = 1;
	pwm_setting.PWM_MODE_FIFO_REGS.GDURATION = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.WAVE_NUM  = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA0 = 0xffffffff;
	pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA1 = 0x0000ffff;
#else
	pwm_setting.pmic_pad = false;
	pwm_setting.mode = PWM_MODE_OLD;
	pwm_setting.clk_src = PWM_CLK_OLD_MODE_BLOCK;
	pwm_setting.clk_div = CLK_DIV4;
	pwm_setting.PWM_MODE_OLD_REGS.IDLE_VALUE = 0;
	pwm_setting.PWM_MODE_OLD_REGS.GUARD_VALUE = 0;
	pwm_setting.PWM_MODE_OLD_REGS.GDURATION = 0;
	pwm_setting.PWM_MODE_OLD_REGS.WAVE_NUM = 0;
	pwm_setting.PWM_MODE_OLD_REGS.DATA_WIDTH = 100; // 100 level
	pwm_setting.PWM_MODE_OLD_REGS.THRESH = 75;
#endif

	pwm_set_spec_config(&pwm_setting);
}

static void gpio_flash_open(void)
{
	printk("sub func:%s Enter\n",__func__);
	mt_set_gpio_mode(FLASH_GPIO_ENF, GPIO_MODE_00);
  	mt_set_gpio_dir(FLASH_GPIO_ENF, GPIO_DIR_OUT);
	mt_set_gpio_out(FLASH_GPIO_ENF, GPIO_OUT_ONE);
}
static void gpio_flash_close(void)
{
	printk("sub func:%s Enter\n",__func__);
	mt_set_gpio_mode(FLASH_GPIO_ENF, GPIO_MODE_00);
  	mt_set_gpio_dir(FLASH_GPIO_ENF, GPIO_DIR_OUT);
	mt_set_gpio_out(FLASH_GPIO_ENF, GPIO_OUT_ZERO);
}
static int FL_Enable(void)
{
	printk("sub func:%s,g_duty = %d\n",__func__,g_duty);
	if(g_duty > 4)//flashlight
		gpio_pwm_flash_50();
		//gpio_flash_open();
	else//torch
	{
		gpio_pwm_flash_15();
		//gpio_flash_open();
	}
	PK_DBG("sub FL_Enable line=%d\n",__LINE__);
	Flashlight2_Switch=1;//aeon add for factory mode  flashlight test
    return 0;
}



static int FL_Disable(void)
{
	printk("sub func:%s,Enter\n",__func__);
	gpio_flash_close();

	PK_DBG("sub FL_Disable line=%d\n",__LINE__);
	Flashlight2_Switch=0;//aeon add for factory mode  flashlight test
    return 0;
}

static int FL_dim_duty(kal_uint32 duty)
{
	PK_DBG("sub FL_dim_duty line=%d\n",__LINE__);
	g_duty = duty;
    return 0;
}




static int FL_Init(void)
{


    //PK_DBG(" FL_Init regVal0=%d isLtVer=%d\n",regVal0, g_bLtVersion);
    printk("subfunc:%s:Enter\n",__func__);

    if(mt_set_gpio_mode(FLASH_GPIO_ENF,GPIO_MODE_00)){PK_DBG("sub[constant_flashlight] set gpio mode failed!! \n");}
    if(mt_set_gpio_dir(FLASH_GPIO_ENF,GPIO_DIR_OUT)){PK_DBG("sub[constant_flashlight] set gpio dir failed!! \n");}
    if(mt_set_gpio_out(FLASH_GPIO_ENF,GPIO_OUT_ZERO)){PK_DBG("sub[constant_flashlight] set gpio failed!! \n");}



    gpio_flash_close();
    PK_DBG("sub FL_Init line=%d\n",__LINE__);
	Flashlight2_Switch=0;//aeon add for factory mode  flashlight test
    return 0;
}


static int FL_Uninit(void)
{
	FL_Disable();
    return 0;
}

/*****************************************************************************
User interface
*****************************************************************************/

static void work_timeOutFunc(struct work_struct *data)
{
    FL_Disable();
    PK_DBG("sub ledTimeOut_callback\n");
    //printk(KERN_ALERT "work handler function./n");
}



static enum hrtimer_restart ledTimeOutCallback(struct hrtimer *timer)
{
    schedule_work(&workTimeOut);
    return HRTIMER_NORESTART;
}
static struct hrtimer g_timeOutTimer;
static void timerInit(void)
{
  INIT_WORK(&workTimeOut, work_timeOutFunc);
	g_timeOutTimeMs=1000; //1s
	hrtimer_init( &g_timeOutTimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
	g_timeOutTimer.function=ledTimeOutCallback;

}



static int sub_strobe_ioctl(MUINT32 cmd, MUINT32 arg)
{
	int i4RetValue = 0;
	int ior_shift;
	int iow_shift;
	int iowr_shift;
	ior_shift = cmd - (_IOR(FLASHLIGHT_MAGIC,0, int));
	iow_shift = cmd - (_IOW(FLASHLIGHT_MAGIC,0, int));
	iowr_shift = cmd - (_IOWR(FLASHLIGHT_MAGIC,0, int));
	PK_DBG("sub_strobe_ioctl() line=%d ior_shift=%d, iow_shift=%d iowr_shift=%d arg=%d\n",__LINE__, ior_shift, iow_shift, iowr_shift, arg);
    switch(cmd)
    {

		case FLASH_IOC_SET_TIME_OUT_TIME_MS:
			PK_DBG("FLASH_IOC_SET_TIME_OUT_TIME_MS: %d\n",arg);
			g_timeOutTimeMs=arg;
		break;


    	case FLASH_IOC_SET_DUTY :
    		PK_DBG("FLASHLIGHT_DUTY: %d\n",arg);
    		FL_dim_duty(arg);
    		break;


    	case FLASH_IOC_SET_STEP:
    		PK_DBG("FLASH_IOC_SET_STEP: %d\n",arg);

    		break;

    	case FLASH_IOC_SET_ONOFF :
    		PK_DBG("FLASHLIGHT_ONOFF: %d\n",arg);
    		if(arg==1)
    		{
				if(g_timeOutTimeMs!=0)
	            {
	            	ktime_t ktime;
					ktime = ktime_set( 0, g_timeOutTimeMs*1000000 );
					hrtimer_start( &g_timeOutTimer, ktime, HRTIMER_MODE_REL );
	            }
    			FL_Enable();
    		}
    		else
    		{
    			FL_Disable();
				hrtimer_cancel( &g_timeOutTimer );
    		}
    		break;
		default :
    		PK_DBG(" No such command \n");
    		i4RetValue = -EPERM;
    		break;
    }
    return i4RetValue;
}




static int sub_strobe_open(void *pArg)
{
    int i4RetValue = 0;
    PK_DBG("sub_strobe_open line=%d\n", __LINE__);

	if (0 == strobe_Res)
	{
	    FL_Init();
		timerInit();
	}
	PK_DBG("sub_strobe_open line=%d\n", __LINE__);
	spin_lock_irq(&g_strobeSMPLock);


    if(strobe_Res)
    {
        PK_ERR(" busy!\n");
        i4RetValue = -EBUSY;
    }
    else
    {
        strobe_Res += 1;
    }


    spin_unlock_irq(&g_strobeSMPLock);
    PK_DBG("sub_strobe_open line=%d\n", __LINE__);

    return i4RetValue;

}


static int sub_strobe_release(void *pArg)
{
    PK_DBG(" sub_strobe_release\n");

    if (strobe_Res)
    {
        spin_lock_irq(&g_strobeSMPLock);

        strobe_Res = 0;
        strobe_Timeus = 0;

        /* LED On Status */
        g_strobe_On = FALSE;

        spin_unlock_irq(&g_strobeSMPLock);

    	FL_Uninit();
    }

    PK_DBG(" Done\n");

    return 0;

}


FLASHLIGHT_FUNCTION_STRUCT	subStrobeFunc=
{
	sub_strobe_open,
	sub_strobe_release,
	sub_strobe_ioctl
};


MUINT32 subStrobeInit(PFLASHLIGHT_FUNCTION_STRUCT *pfFunc)
{
    if (pfFunc != NULL)
    {
        *pfFunc = &subStrobeFunc;
    }
    return 0;
}



static int __init sub_flash_pmic_init(void)
{
	printk("sub_flash_pmic_init\n");

//	g_duty = 3;
//	FL_Enable();
//	gpio_flash_open();
	gpio_flash_close();
	return 0;
}

static void __exit sub_flash_pmic_exit(void)
{
	printk("sub_flash_pmic_exit\n");
}


//module_init(sub_flash_pmic_init);
late_initcall(sub_flash_pmic_init);
module_exit(sub_flash_pmic_exit);

MODULE_DESCRIPTION("sub Flash driver for PMIC");
MODULE_AUTHOR("feng.guangyue");
MODULE_LICENSE("GPL v2");

/*************aeon add for factory mode flashlight test*********/
void Flashlight2_ON(void)
{
	//hrtimer_cancel( &g_timeOutTimer );
	FL_dim_duty(1);
	if(0 == strobe_Res)
	{	
		FL_Init();	
	}
	if(flag1==1)
	{
		FL_Enable();
	}
}
void Flashlight2_OFF(void)
{	
	FL_Uninit();
}

EXPORT_SYMBOL(Flashlight2_ON);
EXPORT_SYMBOL(Flashlight2_OFF);
EXPORT_SYMBOL(Flashlight2_Switch);
/**************************end**********************/
