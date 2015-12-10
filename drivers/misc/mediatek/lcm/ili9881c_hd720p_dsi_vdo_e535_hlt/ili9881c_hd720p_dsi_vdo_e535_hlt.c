#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <mt_gpio.h>
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
#else
	#include <mach/mt_gpio.h>
#endif

#ifdef BUILD_LK
#include <stdio.h>
#include <string.h>
#else
#include <linux/string.h>
#include <linux/kernel.h>
#endif

//#define USE_LCM_THREE_LANE 1

#define _LCM_DEBUG_

#include <cust_gpio_usage.h>

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1280)

#define REGFLAG_DELAY             							0XFEFF
#define REGFLAG_END_OF_TABLE      							0xFFFF   // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE									0

#define LCM_ID_ILI9881C 0x9881
//#define GPIO_LCM_RST_PIN         (GPIO141 | 0x80000000)

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))
#define SET_GPIO_OUT(gpio_num,val)    						(lcm_util.set_gpio_out((gpio_num),(val)))


#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))



//#define _SYNA_INFO_
//#define _SYNA_DEBUG_
//#define _LCM_DEBUG_
//#define _LCM_INFO_
/*
#ifdef _LCM_DEBUG_
#define lcm_debug(fmt, args...) printk(fmt, ##args)
#else
#define lcm_debug(fmt, args...) do { } while (0)
#endif

#ifdef _LCM_INFO_
#define lcm_info(fmt, args...) printk(fmt, ##args)
#else
#define lcm_info(fmt, args...) do { } while (0)
#endif
#define lcm_err(fmt, args...) printk(fmt, ##args) */

#ifdef _LCM_DEBUG_
  #ifdef BUILD_LK
  #define LCM_PRINT printf
  #else
  #define LCM_PRINT printk
  #endif
#else
	#define LCM_PRINT
#endif

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   			lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    
       

static struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};

static struct LCM_setting_table lcm_initialization_setting[] = {
	
	/*
	Note :

	Data ID will depends on the following rule.
	
		count of parameters > 1	=> Data ID = 0x39
		count of parameters = 1	=> Data ID = 0x15
		count of parameters = 0	=> Data ID = 0x05

	Structure Format :

	{DCS command, count of parameters, {parameter list}}
	{REGFLAG_DELAY, milliseconds of time, {}},

	...

	Setting ending by predefined flag
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
	*/

#if 0
    {0XFF,    3,    {0X98, 0X81, 0X03}},
       {0X01,    1,    {0X00}},
    {0X02,    1,    {0X00}},
    {0X03,    1,    {0X53}},
    {0X04,    1,    {0X13}},
    {0X05,    1,    {0X13}},
    {0X06,    1,    {0X06}},
    {0X07,    1,    {0X00}},
    {0X08,    1,    {0X04}},
    {0X09,    1,    {0X04}},
    {0X0a,    1,    {0X03}},
    {0X0b,    1,    {0X03}},
    {0X0c,    1,    {0X00}},
    {0X0d,    1,    {0X00}},
    {0X0e,    1,    {0X00}},
    {0X0f,    1,    {0X04}},
    {0X10,    1,    {0X04}},
    {0X11,    1,    {0X00}},
    {0X12,    1,    {0X00}},
    {0X13,    1,    {0X00}},
    {0X14,    1,    {0X00}},
    {0X15,    1,    {0X00}},
    {0X16,    1,    {0X00}},
    {0X17,    1,    {0X00}},
    {0X18,    1,    {0X00}},
    {0X19,    1,    {0X00}},
    {0X1a,    1,    {0X00}},
    {0X1b,    1,    {0X00}},
    {0X1c,    1,    {0X00}},
    {0X1d,    1,    {0X00}},
    {0X1e,    1,    {0Xc0}},
    {0X1f,    1,    {0X80}},
    {0X20,    1,    {0X04}},
    {0X21,    1,    {0X0B}},
    {0X28,    1,    {0X55}},
    {0X29,    1,    {0X03}},
    {0X2c,    1,    {0X06}},
    {0X33,    1,    {0X30}},
    {0X34,    1,    {0X04}},
    {0X35,    1,    {0X05}},
    {0X36,    1,    {0X05}},
    {0X3a,    1,    {0X40}},
    {0X3b,    1,    {0X40}},
    {0X50,    1,    {0X01}},
    {0X51,    1,    {0X23}},
    {0X52,    1,    {0X45}},
    {0X53,    1,    {0X67}},
    {0X54,    1,    {0X89}},
    {0X55,    1,    {0XAB}},
    {0X56,    1,    {0X01}},
    {0X57,    1,    {0X23}},
    {0X58,    1,    {0X45}},
    {0X59,    1,    {0X67}},
    {0X5a,    1,    {0X89}},
    {0X5b,    1,    {0XAB}},
    {0X5c,    1,    {0XCD}},
    {0X5d,    1,    {0XEF}},
    {0X5e,    1,    {0X01}},
    {0X5f,    1,    {0X14}},
    {0X60,    1,    {0X15}},
    {0X61,    1,    {0X0C}},
    {0X62,    1,    {0X0D}},
    {0X63,    1,    {0X0E}},
    {0X64,    1,    {0X0F}},
    {0X65,    1,    {0X10}},
    {0X66,    1,    {0X11}},
    {0X67,    1,    {0X08}},
    {0X68,    1,    {0X02}},
    {0X69,    1,    {0X0A}},
    {0X6a,    1,    {0X02}},
    {0X6b,    1,    {0X02}},
    {0X6c,    1,    {0X02}},
    {0X6d,    1,    {0X02}},
    {0X6e,    1,    {0X02}},
    {0X6f,    1,    {0X02}},
    {0X70,    1,    {0X02}},
    {0X71,    1,    {0X02}},
    {0X72,    1,    {0X06}},
    {0X73,    1,    {0X02}},
    {0X74,    1,    {0X02}},
    {0X75,    1,    {0X14}},
    {0X76,    1,    {0X15}},
    {0X77,    1,    {0X11}},
    {0X78,    1,    {0X10}},
    {0X79,    1,    {0X0F}},
    {0X7a,    1,    {0X0E}},
    {0X7b,    1,    {0X0D}},
    {0X7c,    1,    {0X0C}},
    {0X7d,    1,    {0X06}},
    {0X7e,    1,    {0X02}},
    {0X7f,    1,    {0X0a}},
    {0X80,    1,    {0X02}},
    {0X81,    1,    {0X02}},
    {0X82,    1,    {0X02}},
    {0X83,    1,    {0X02}},
    {0X84,    1,    {0X02}},
    {0X85,    1,    {0X02}},
    {0X86,    1,    {0X02}},
    {0X87,    1,    {0X02}},
    {0X88,    1,    {0X08}},
    {0X89,    1,    {0X02}},
		{0X8a,    1,    {0X02}},
		{0xFF,    3,    {0X98, 0X81, 0X04}},
		{0X6c,    1,    {0X15}},
		{0X6e,    1,    {0X3b}},
		{0X6f,    1,    {0X53}},
		{0X3a,    1,    {0Xa4}},
		{0X8d,    1,    {0X15}},
		{0X87,    1,    {0Xba}},
		{0Xb2,    1,    {0Xd1}},
		{0X26,    1,    {0X76}},
		{0xFF,    3,    {0X98, 0X81, 0X01}},
		{0X22,    1,    {0X0A}},
		{0X31,    1,    {0X00}},
		{0X53,    1,    {0X79}},
		{0X55,    1,    {0X88}},
		{0X50,    1,    {0Xa6}},
		{0X51,    1,    {0Xa6}},
		{0X60,    1,    {0X14}},
		{0XA0,    1,    {0X08}},
		{0XA1,    1,    {0X27}},
		{0XA2,    1,    {0X36}},
		{0XA3,    1,    {0X15}},
		{0XA4,    1,    {0X17}},
		{0XA5,    1,    {0X2b}},
		{0XA6,    1,    {0X1e}},
		{0XA7,    1,    {0X1f}},
		{0XA8,    1,    {0X96}},
		{0XA9,    1,    {0X1c}},
		{0XAA,    1,    {0X28}},
		{0XAB,    1,    {0X7c}},
		{0XAC,    1,    {0X1b}},
		{0XAD,    1,    {0X1a}},
		{0XAE,    1,    {0X4d}},
		{0XAF,    1,    {0X23}},
		{0XB0,    1,    {0X29}},
		{0XB1,    1,    {0X4b}},
		{0XB2,    1,    {0X5a}},
		{0XB3,    1,    {0X2c}},
		{0XC0,    1,    {0X08}},
		{0XC1,    1,    {0X26}},
		{0XC2,    1,    {0X36}},
		{0XC3,    1,    {0X15}},
		{0XC4,    1,    {0X17}},
		{0XC5,    1,    {0X2b}},
		{0XC6,    1,    {0X1f}},
		{0XC7,    1,    {0X1f}},
		{0XC8,    1,    {0X96}},
		{0XC9,    1,    {0X1c}},
		{0XCA,    1,    {0X29}},
		{0XCB,    1,    {0X7c}},
		{0XCC,    1,    {0X1a}},
		{0XCD,    1,    {0X19}},
		{0XCE,    1,    {0X4d}},
		{0XCF,    1,    {0X22}},
		{0XD0,    1,    {0X29}},
		{0XD1,    1,    {0X4b}},
		{0XD2,    1,    {0X59}},
		{0XD3,    1,    {0X2c}},
#else

{0xFF,3,{0x98,0x81,0x03}},
	{0x01,1,{0x00}},
    {0x02,1,{0x00}},
    {0x03,1,{0x72}},
    {0x04,1,{0x00}},
    {0x05,1,{0x00}},
    {0x06,1,{0x09}},
	{0x07,1,{0x00}},
	{0x08,1,{0x00}},
	{0x09,1,{0x01}},
	{0x0A,1,{0x00}},
	{0x0B,1,{0x00}},
	{0x0C,1,{0x01}},
	{0x0D,1,{0x00}},
	{0x0E,1,{0x00}},
	{0x0F,1,{0x00}},//14
	{0x10,1,{0x00}},//14
	{0x11,1,{0x00}},
	{0x12,1,{0x00}},
	{0x13,1,{0x00}},
	{0x14,1,{0x00}},
	{0x15,1,{0x00}},
	{0x16,1,{0x00}},
	{0x17,1,{0x00}},
	{0x18,1,{0x00}},
	{0x19,1,{0x00}},
	{0x1A,1,{0x00}},
	{0x1B,1,{0x00}},
	{0x1C,1,{0x00}},
	{0x1D,1,{0x00}},
	{0x1E,1,{0x40}},
	{0x1F,1,{0x80}},
	{0x20,1,{0x05}},
	{0x21,1,{0x02}},
	{0x22,1,{0x00}},
	{0x23,1,{0x00}},
	{0x24,1,{0x00}},
	{0x25,1,{0x00}},
	{0x26,1,{0x00}},
	{0x27,1,{0x00}},
	{0x28,1,{0x33}},
	{0x29,1,{0x02}},
	{0x2A,1,{0x00}},
	{0x2B,1,{0x00}},
	{0x2C,1,{0x00}},
	{0x2D,1,{0x00}},
	{0x2E,1,{0x00}},
	{0x2F,1,{0x00}},
	{0x30,1,{0x00}},
	{0x31,1,{0x00}},
	{0x32,1,{0x00}},
	{0x33,1,{0x00}},
	{0x34,1,{0x04}},
	{0x35,1,{0x00}},
	{0x36,1,{0x00}},
	{0x37,1,{0x00}},
	{0x38,1,{0x3c}},//78
	{0x39,1,{0x00}},
	{0x3A,1,{0x40}},
	{0x3B,1,{0x40}},
	{0x3C,1,{0x00}},
	{0x3D,1,{0x00}},
	{0x3E,1,{0x00}},
	{0x3F,1,{0x00}},
	{0x40,1,{0x00}},
	{0x41,1,{0x00}},
	{0x42,1,{0x00}},
	{0x43,1,{0x00}},
	{0x44,1,{0x00}},




//GIP_2
{0x50, 1,{0x01}},
{0x51, 1,{0x23}},
{0x52, 1,{0x45}},
{0x53, 1,{0x67}},
{0x54, 1,{0x89}},
{0x55, 1,{0xAB}},
{0x56, 1,{0x01}},
{0x57, 1,{0x23}},
{0x58, 1,{0x45}},
{0x59, 1,{0x67}},
{0x5A, 1,{0x89}},
{0x5B, 1,{0xAB}},
{0x5C, 1,{0xCD}},
{0x5D, 1,{0xEF}},
 
//GIP_3
{0x5E, 1,{0x11}},
{0x5F, 1,{0x01}},
{0x60, 1,{0x00}},
{0x61, 1,{0x15}},
{0x62, 1,{0x14}},
{0x63, 1,{0x0E}},
{0x64, 1,{0x0F}},
{0x65, 1,{0x0C}},
{0x66, 1,{0x0D}},
{0x67, 1,{0x06}},
{0x68, 1,{0x02}},
{0x69, 1,{0x07}},
{0x6A, 1,{0x02}},
{0x6B, 1,{0x02}},
{0x6C, 1,{0x02}},
{0x6D, 1,{0x02}},
{0x6E, 1,{0x02}},
{0x6F, 1,{0x02}},
{0x70, 1,{0x02}},
{0x71, 1,{0x02}},
{0x72, 1,{0x02}},
{0x73, 1,{0x02}},
{0x74, 1,{0x02}},
{0x75, 1,{0x01}},
{0x76, 1,{0x00}},
{0x77, 1,{0x14}},
{0x78, 1,{0x15}},
{0x79, 1,{0x0E}},
{0x7A, 1,{0x0F}},
{0x7B, 1,{0x0C}},
{0x7C, 1,{0x0D}},
{0x7D, 1,{0x06}},
{0x7E, 1,{0x02}},
{0x7F, 1,{0x07}},
{0x80, 1,{0x02}},
{0x81, 1,{0x02}},
{0x82, 1,{0x02}},
{0x83, 1,{0x02}},
{0x84, 1,{0x02}},
{0x85, 1,{0x02}},
{0x86, 1,{0x02}},
{0x87, 1,{0x02}},
{0x88, 1,{0x02}},
{0x89, 1,{0x02}},
{0x8A, 1,{0x02}},

//CMD_PAgE 4
{0xFF, 3,{0x98,0x81,0x04}},
{0x00, 1,{0x80}},                 //0x00 for 3lane, 0x80 for 4lane 
{0x6C, 1,{0x15}},                //SEt VCORE voltAgE =1.5V
{0x6E, 1,{0x2A}},                //Di_pwr_rEg=0 For powEr moDE 2A //VGH ClAmp 15V
{0x6F, 1,{0xb4}},                // rEg vCl + pumping rAtio VGH=3x VGL=-2.5x
{0x3A, 1,{0x94}},                //POWER SAVING
{0x8D, 1,{0x15}},               //VGL ClAmp -11V
{0x87, 1,{0xBA}},               //ESD               
{0x26, 1,{0x76}},            
{0xB2, 1,{0xD1}},
{0xB5, 1,{0x06}},

//以下僅量測使用!!-----------------------------------------------------------------------------------------
//{0x8B, 1,{0xF3}},               //Di_vcom_nvref_test_en=1,bypass NVREF(Test_OUTN) to pad
//{0x87, 1,{0xFE}},             //Di_vref_sw_on=1,        Bypass VREF to ppad
//{0x7A, 1,{0x10}},               //DI_REG_REG1_EN_CAP=0,   Bypass VREG1/2 to ppad
//{0x21, 1,{0x30}},               //BlAnk BlACk
//-----------------------------------------------------------------------------------------




//CMD_PAgE 1
{0xFF,3,{0x98,0x81,0x01}},
	{0x22,1,{0x0A}},
	{0x31,1,{0x00}},
	{0x53,1,{0x8f}},
	{0x55,1,{0x8F}},
	{0x50,1,{0x9a}},//C0
	{0x51,1,{0x9a}},//C0
	{0x60,1,{0x22}},//08 fae 2f
	{0xA0,1,{0x08}},
	{0xA1,1,{0x11}},
	{0xA2,1,{0x1a}},
	{0xA3,1,{0x01}},
	{0xA4,1,{0x1e}},
	{0xA5,1,{0x2e}},
	{0xA6,1,{0x20}},
	{0xA7,1,{0x22}},
	{0xA8,1,{0x53}},
	{0xA9,1,{0x24}},
	{0xAA,1,{0x2f}},
	{0xAB,1,{0x4a}},
	{0xAC,1,{0x22}},
	{0xAD,1,{0x1d}},
	{0xAE,1,{0x52}},
	{0xAF,1,{0x28}},
	{0xB0,1,{0x25}},
	{0xB1,1,{0x49}},
	{0xB2,1,{0x63}},
	{0xB3,1,{0x39}},
	
	{0xC0,1,{0x08}},
	{0xC1,1,{0x15}},
	{0xC2,1,{0x1f}},
	{0xC3,1,{0x1a}},
	{0xC4,1,{0x03}},
	{0xC5,1,{0x18}},
	{0xC6,1,{0x0f}},
	{0xC7,1,{0x17}},
	{0xC8,1,{0x65}},
	{0xC9,1,{0x16}},
	{0xCA,1,{0x21}},
	{0xCB,1,{0x64}},
	{0xCC,1,{0x16}},
	{0xCD,1,{0x14}},
	{0xCE,1,{0x49}},
	{0xCF,1,{0x1b}},
	{0xD0,1,{0x2a}},
	{0xD1,1,{0x56}},
	{0xD2,1,{0x64}},
	{0xD3,1,{0x39}},

#endif
		
    {0xFF,    3,    {0x98, 0x81, 0x00}},
    {0X55,    1,    {0XB0}},
		

    {0x11,0,{}},
    {REGFLAG_DELAY, 800, {}},

    {0x29,0,{}},
    {REGFLAG_DELAY, 20, {}},

    {0x35,0,{}},

};

static struct LCM_setting_table lcm_set_window[] = {
	{0x2A,	4,	{0x00, 0x00, (FRAME_WIDTH>>8), (FRAME_WIDTH&0xFF)}},
	{0x2B,	4,	{0x00, 0x00, (FRAME_HEIGHT>>8), (FRAME_HEIGHT&0xFF)}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 1, {0x00}},
  {REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 10, {}},
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_sleep_in_setting[] = {
	// Display off sequence
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 10, {}},

    // Sleep Mode On
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;
	LCM_PRINT("%s %d\n", __func__,__LINE__);
    for(i = 0; i < count; i++) {
		
        unsigned cmd;
        cmd = table[i].cmd;
		
        switch (cmd) {
			
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
				
            case REGFLAG_END_OF_TABLE :
                break;
				
            default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
       	}
    }
	
}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
	LCM_PRINT("%s %d\n", __func__,__LINE__);
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
  	LCM_PRINT("junTang %s %d\n", __func__,__LINE__);
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type   = LCM_TYPE_DSI;

	params->width  = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

	// enable tearing-free
	 //params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
	 //params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

#if (LCM_DSI_CMD_MODE)
	params->dsi.mode   = CMD_MODE;
#else
	params->dsi.mode   = BURST_VDO_MODE;
//	params->dsi.mode   = SYNC_PULSE_VDO_MODE;	
#endif
	
	// DSI
	/* Command mode setting */
#ifdef USE_LCM_THREE_LANE
	params->dsi.LANE_NUM				= LCM_THREE_LANE; 
#else
	params->dsi.LANE_NUM				= LCM_FOUR_LANE;
#endif
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

	// Highly depends on LCD driver capability.
	// Not support in MT6573
	params->dsi.packet_size=256;

	// Video mode setting		
	params->dsi.intermediat_buffer_num = 0;

	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active				= 8;//2;//2;
	params->dsi.vertical_backporch					= 18;//14;
	params->dsi.vertical_frontporch					= 16;
	params->dsi.vertical_active_line				= FRAME_HEIGHT; 

	params->dsi.horizontal_sync_active				= 50;//2;//2;
	params->dsi.horizontal_backporch				= 74;//42;//44;//42;
	params->dsi.horizontal_frontporch				= 74;//44;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
#ifdef USE_LCM_THREE_LANE
    params->dsi.PLL_CLOCK = 250;//156;
#else
		params->dsi.PLL_CLOCK = 208;//156;
#endif
	
#if 1
    params->dsi.esd_check_enable =1;
	params->dsi.customization_esd_check_enable =1;
	params->dsi.lcm_esd_check_table[0].cmd = 0xAC;
	params->dsi.lcm_esd_check_table[0].count =1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x00;
#else
	params->dsi.cont_clock=1;
	params->dsi.clk_lp_per_line_enable = 0;
	params->dsi.esd_check_enable = 0;
	params->dsi.customization_esd_check_enable = 0;
	params->dsi.lcm_esd_check_table[0].cmd          = 0x53;
	params->dsi.lcm_esd_check_table[0].count        = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x24;
#endif
	params->dsi.ssc_disable = 1;
//	params->dsi.clk_lp_per_line_enable=1;
	// Bit rate calculation
	// params->dsi.pll_div1=0;		// fref=26MHz, fvco=fref*(div1+1)	(div1=0~63, fvco=500MHZ~1GHz)
	// params->dsi.pll_div2=1; 		// div2=0~15: fout=fvo/(2*div2)
	// params->dsi.fbk_div =11;    //fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)

}
	


static unsigned int lcm_compare_id(void)
{
	int array[4];
        char buffer[5];
        char id_high=0;
        char id_low=0;
        int id=0;

        SET_RESET_PIN(1);
        SET_RESET_PIN(0);
        MDELAY(25);       
        SET_RESET_PIN(1);
        MDELAY(50);      
       
        array[0]=0x00043902;
        array[1]=0x018198FF;
        dsi_set_cmdq(array, 2, 1);
        MDELAY(10);
 
        array[0]=0x00023700;
        dsi_set_cmdq(array, 1, 1);
    
        read_reg_v2(0x00, buffer,1);
        id_high = buffer[0];         //0x98
 
        read_reg_v2(0x01, buffer,1);
        id_low = buffer[0];          //0x81
 
        id =(id_high << 8) | id_low;

	LCM_PRINT("id ili9881c_hd720p_dsi_vdo_e535_hlt id_high= 0x%08x\n", id_high);
	LCM_PRINT("id ili9881c_hd720p_dsi_vdo_e535_hlt id_low= 0x%08x\n", id_low);
	LCM_PRINT("id ili9881c_hd720p_dsi_vdo_e535_hlt id= 0x%08x\n", id);

	return (id == LCM_ID_ILI9881C)?1:0;
        
}

static void lcm_init(void)
{
	unsigned int data_array[16];
	LCM_PRINT(" %s %d\n", __func__,__LINE__);
	SET_RESET_PIN(1);
	MDELAY(10);
    SET_RESET_PIN(0);
	MDELAY(20);//50
    SET_RESET_PIN(1);
	MDELAY(100);//100
  //lcm_compare_id();
	//lcm_init_registers();
	//data_array[0] = 0x00352500;
	//dsi_set_cmdq(&data_array, 1, 1);
	push_table(lcm_initialization_setting,sizeof(lcm_initialization_setting)/sizeof(lcm_initialization_setting[0]),1);
	push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_suspend(void)
{
//	LCM_PRINT("%s %d\n", __func__,__LINE__);
//	SET_GPIO_OUT(GPIO_LCM_RST_PIN,1); 	
//	MDELAY(1);	
//	SET_GPIO_OUT(GPIO_LCM_RST_PIN,1); 
    push_table(lcm_sleep_in_setting, sizeof(lcm_sleep_in_setting) / sizeof(struct LCM_setting_table), 1);
	SET_RESET_PIN(1);
	MDELAY(20);//50
    SET_RESET_PIN(0);
	MDELAY(20);//50
    SET_RESET_PIN(1);
	MDELAY(100);//100
//	push_table(lcm_sleep_in_setting, sizeof(lcm_sleep_in_setting) / sizeof(struct LCM_setting_table), 1);
    
//	SET_GPIO_OUT(GPIO_LCM_PWR_EN,0);//Disable LCM Power
}



static void lcm_resume(void)
{
//	LCM_PRINT("%s %d\n", __func__,__LINE__);
//	SET_GPIO_OUT(GPIO_LCM_RST_PIN,1);  //Enable LCM Power
	lcm_init();
//	push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	LCM_PRINT("%s %d\n", __func__,__LINE__);
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	data_array[3]= 0x00000000;
	data_array[4]= 0x00053902;
	data_array[5]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[6]= (y1_LSB);
	data_array[7]= 0x00000000;
	data_array[8]= 0x002c3909;

	dsi_set_cmdq(&data_array, 9, 0);

}

LCM_DRIVER ili9881c_hd720p_dsi_vdo_e535_hlt_lcm_drv = 
{
    .name			= "ili9881c_hd720p_dsi_vdo_e535_hlt",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
#if (LCM_DSI_CMD_MODE)
	.set_backlight	= lcm_setbacklight,
    .update         = lcm_update,
#endif
};
