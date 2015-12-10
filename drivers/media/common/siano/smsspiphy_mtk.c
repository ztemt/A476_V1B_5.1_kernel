#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/spi/spi.h>
#include <linux/dma-mapping.h>
#include <linux/irq.h>
#include <linux/interrupt.h>

#include "smscoreapi.h"

#include <linux/gpio.h>
#include "smsspicommon.h"

#include <mach/mt_spi.h>
#include <mach/eint.h>
#include <mach/mt_gpio.h>
#include <cust_gpio_boot.h>
#include <cust_eint.h>



/************************************************************/
/*Platform specific defaults - can be changes by parameters */
/*or in compilation at this section                         */
/************************************************************/

/*Host GPIO pin used for SMS interrupt*/
#define HOST_INTERRUPT_PIN 	135

/*Host SPI bus number used for SMS*/
#define HOST_SPI_BUS_NUM	0

/*Host SPI CS used by SPI bus*/
#define HOST_SPI_CS_NUM		0


/*Maximum SPI speed during download (may be lower tha active state)*/
#define MAX_SPEED_DURING_DOWNLOAD	6000000

/*Maximum SPI speed during active state*/
#define MAX_SPEED_DURING_WORK		6000000	

/*Default SMS device type connected to SPI bus.*/
#define DEFAULT_SMS_DEVICE_TYPE		 SMS_RIO	


extern struct device * siano_mm;


/*************************************/
/*End of platform specific parameters*/ 
/*************************************/



int host_spi_intr_pin = HOST_INTERRUPT_PIN;
int host_spi_bus = HOST_SPI_BUS_NUM;
int host_spi_cs = HOST_SPI_CS_NUM;
int spi_max_speed = MAX_SPEED_DURING_WORK;
int spi_default_type =DEFAULT_SMS_DEVICE_TYPE;

struct sms_spi {
	struct spi_device	*spi_dev;
	char			*zero_txbuf;
	dma_addr_t 		zero_txbuf_phy_addr;
	int 			bus_speed;
	void (*interruptHandler) (void *);
	void			*intr_context;
};

/*!
invert the endianness of a single 32it integer

\param[in]		u: word to invert

\return		the inverted word
*/
static inline u32 invert_bo(u32 u)
{
	return ((u & 0xff) << 24) | ((u & 0xff00) << 8) | ((u & 0xff0000) >> 8)
		| ((u & 0xff000000) >> 24);
}

/*!
invert the endianness of a data buffer

\param[in]		buf: buffer to invert
\param[in]		len: buffer length

\return		the inverted word
*/


static int invert_endianness(char *buf, int len)
{
#if 0
	int i;
	u32 *ptr = (u32 *) buf;

	len = (len + 3) / 4;
	for (i = 0; i < len; i++, ptr++)
	{
		*ptr = invert_bo(*ptr);
	}
#endif
	return 4 * ((len + 3) & (~3));
}

static irqreturn_t spibus_interrupt(int irq, void *context)
{
	struct sms_spi *sms_spi = (struct sms_spi *)context;
	sms_debug("SPI interrupt received.");
	return 0; 
	if (sms_spi->interruptHandler)
		sms_spi->interruptHandler(sms_spi->intr_context);
	return IRQ_HANDLED;
}


#ifdef CONFIG_MEDIATEK_SOLUTION


#define MT_GPIO_IRQ 		GPIO_CMMB_EINT_PIN
#define MT_GPIO_IRQ_M_EINT GPIO_CMMB_EINT_PIN_M_EINT

#define MT_GPIO_IRQ_NUM  CUST_EINT_CMMB_NUM

void mt_irq_set_mode(void)
{
	mt_set_gpio_mode(MT_GPIO_IRQ, MT_GPIO_IRQ_M_EINT);
	mt_set_gpio_dir(MT_GPIO_IRQ, GPIO_DIR_IN);
	//mt_set_gpio_pull_select(MT_GPIO_IRQ, GPIO_PULL_UP);
	mt_set_gpio_pull_enable(MT_GPIO_IRQ, GPIO_PULL_DISABLE);
}

static struct sms_spi *mt_sms_spi = NULL;
static void mt_spibus_interrupt(void)
{
	struct sms_spi *sms_spi = mt_sms_spi;
	//sms_info("sms SPI interrupt received.");
	if (sms_spi && sms_spi->interruptHandler)
		sms_spi->interruptHandler(sms_spi->intr_context);
}

static int mt_spi_setup(struct spi_device *spi)
{
	struct mt_chip_conf *chip_config;

	chip_config = (struct mt_chip_conf *)spi->controller_data;
	if (!chip_config) {
		sms_err("mt spi chip config null.\n");
		return -EINVAL;
	}
/*!
refet to mtk spi driver
SPI_MODULE_CLOCK 104000
spi clk = 	speed = SPI_MODULE_CLOCK / (chip_config->low_time + chip_config->high_time);
*/
	sms_debug("chip_config:%p\n", chip_config);


	chip_config->setuptime = 15;
	chip_config->holdtime =15;
	chip_config->high_time = 10;
	chip_config->low_time = 10;
	chip_config->cs_idletime = 20;
	chip_config->ulthgh_thrsh = 0;

	chip_config->cpol = 0;
	chip_config->cpha = 0;

	chip_config->rx_mlsb = 1;
	chip_config->tx_mlsb = 1;

	chip_config->tx_endian = 0;
	chip_config->rx_endian = 0;

	chip_config->com_mod = DMA_TRANSFER;
	chip_config->pause = 0;
	chip_config->finish_intr = 1;
	chip_config->deassert = 0;
	chip_config->ulthigh = 0;
	chip_config->tckdly = 0;

	sms_debug("com mode:%d, clk:%d KHz,CPOL%d,CPHA%d\n", chip_config->com_mod,
		(104000 / (chip_config->low_time + chip_config->high_time)), chip_config->cpol, chip_config->cpha);
	spi->bits_per_word = 32;
	return spi_setup(spi);

}


static int mt_spi_resetup(struct spi_device *spi)
{
#if 0
	struct mt_chip_conf *chip_config;

	chip_config = (struct mt_chip_conf *) spi->controller_data;
	if (!chip_config) {
		sms_err("mt spi chip config null.\n");
		return -EINVAL;
	}
	#if 1
    chip_config->setuptime = 6;
	chip_config->holdtime = 6;
	chip_config->high_time = 4;
	chip_config->low_time = 4;
	chip_config->cs_idletime = 8;
	#else
	chip_config->setuptime = 15;
	chip_config->holdtime = 15;
	chip_config->high_time = 10;
	chip_config->low_time = 10;
	chip_config->cs_idletime = 20;
	#endif
	sms_debug("mode:%d, clk:%d KHz,CPOL%d,CPHA%d\n", chip_config->com_mod,
		(104000 / (chip_config->low_time + chip_config->high_time)), chip_config->cpol, chip_config->cpha);
	return spi_setup(spi);
#else
	return 0;
#endif

}
static void mt_spi_set_transfer_mode(struct spi_device *spi, int mode)
{
	struct mt_chip_conf *chip_config;
		
	if(!spi || !spi->controller_data)
	{
		sms_err("spi config fail.");
		return;
	}
	
	chip_config = (struct mt_chip_conf *) spi->controller_data;

	if(1 == mode) {
		chip_config->com_mod = DMA_TRANSFER;
	} else {
		chip_config->com_mod = FIFO_TRANSFER;
	}
	
	if(spi_setup(spi)) {
		sms_err("spi_setup fail");
	}
}


#endif
void prepareForFWDnl(void *context)
{
	/*Reduce clock rate for FW download*/
	struct sms_spi *sms_spi = (struct sms_spi *)context;
	sms_spi->bus_speed = MAX_SPEED_DURING_DOWNLOAD;
	sms_spi->spi_dev->max_speed_hz = sms_spi->bus_speed;
#ifdef CONFIG_MEDIATEK_SOLUTION
	if(mt_spi_setup(sms_spi->spi_dev))
#else
	if(spi_setup(sms_spi->spi_dev))
#endif
	{
		sms_err("SMS device setup failed");
	}

	sms_debug ("Start FW download.");
	msleep(10);
	sms_debug ("done sleeping.");
}

void fwDnlComplete(void *context, int App)
{
	/*Set clock rate for working mode*/
	struct sms_spi *sms_spi = (struct sms_spi *)context;
	sms_spi->bus_speed = spi_max_speed;
	sms_spi->spi_dev->max_speed_hz = sms_spi->bus_speed;
#ifdef CONFIG_MEDIATEK_SOLUTION
	if(mt_spi_resetup(sms_spi->spi_dev))
#else
	if(spi_setup(sms_spi->spi_dev))
#endif
	{
		sms_err("SMS device setup failed");
	}
	sms_debug("FW download complete.");
	msleep(10);
}

//#define SMSSPI_DUMP
#ifdef SMSSPI_DUMP
static void smsspiphy_dump(char *label, unsigned char *buf, int len)
{
	int i;

	if (!buf || !len)
		return;
	sms_info("\t [%s|%d]\n\t", label, len);
	for(i = 0; i < len; i++) {
		printk("%02X ", buf[i]);
		if(i && !(i%16)) printk("\n\t");
	}
	sms_info("\n");
}
#endif

/*
* please note MTK SPI limited.  
* DMA mode: transfer size should be <= PACKET_SIZE, or multiple times of PACKET_SIZE
* fifo mode: transfer size should be < SPI_FIFO_SIZE
*/
#define PACKET_SIZE 0x400
#define SPI_FIFO_SIZE 32

void mt_spibus_xfer(void *context, unsigned char *txbuf,
		    unsigned long txbuf_phy_addr, unsigned char *rxbuf,
		    unsigned long rxbuf_phy_addr, int len)
{
	int transfer_len;
	int remainder;
	struct sms_spi *sms_spi = (struct sms_spi *)context;
	struct spi_message msg;
	struct spi_transfer xfer[2];

	memset(xfer, 0, sizeof(xfer));
	spi_message_init(&msg);
	msg.is_dma_mapped = 1;

	if (len < PACKET_SIZE) {
		transfer_len = len;
	} else {
		transfer_len = len & ~(PACKET_SIZE -1);
	}

	xfer[0].tx_buf = txbuf;
	xfer[0].rx_buf = rxbuf;
	xfer[0].len = transfer_len;
	xfer[0].tx_dma = txbuf_phy_addr;
	xfer[0].rx_dma = rxbuf_phy_addr;
	xfer[0].cs_change = 0;
	if (!txbuf) {
		xfer[0].tx_buf = sms_spi->zero_txbuf;
		xfer[0].tx_dma = sms_spi->zero_txbuf_phy_addr;
	}
	//sms_debug("[t0]: [0x%x] %d", transfer_len, transfer_len);
	spi_message_add_tail(&xfer[0], &msg);
	
	remainder = len - transfer_len;

	if (remainder > 0) {
		xfer[1].tx_buf = txbuf ? (txbuf + transfer_len) : NULL;
		xfer[1].rx_buf = rxbuf ? (rxbuf + transfer_len) : NULL;
		xfer[1].len = remainder;
		xfer[1].tx_dma = txbuf_phy_addr ? (txbuf_phy_addr + transfer_len) : NULL;
		xfer[1].rx_dma = rxbuf_phy_addr ? (rxbuf_phy_addr + transfer_len) : NULL;
		xfer[1].cs_change = 0;
		if (!txbuf) {
			xfer[1].tx_buf = sms_spi->zero_txbuf + transfer_len;
			xfer[1].tx_dma = sms_spi->zero_txbuf_phy_addr + transfer_len;
		}
		//sms_debug("[t1]: [0x%x] %d", remainder, remainder);
		spi_message_add_tail(&xfer[1], &msg);

	}
	spi_sync(sms_spi->spi_dev, &msg);

}


void smsspibus_xfer_dma(void *context, unsigned char *txbuf,
		    unsigned long txbuf_phy_addr, unsigned char *rxbuf,
		    unsigned long rxbuf_phy_addr, int len)
{
	mt_spibus_xfer(context, txbuf, txbuf_phy_addr, rxbuf, rxbuf_phy_addr, len);
}


void smsspibus_xfer_fifo(void *context, unsigned char *txbuf,
		    unsigned long txbuf_phy_addr, unsigned char *rxbuf,
		    unsigned long rxbuf_phy_addr, int len)
{
	int transfered_len;
	for (transfered_len = 0; transfered_len < len; ) {
		mt_spibus_xfer(context, txbuf + transfered_len, txbuf_phy_addr + transfered_len,\
			rxbuf + transfered_len, rxbuf_phy_addr + transfered_len, \
			min(SPI_FIFO_SIZE, (len - transfered_len)));
		transfered_len += SPI_FIFO_SIZE;
	}
}

void smsspibus_xfer(void *context, unsigned char *txbuf,
		    unsigned long txbuf_phy_addr, unsigned char *rxbuf,
		    unsigned long rxbuf_phy_addr, int len)
{
	struct sms_spi *sms_spi = (struct sms_spi *)context;

	//sms_debug("tx: %p, rx: %p, len: %d\n", txbuf, rxbuf, len);
	
	if (txbuf) {
		invert_endianness(txbuf, len);
	}
#ifdef SMSSPI_DUMP
	smsspiphy_dump("TX", txbuf, 32);
#endif
#if 0
	if (txbuf) {
		mt_spi_set_transfer_mode(sms_spi->spi_dev, 0);
		smsspibus_xfer_fifo(context, txbuf, txbuf_phy_addr, rxbuf, rxbuf_phy_addr, len);

	} else {
		mt_spi_set_transfer_mode(sms_spi->spi_dev, 1);
		smsspibus_xfer_dma(context, txbuf, txbuf_phy_addr, rxbuf, rxbuf_phy_addr, len);
	}
#else
	mt_spibus_xfer(context, txbuf, txbuf_phy_addr, rxbuf, rxbuf_phy_addr, len);
#endif
#ifdef SMSSPI_DUMP
	smsspiphy_dump("RX", rxbuf, len);
#endif
	invert_endianness(rxbuf, len);

}

static int device_exits = 0;
void sms_chip_set_exists(int isexits)
{
	device_exits = isexits;
}

int sms_chip_get_exists(void)
{
	return device_exits;
}

int smsspiphy_is_device_exists(void)
{
#if 0
	int i = 0;

	/* Check 3 times if the interrupt pin is 0. if it never goes down - 
	there is not SPI device,*/
	for (i = 0; i < 3; i++)
	{
		if (gpio_get_value(host_spi_intr_pin) == 0)
		{
			return 1;
		}
		msleep(1);
	}
	return 0;
#else
	return sms_chip_get_exists();

#endif
}


#if defined(CONFIG_MEDIATEK_SOLUTION)
struct mt_chip_conf smsspi_chip = {
	};

#endif

int smsspiphy_init(void *context)
{
	struct sms_spi *sms_spi = (struct sms_spi *)context;
	int ret;
	struct spi_device *sms_device;

	struct spi_master *master = spi_busnum_to_master(host_spi_bus);
	struct spi_board_info sms_chip = {
		.modalias = "SmsSPI",
		.platform_data 	= NULL,
#if defined(CONFIG_MEDIATEK_SOLUTION)
		.controller_data = &smsspi_chip,
#else
		.controller_data = NULL,
#endif
		.irq		= 0, 
		.max_speed_hz	= MAX_SPEED_DURING_DOWNLOAD,
		.bus_num	= host_spi_bus,
		.chip_select 	= host_spi_cs,
		.mode		= SPI_MODE_0,
	};
	if (!master)
	{
		sms_err("Could not find SPI master device.");
		ret = -ENODEV;
		goto no_spi_master;
	}
	

	sms_device = spi_new_device(master, &sms_chip);	
	if (!sms_device)
	{
		sms_err("Failed on allocating new SPI device for SMS");
		ret = -ENODEV;
		goto no_spi_master;
	}

	sms_device->bits_per_word = 32;
#ifdef CONFIG_MEDIATEK_SOLUTION
	if(mt_spi_setup(sms_device))
#else
	if(spi_setup(sms_device))
#endif
	{
		sms_err("SMS device setup failed");
		ret = -ENODEV;
		goto spi_setup_failed;
	}

	sms_spi->spi_dev = sms_device;
	sms_spi->bus_speed = MAX_SPEED_DURING_DOWNLOAD;
	sms_debug("after init sms_spi=%p, spi_dev = %p", sms_spi, sms_spi->spi_dev);

	return 0;

spi_setup_failed:
	spi_unregister_device(sms_device);
no_spi_master:
	return ret;
}

void smsspiphy_deinit(void *context)
{
	struct sms_spi *sms_spi = (struct sms_spi *)context;
	sms_debug("smsspiphy_deinit\n");
	/*Release the SPI device*/
	spi_unregister_device(sms_spi->spi_dev);
	sms_spi->spi_dev = NULL;

}

void *smsspiphy_register(void *context, void (*smsspi_interruptHandler) (void *), 
		     void *intr_context)
{
	int ret;
	struct sms_spi *sms_spi; 
	sms_spi = kzalloc(sizeof(struct sms_spi), GFP_KERNEL);
	if (!sms_spi)
	{
		sms_err("SMS device mempory allocating");
		goto memory_allocate_failed;

	}

	sms_spi->zero_txbuf =  dma_alloc_coherent(siano_mm, ZERO_TXFUT_SIZE,
			       &sms_spi->zero_txbuf_phy_addr,
			       GFP_KERNEL | GFP_DMA);
	if (!sms_spi->zero_txbuf) {
		sms_err ("dma_alloc_coherent(...) failed\n");
		goto dma_allocate_failed;
	}
	sms_debug("dma_alloc_coherent(zero_txbuf): %p, size:%d\n", sms_spi->zero_txbuf, ZERO_TXFUT_SIZE);
	memset (sms_spi->zero_txbuf, 0, ZERO_TXFUT_SIZE);
	sms_spi->interruptHandler = smsspi_interruptHandler;
	sms_spi->intr_context = intr_context;

#ifdef CONFIG_MEDIATEK_SOLUTION
	mt_sms_spi = sms_spi;
	mt_irq_set_mode();
	//mt_eint_set_hw_debounce(MT_GPIO_IRQ_NUM, 0);
	mt_eint_registration(MT_GPIO_IRQ_NUM, EINTF_TRIGGER_FALLING, mt_spibus_interrupt, 1);
	//mt_eint_unmask(MT_GPIO_IRQ_NUM);
	sms_debug("mtk irq register.\n");
#else

	if (gpio_request(host_spi_intr_pin, "SMSSPI"))
	{
		sms_err("Could not get GPIO for SMS device intr.\n");
		goto request_gpio_failed;
	}
	gpio_direction_input(host_spi_intr_pin);
	gpio_export(host_spi_intr_pin, 1);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0)
	irq_set_irq_type(gpio_to_irq(host_spi_intr_pin), IRQ_TYPE_EDGE_FALLING);
#else
	set_irq_type(gpio_to_irq(host_spi_intr_pin), IRQ_TYPE_EDGE_FALLING);
#endif
	ret = request_irq(gpio_to_irq(host_spi_intr_pin), spibus_interrupt, IRQF_TRIGGER_FALLING, "SMSSPI", sms_spi);
	if (ret) {
		sms_err("Could not get interrupt for SMS device. status =%d\n", ret);
		goto request_irq_failed;
	}
#endif
	return sms_spi;

request_irq_failed:
	gpio_free(host_spi_intr_pin);

request_gpio_failed:
	dma_free_coherent(NULL, SPI_PACKET_SIZE, sms_spi->zero_txbuf, sms_spi->zero_txbuf_phy_addr);
dma_allocate_failed:
	kfree(sms_spi);
memory_allocate_failed:
	return NULL;
}

void smsspiphy_unregister(void *context)
{
	struct sms_spi *sms_spi = (struct sms_spi *)context;
#ifdef CONFIG_MEDIATEK_SOLUTION
	mt_eint_mask(MT_GPIO_IRQ_NUM);
	mt_eint_registration(MT_GPIO_IRQ_NUM, EINTF_TRIGGER_FALLING, NULL, 0);
#else
	/*Release the IRQ line*/
	free_irq(gpio_to_irq(host_spi_intr_pin), sms_spi);
	/*Release the GPIO lines*/
	gpio_free(host_spi_intr_pin);
#endif
	/*Release the DMA buffer*/
	dma_free_coherent(NULL, SPI_PACKET_SIZE, sms_spi->zero_txbuf, sms_spi->zero_txbuf_phy_addr);
	/*Release memory*/
	kfree(sms_spi);
}

void smschipreset(void *context)
{
	msleep(100);
}


