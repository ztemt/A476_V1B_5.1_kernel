/*
** $Id: @(#) gl_bow.c@@
*/

/*! \file   gl_bow.c
    \brief  Main routines of Linux driver interface for 802.11 PAL (BT 3.0 + HS)

    This file contains the main routines of Linux driver for MediaTek Inc. 802.11
    Wireless LAN Adapters.
*/

/*
** $Log: gl_bow.c $
**
** 07 30 2013 yuche.tsai
** [BORA00002398] [MT6630][Volunteer Patch] P2P Driver Re-Design for Multiple BSS support
** Temp fix Hot-spot data path issue.
**
** 07 26 2013 terry.wu
** [BORA00002207] [MT6630 Wi-Fi] TXM & MQM Implementation
** 1. Reduce extra Tx frame header parsing
** 2. Add TX port control
** 3. Add net interface to BSS binding
**
** 01 23 2013 terry.wu
** [BORA00002207] [MT6630 Wi-Fi] TXM & MQM Implementation
** Refine net dev implementation
**
** 01 21 2013 terry.wu
** [BORA00002207] [MT6630 Wi-Fi] TXM & MQM Implementation
** Update TX path based on new ucBssIndex modifications.
**
** 01 17 2013 cm.chang
** [BORA00002149] [MT6630 Wi-Fi] Initial software development
** Use ucBssIndex to replace eNetworkTypeIndex
**
** 09 17 2012 cm.chang
** [BORA00002149] [MT6630 Wi-Fi] Initial software development
** Duplicate source from MT6620 v2.3 driver branch
** (Davinci label: MT6620_WIFI_Driver_V2_3_120913_1942_As_MT6630_Base)
**
** 07 24 2012 yuche.tsai
** NULL
** Bug fix for JB.
 *
 * 02 16 2012 chinghwa.yu
 * [WCXRP00000065] Update BoW design and settings
 * [ALPS00235223] [Rose][ICS][Cross Feature][AEE-IPANIC]The device reboot automatically and then the "KE" pops up after you turn on the "Airplane mode".(once)
 *
 * [Root Cause]
 * PAL operates BOW char dev poll after BOW char dev is registered.
 *
 * [Solution]
 * Rejects PAL char device operation after BOW is unregistered or when wlan GLUE_FLAG_HALT is set.
 *
 * This is a workaround for BOW driver robustness, happens only in ICS.
 *
 * Root cause should be fixed by CR [ALPS00231570]
 *
 * 02 03 2012 chinghwa.yu
 * [WCXRP00000065] Update BoW design and settings
 * [ALPS00118114] [Rose][ICS][Free Test][Bluetooth]The "KE" pops up after you turn on the airplane mode.(5/5)
 *
 * [Root Cause]
 * PAL operates BOW char dev poll after BOW char dev is registered.
 *
 * [Solution]
 * Rejects PAL char device operation after BOW is unregistered.
 *
 * Happens only in ICS.
 *
 * Notified PAL owener to reivew MTKBT/PAL closing BOW char dev procedure.
 *
 * [Side Effect]
 * None.
 *
 * 01 16 2012 chinghwa.yu
 * [WCXRP00000065] Update BoW design and settings
 * Support BOW for 5GHz band.
 *
 * 11 10 2011 cp.wu
 * [WCXRP00001098] [MT6620 Wi-Fi][Driver] Replace printk by DBG LOG macros in linux porting layer
 * 1. eliminaite direct calls to printk in porting layer.
 * 2. replaced by DBGLOG, which would be XLOG on ALPS platforms.
 *
 * 10 25 2011 chinghwa.yu
 * [WCXRP00000065] Update BoW design and settings
 * Modify ampc0 char device for major number 151 for all MT6575 projects.
 *
 * 07 28 2011 cp.wu
 * [WCXRP00000884] [MT6620 Wi-Fi][Driver] Deprecate ioctl interface by unlocked ioctl
 * unlocked_ioctl returns as long instead of int.
 *
 * 07 28 2011 cp.wu
 * [WCXRP00000884] [MT6620 Wi-Fi][Driver] Deprecate ioctl interface by unlocked ioctl
 * migrate to unlocked ioctl interface
 *
 * 04 12 2011 chinghwa.yu
 * [WCXRP00000065] Update BoW design and settings
 * Add WMM IE for BOW initiator data.
 *
 * 04 10 2011 chinghwa.yu
 * [WCXRP00000065] Update BoW design and settings
 * Change Link disconnection event procedure for hotspot and change skb length check to 1514 bytes.
 *
 * 04 09 2011 chinghwa.yu
 * [WCXRP00000065] Update BoW design and settings
 * Change Link connection event procedure and change skb length check to 1512 bytes.
 *
 * 03 27 2011 chinghwa.yu
 * [WCXRP00000065] Update BoW design and settings
 * Support multiple physical link.
 *
 * 03 06 2011 chinghwa.yu
 * [WCXRP00000065] Update BoW design and settings
 * Sync BOW Driver to latest person development branch version..
 *
 * 03 03 2011 jeffrey.chang
 * [WCXRP00000512] [MT6620 Wi-Fi][Driver] modify the net device relative functions to support the H/W multiple queue
 * support concurrent network
 *
 * 03 03 2011 jeffrey.chang
 * [WCXRP00000512] [MT6620 Wi-Fi][Driver] modify the net device relative functions to support the H/W multiple queue
 * replace alloc_netdev to alloc_netdev_mq for BoW
 *
 * 03 03 2011 jeffrey.chang
 * [WCXRP00000512] [MT6620 Wi-Fi][Driver] modify the net device relative functions to support the H/W multiple queue
 * modify net device relative functions to support multiple H/W queues
 *
 * 02 15 2011 chinghwa.yu
 * [WCXRP00000065] Update BoW design and settings
 * Update net register and BOW for concurrent features.
 *
 * 02 10 2011 chinghwa.yu
 * [WCXRP00000065] Update BoW design and settings
 * Fix kernel API change issue.
 * Before ALPS 2.2 (2.2 included), kfifo_alloc() is
 * struct kfifo *kfifo_alloc(unsigned int size, gfp_t gfp_mask, spinlock_t *lock);
 * After ALPS 2.3, kfifo_alloc() is changed to
 * int kfifo_alloc(struct kfifo *fifo, unsigned int size, gfp_t gfp_mask);
 *
 * 02 09 2011 cp.wu
 * [WCXRP00000430] [MT6620 Wi-Fi][Firmware][Driver] Create V1.2 branch for MT6620E1 and MT6620E3
 * create V1.2 driver branch based on label MT6620_WIFI_DRIVER_V1_2_110209_1031
 * with BOW and P2P enabled as default
 *
 * 02 08 2011 chinghwa.yu
 * [WCXRP00000065] Update BoW design and settings
 * Replace kfifo_get and kfifo_put with kfifo_out and kfifo_in.
 * Update BOW get MAC status, remove returning event for AIS network type.
 *
 * 01 12 2011 cp.wu
 * [WCXRP00000357] [MT6620 Wi-Fi][Driver][Bluetooth over Wi-Fi] add another net device interface for BT AMP
 * implementation of separate BT_OVER_WIFI data path.
 *
 * 01 12 2011 cp.wu
 * [WCXRP00000356] [MT6620 Wi-Fi][Driver] fill mac header length for security frames 'cause hardware header translation needs such information
 * fill mac header length information for 802.1x frames.
 *
 * 11 11 2010 chinghwa.yu
 * [WCXRP00000065] Update BoW design and settings
 * Fix BoW timer assert issue.
 *
 * 09 14 2010 chinghwa.yu
 * NULL
 * Add bowRunEventAAAComplete.
 *
 * 09 14 2010 cp.wu
 * NULL
 * correct typo: POLLOUT instead of POLL_OUT
 *
 * 09 13 2010 cp.wu
 * NULL
 * add waitq for poll() and read().
 *
 * 08 24 2010 chinghwa.yu
 * NULL
 * Update BOW for the 1st time.
 *
 * 07 08 2010 cp.wu
 *
 * [WPD00003833] [MT6620 and MT5931] Driver migration - move to new repository.
 *
 * 06 06 2010 kevin.huang
 * [WPD00003832][MT6620 5931] Create driver base
 * [MT6620 5931] Create driver base
 *
 * 05 05 2010 cp.wu
 * [WPD00003823][MT6620 Wi-Fi] Add Bluetooth-over-Wi-Fi support
 * change variable names for multiple physical link to match with coding convention
 *
 * 05 05 2010 cp.wu
 * [WPD00003823][MT6620 Wi-Fi] Add Bluetooth-over-Wi-Fi support
 * multiple BoW interfaces need to compare with peer address
 *
 * 04 28 2010 cp.wu
 * [WPD00003823][MT6620 Wi-Fi] Add Bluetooth-over-Wi-Fi support
 * change prefix for data structure used to communicate with 802.11 PAL
 * to avoid ambiguous naming with firmware interface
 *
 * 04 28 2010 cp.wu
 * [WPD00003823][MT6620 Wi-Fi] Add Bluetooth-over-Wi-Fi support
 * fix kalIndicateBOWEvent.
 *
 * 04 27 2010 cp.wu
 * [WPD00003823][MT6620 Wi-Fi] Add Bluetooth-over-Wi-Fi support
 * add multiple physical link support
 *
 * 04 13 2010 cp.wu
 * [WPD00003823][MT6620 Wi-Fi] Add Bluetooth-over-Wi-Fi support
 * add framework for BT-over-Wi-Fi support.
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  * 1) prPendingCmdInfo is replaced by queue for multiple handler capability
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  * 2) command sequence number is now increased atomically
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  * 3) private data could be hold and taken use for other purpose
**
*/

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include "gl_os.h"
#include "debug.h"
#include "wlan_lib.h"
#include "gl_wext.h"
#include "precomp.h"
#include <linux/poll.h>
#include "bss.h"

#if CFG_ENABLE_BT_OVER_WIFI

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
/* @FIXME if there is command/event with payload length > 28 */
#define MAX_BUFFER_SIZE         (64)

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

UINT_32 g_u4PrevSysTime = 0;
UINT_32 g_u4CurrentSysTime = 0;
UINT_32 g_arBowRevPalPacketTime[32];

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/* forward declarations */
static ssize_t
bow_ampc_read(IN struct file *filp, IN char __user *buf, IN size_t size, IN OUT loff_t *ppos);

static ssize_t
bow_ampc_write(IN struct file *filp,
	       OUT const char __user *buf, IN size_t size, IN OUT loff_t *ppos);

static long bow_ampc_ioctl(IN struct file *filp, IN unsigned int cmd, IN OUT unsigned long arg);

static unsigned int bow_ampc_poll(IN struct file *filp, IN poll_table * wait);

static int bow_ampc_open(IN struct inode *inodep, IN struct file *filp);

static int bow_ampc_release(IN struct inode *inodep, IN struct file *filp);


/* character file operations */
static const struct file_operations bow_ampc_fops = {
	/* .owner              = THIS_MODULE, */
	.read = bow_ampc_read,
	.write = bow_ampc_write,
	.unlocked_ioctl = bow_ampc_ioctl,
	.poll = bow_ampc_poll,
	.open = bow_ampc_open,
	.release = bow_ampc_release,
};

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/


/*----------------------------------------------------------------------------*/
/*!
* \brief Register for character device to communicate with 802.11 PAL
*
* \param[in] prGlueInfo      Pointer to glue info
*
* \return   TRUE
*           FALSE
*/
/*----------------------------------------------------------------------------*/
BOOLEAN glRegisterAmpc(IN P_GLUE_INFO_T prGlueInfo)
{
	ASSERT(prGlueInfo);

	DBGLOG(BOW, INFO, ("Register for character device to communicate with 802.11 PAL.\n"));

	if (prGlueInfo->rBowInfo.fgIsRegistered == TRUE) {
		return FALSE;
	} else {
#if 0
		/* 1. allocate major number dynamically */

		if (alloc_chrdev_region(&(prGlueInfo->rBowInfo.u4DeviceNumber), 0,	/* first minor number */
					1,	/* number */
					GLUE_BOW_DEVICE_NAME) != 0)

			return FALSE;
#endif

#if 1

#if defined(CONFIG_AMPC_CDEV_NUM)
		prGlueInfo->rBowInfo.u4DeviceNumber = MKDEV(CONFIG_AMPC_CDEV_NUM, 0);
#else
		prGlueInfo->rBowInfo.u4DeviceNumber = MKDEV(226, 0);
#endif

		if (register_chrdev_region(prGlueInfo->rBowInfo.u4DeviceNumber, 1,	/* number */
					   GLUE_BOW_DEVICE_NAME) != 0)

			return FALSE;
#endif

		/* 2. spin-lock initialization */
		/* spin_lock_init(&(prGlueInfo->rBowInfo.rSpinLock)); */

		/* 3. initialize kfifo */
/*        prGlueInfo->rBowInfo.prKfifo = kfifo_alloc(GLUE_BOW_KFIFO_DEPTH,
		GFP_KERNEL,
		&(prGlueInfo->rBowInfo.rSpinLock));*/
		if ((kfifo_alloc
		     ((struct kfifo *)&(prGlueInfo->rBowInfo.rKfifo), GLUE_BOW_KFIFO_DEPTH,
		      GFP_KERNEL)))
			goto fail_kfifo_alloc;

/* if(prGlueInfo->rBowInfo.prKfifo == NULL) */
		if (&(prGlueInfo->rBowInfo.rKfifo) == NULL)
			goto fail_kfifo_alloc;

		/* 4. initialize cdev */
		cdev_init(&(prGlueInfo->rBowInfo.cdev), &bow_ampc_fops);
		/* prGlueInfo->rBowInfo.cdev.owner = THIS_MODULE; */
		prGlueInfo->rBowInfo.cdev.ops = &bow_ampc_fops;

		/* 5. add character device */
		if (cdev_add(&(prGlueInfo->rBowInfo.cdev), prGlueInfo->rBowInfo.u4DeviceNumber, 1))
			goto fail_cdev_add;


		/* 6. in queue initialization */
		init_waitqueue_head(&(prGlueInfo->rBowInfo.outq));

		/* 7. finish */
		prGlueInfo->rBowInfo.fgIsRegistered = TRUE;
		return TRUE;

fail_cdev_add:
		kfifo_free(&(prGlueInfo->rBowInfo.rKfifo));
/* kfifo_free(prGlueInfo->rBowInfo.prKfifo); */
fail_kfifo_alloc:
		unregister_chrdev_region(prGlueInfo->rBowInfo.u4DeviceNumber, 1);
		return FALSE;
	}
}				/* end of glRegisterAmpc */


/*----------------------------------------------------------------------------*/
/*!
* \brief Unregister character device for communicating with 802.11 PAL
*
* \param[in] prGlueInfo      Pointer to glue info
*
* \return   TRUE
*           FALSE
*/
/*----------------------------------------------------------------------------*/
BOOLEAN glUnregisterAmpc(IN P_GLUE_INFO_T prGlueInfo)
{
	ASSERT(prGlueInfo);

	DBGLOG(BOW, INFO, ("Unregister character device for communicating with 802.11 PAL.\n"));

	if (prGlueInfo->rBowInfo.fgIsRegistered == FALSE) {
		return FALSE;
	} else {
		prGlueInfo->rBowInfo.fgIsRegistered = FALSE;

		/* 1. free netdev if necessary */
#if CFG_BOW_SEPARATE_DATA_PATH
		kalUninitBowDevice(prGlueInfo);
#endif

		/* 2. removal of character device */
		cdev_del(&(prGlueInfo->rBowInfo.cdev));

		/* 3. free kfifo */
/* kfifo_free(prGlueInfo->rBowInfo.prKfifo); */
		kfifo_free(&(prGlueInfo->rBowInfo.rKfifo));
/* prGlueInfo->rBowInfo.prKfifo = NULL; */
/* prGlueInfo->rBowInfo.rKfifo = NULL; */

		/* 4. free device number */
		unregister_chrdev_region(prGlueInfo->rBowInfo.u4DeviceNumber, 1);

		return TRUE;
	}
}				/* end of glUnregisterAmpc */


/*----------------------------------------------------------------------------*/
/*!
* \brief read handler for character device to communicate with 802.11 PAL
*
* \param[in]
* \return
*           Follows Linux Character Device Interface
*
*/
/*----------------------------------------------------------------------------*/
static ssize_t
bow_ampc_read(IN struct file *filp, IN char __user *buf, IN size_t size, IN OUT loff_t *ppos)
{
	UINT_8 aucBuffer[MAX_BUFFER_SIZE];
	ssize_t retval;

	P_GLUE_INFO_T prGlueInfo;
	prGlueInfo = (P_GLUE_INFO_T) (filp->private_data);

	ASSERT(prGlueInfo);

	DBGLOG(BOW, INFO, ("BoW EVENT read.\n"));

	if ((prGlueInfo->rBowInfo.fgIsRegistered == FALSE) || (prGlueInfo->ulFlag & GLUE_FLAG_HALT)) {
		return -EFAULT;
	}
	/* size check */
/* if(kfifo_len(prGlueInfo->rBowInfo.prKfifo) >= size) */
	if (kfifo_len(&(prGlueInfo->rBowInfo.rKfifo)) >= size)
		retval = size;
	else
		retval = kfifo_len(&(prGlueInfo->rBowInfo.rKfifo));
/* retval = kfifo_len(prGlueInfo->rBowInfo.prKfifo); */

/* kfifo_get(prGlueInfo->rBowInfo.prKfifo, aucBuffer, retval); */
/* kfifo_out(prGlueInfo->rBowInfo.prKfifo, aucBuffer, retval); */
	if (!(kfifo_out(&(prGlueInfo->rBowInfo.rKfifo), aucBuffer, retval)))
		retval = -EIO;

	if (copy_to_user(buf, aucBuffer, retval))
		retval = -EIO;

	return retval;
}


/*----------------------------------------------------------------------------*/
/*!
* \brief write handler for character device to communicate with 802.11 PAL
*
* \param[in]
* \return
*           Follows Linux Character Device Interface
*
*/
/*----------------------------------------------------------------------------*/
static ssize_t
bow_ampc_write(IN struct file *filp,
	       OUT const char __user *buf, IN size_t size, IN OUT loff_t *ppos)
{
	UINT_8 i;

	UINT_8 aucBuffer[MAX_BUFFER_SIZE];
	P_AMPC_COMMAND prCmd;
	P_GLUE_INFO_T prGlueInfo;

	prGlueInfo = (P_GLUE_INFO_T) (filp->private_data);
	ASSERT(prGlueInfo);

	if ((prGlueInfo->rBowInfo.fgIsRegistered == FALSE) || (prGlueInfo->ulFlag & GLUE_FLAG_HALT)) {
		return -EFAULT;
	}

	if (size > MAX_BUFFER_SIZE)
		return -EINVAL;
	else if (copy_from_user(aucBuffer, buf, size))
		return -EIO;

	DBGLOG(BOW, EVENT, ("AMP driver CMD buffer size : %d.\n", size));

	for (i = 0; i < MAX_BUFFER_SIZE; i++) {
		DBGLOG(BOW, EVENT, ("AMP write content : 0x%x.\n", aucBuffer[i]));
	}

	DBGLOG(BOW, EVENT, ("BoW CMD write.\n"));

	prCmd = (P_AMPC_COMMAND) aucBuffer;

	DBGLOG(BOW, EVENT,
	       ("AMP write content payload length : %d.\n", prCmd->rHeader.u2PayloadLength));

	DBGLOG(BOW, EVENT,
	       ("AMP write content header length : %d.\n", sizeof(AMPC_COMMAND_HEADER_T)));

	/* size check */
	if (prCmd->rHeader.u2PayloadLength + sizeof(AMPC_COMMAND_HEADER_T) != size) {
		DBGLOG(BOW, EVENT, ("Wrong CMD total length.\n"));

		return -EINVAL;
	}

	if (wlanbowHandleCommand(prGlueInfo->prAdapter, prCmd) == WLAN_STATUS_SUCCESS)
		return size;
	else
		return -EINVAL;
}


/*----------------------------------------------------------------------------*/
/*!
* \brief ioctl handler for character device to communicate with 802.11 PAL
*
* \param[in]
* \return
*           Follows Linux Character Device Interface
*
*/
/*----------------------------------------------------------------------------*/
static long bow_ampc_ioctl(IN struct file *filp, IN unsigned int cmd, IN OUT unsigned long arg)
{
	int err = 0;
	P_GLUE_INFO_T prGlueInfo;
	prGlueInfo = (P_GLUE_INFO_T) (filp->private_data);

	ASSERT(prGlueInfo);

	if ((prGlueInfo->rBowInfo.fgIsRegistered == FALSE) || (prGlueInfo->ulFlag & GLUE_FLAG_HALT)) {
		return -EFAULT;
	}
	/* permission check */
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	if (err)
		return -EFAULT;

	/* no ioctl is implemented yet */
	return 0;
}


/*----------------------------------------------------------------------------*/
/*!
* \brief ioctl handler for character device to communicate with 802.11 PAL
*
* \param[in]
* \return
*           Follows Linux Character Device Interface
*
*/
/*----------------------------------------------------------------------------*/
static unsigned int bow_ampc_poll(IN struct file *filp, IN poll_table * wait)
{
	unsigned int retval;
	P_GLUE_INFO_T prGlueInfo;
	prGlueInfo = (P_GLUE_INFO_T) (filp->private_data);

	ASSERT(prGlueInfo);

	if ((prGlueInfo->rBowInfo.fgIsRegistered == FALSE) || (prGlueInfo->ulFlag & GLUE_FLAG_HALT)) {
		return -EFAULT;
	}

	poll_wait(filp, &prGlueInfo->rBowInfo.outq, wait);

	retval = (POLLOUT | POLLWRNORM);	/* always accepts incoming command packets */

/* DBGLOG(BOW, EVENT, ("bow_ampc_pol, POLLOUT | POLLWRNORM, %x\n", retval)); */

/* if(kfifo_len(prGlueInfo->rBowInfo.prKfifo) > 0) */
	if (kfifo_len(&(prGlueInfo->rBowInfo.rKfifo)) > 0) {
		retval |= (POLLIN | POLLRDNORM);

/* DBGLOG(BOW, EVENT, ("bow_ampc_pol, POLLIN | POLLRDNORM, %x\n", retval)); */

	}

	return retval;
}


/*----------------------------------------------------------------------------*/
/*!
* \brief open handler for character device to communicate with 802.11 PAL
*
* \param[in]
* \return
*           Follows Linux Character Device Interface
*
*/
/*----------------------------------------------------------------------------*/
static int bow_ampc_open(IN struct inode *inodep, IN struct file *filp)
{
	P_GLUE_INFO_T prGlueInfo;
	P_GL_BOW_INFO prBowInfo;

	DBGLOG(BOW, INFO, ("in %s\n", __func__));

	prBowInfo = container_of(inodep->i_cdev, GL_BOW_INFO, cdev);
	ASSERT(prBowInfo);

	prGlueInfo = container_of(prBowInfo, GLUE_INFO_T, rBowInfo);
	ASSERT(prGlueInfo);

	/* set-up private data */
	filp->private_data = prGlueInfo;

	return 0;
}


/*----------------------------------------------------------------------------*/
/*!
* \brief close handler for character device to communicate with 802.11 PAL
*
* \param[in]
* \return
*           Follows Linux Character Device Interface
*
*/
/*----------------------------------------------------------------------------*/
static int bow_ampc_release(IN struct inode *inodep, IN struct file *filp)
{
	P_GLUE_INFO_T prGlueInfo;
	prGlueInfo = (P_GLUE_INFO_T) (filp->private_data);

	DBGLOG(BOW, INFO, ("in %s\n", __func__));

	ASSERT(prGlueInfo);

	return 0;
}


/*----------------------------------------------------------------------------*/
/*!
* \brief to indicate event for Bluetooth over Wi-Fi
*
* \param[in]
*           prGlueInfo
*           prEvent
* \return
*           none
*/
/*----------------------------------------------------------------------------*/
VOID kalIndicateBOWEvent(IN P_GLUE_INFO_T prGlueInfo, IN P_AMPC_EVENT prEvent)
{
	size_t u4AvailSize, u4EventSize;

	ASSERT(prGlueInfo);
	ASSERT(prEvent);

	/* check device */
	if ((prGlueInfo->rBowInfo.fgIsRegistered == FALSE) || (prGlueInfo->ulFlag & GLUE_FLAG_HALT)) {
		return;
	}

/*    u4AvailSize =
	GLUE_BOW_KFIFO_DEPTH - kfifo_len(prGlueInfo->rBowInfo.prKfifo);*/

	u4AvailSize = GLUE_BOW_KFIFO_DEPTH - kfifo_len(&(prGlueInfo->rBowInfo.rKfifo));


	u4EventSize = prEvent->rHeader.u2PayloadLength + sizeof(AMPC_EVENT_HEADER_T);

	/* check kfifo availability */
	if (u4AvailSize < u4EventSize) {
		DBGLOG(BOW, EVENT, ("[bow] no space for event: %d/%d\n", u4EventSize, u4AvailSize));
		return;
	}
	/* queue into kfifo */
/* kfifo_put(prGlueInfo->rBowInfo.prKfifo, (PUINT_8)prEvent, u4EventSize); */
/* kfifo_in(prGlueInfo->rBowInfo.prKfifo, (PUINT_8)prEvent, u4EventSize); */
	kfifo_in(&(prGlueInfo->rBowInfo.rKfifo), (PUINT_8) prEvent, u4EventSize);
	wake_up_interruptible(&(prGlueInfo->rBowInfo.outq));

	return;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief to retrieve Bluetooth-over-Wi-Fi state from glue layer
*
* \param[in]
*           prGlueInfo
*           rPeerAddr
* \return
*           ENUM_BOW_DEVICE_STATE
*/
/*----------------------------------------------------------------------------*/
ENUM_BOW_DEVICE_STATE kalGetBowState(IN P_GLUE_INFO_T prGlueInfo, IN UINT_8 aucPeerAddress[6]
    )
{
	UINT_8 i;

	ASSERT(prGlueInfo);

	DBGLOG(BOW, EVENT, ("kalGetBowState.\n"));

	for (i = 0; i < CFG_BOW_PHYSICAL_LINK_NUM; i++) {
		if (EQUAL_MAC_ADDR(prGlueInfo->rBowInfo.arPeerAddr, aucPeerAddress) == 0) {
			DBGLOG(BOW, EVENT,
			       ("kalGetBowState, aucPeerAddress %x, %x:%x:%x:%x:%x:%x.\n", i,
				aucPeerAddress[0], aucPeerAddress[1], aucPeerAddress[2],
				aucPeerAddress[3], aucPeerAddress[4], aucPeerAddress[5]));

			DBGLOG(BOW, EVENT,
			       ("kalGetBowState, prGlueInfo->rBowInfo.aeState %x, %x.\n", i,
				prGlueInfo->rBowInfo.aeState[i]));


			return prGlueInfo->rBowInfo.aeState[i];
		}
	}

	return BOW_DEVICE_STATE_DISCONNECTED;
}


/*----------------------------------------------------------------------------*/
/*!
* \brief to set Bluetooth-over-Wi-Fi state in glue layer
*
* \param[in]
*           prGlueInfo
*           eBowState
*           rPeerAddr
* \return
*           none
*/
/*----------------------------------------------------------------------------*/
BOOLEAN
kalSetBowState(IN P_GLUE_INFO_T prGlueInfo,
	       IN ENUM_BOW_DEVICE_STATE eBowState, IN UINT_8 aucPeerAddress[6]
    )
{
	UINT_8 i;

	ASSERT(prGlueInfo);

	DBGLOG(BOW, EVENT, ("kalSetBowState.\n"));

	DBGLOG(BOW, EVENT, ("kalSetBowState, prGlueInfo->rBowInfo.arPeerAddr, %x:%x:%x:%x:%x:%x.\n",
			    prGlueInfo->rBowInfo.arPeerAddr[0],
			    prGlueInfo->rBowInfo.arPeerAddr[1],
			    prGlueInfo->rBowInfo.arPeerAddr[2],
			    prGlueInfo->rBowInfo.arPeerAddr[3],
			    prGlueInfo->rBowInfo.arPeerAddr[4],
			    prGlueInfo->rBowInfo.arPeerAddr[5]));

	DBGLOG(BOW, EVENT, ("kalSetBowState, aucPeerAddress, %x:%x:%x:%x:%x:%x.\n",
			    aucPeerAddress[0],
			    aucPeerAddress[1],
			    aucPeerAddress[2],
			    aucPeerAddress[3], aucPeerAddress[4], aucPeerAddress[5]));

	for (i = 0; i < CFG_BOW_PHYSICAL_LINK_NUM; i++) {
		if (EQUAL_MAC_ADDR(prGlueInfo->rBowInfo.arPeerAddr, aucPeerAddress) == 0) {
			prGlueInfo->rBowInfo.aeState[i] = eBowState;

			DBGLOG(BOW, EVENT,
			       ("kalSetBowState, aucPeerAddress %x, %x:%x:%x:%x:%x:%x.\n", i,
				aucPeerAddress[0], aucPeerAddress[1], aucPeerAddress[2],
				aucPeerAddress[3], aucPeerAddress[4], aucPeerAddress[5]));

			DBGLOG(BOW, EVENT,
			       ("kalSetBowState, prGlueInfo->rBowInfo.aeState %x, %x.\n", i,
				prGlueInfo->rBowInfo.aeState[i]));

			return TRUE;
		}
	}

	return FALSE;
}


/*----------------------------------------------------------------------------*/
/*!
* \brief to retrieve Bluetooth-over-Wi-Fi global state
*
* \param[in]
*           prGlueInfo
*
* \return
*           BOW_DEVICE_STATE_DISCONNECTED
*               in case there is no BoW connection or
*               BoW connection under initialization
*
*           BOW_DEVICE_STATE_STARTING
*               in case there is no BoW connection but
*               some BoW connection under initialization
*
*           BOW_DEVICE_STATE_CONNECTED
*               in case there is any BoW connection available
*/
/*----------------------------------------------------------------------------*/
ENUM_BOW_DEVICE_STATE kalGetBowGlobalState(IN P_GLUE_INFO_T prGlueInfo)
{
	UINT_32 i;

	ASSERT(prGlueInfo);


/* Henry, can reduce this logic to indentify state change */

	for (i = 0; i < CFG_BOW_PHYSICAL_LINK_NUM; i++) {
		if (prGlueInfo->rBowInfo.aeState[i] == BOW_DEVICE_STATE_CONNECTED) {
			return BOW_DEVICE_STATE_CONNECTED;
		}
	}

	for (i = 0; i < CFG_BOW_PHYSICAL_LINK_NUM; i++) {
		if (prGlueInfo->rBowInfo.aeState[i] == BOW_DEVICE_STATE_STARTING) {
			return BOW_DEVICE_STATE_STARTING;
		}
	}

	return BOW_DEVICE_STATE_DISCONNECTED;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief to retrieve Bluetooth-over-Wi-Fi operating frequency
*
* \param[in]
*           prGlueInfo
*
* \return
*           in unit of KHz
*/
/*----------------------------------------------------------------------------*/
UINT_32 kalGetBowFreqInKHz(IN P_GLUE_INFO_T prGlueInfo)
{
	ASSERT(prGlueInfo);

	return prGlueInfo->rBowInfo.u4FreqInKHz;
}


/*----------------------------------------------------------------------------*/
/*!
* \brief to retrieve Bluetooth-over-Wi-Fi role
*
* \param[in]
*           prGlueInfo
*
* \return
*           0: Responder
*           1: Initiator
*/
/*----------------------------------------------------------------------------*/
UINT_8 kalGetBowRole(IN P_GLUE_INFO_T prGlueInfo, IN PARAM_MAC_ADDRESS rPeerAddr)
{
	UINT_32 i;

	ASSERT(prGlueInfo);

	for (i = 0; i < CFG_BOW_PHYSICAL_LINK_NUM; i++) {
		if (EQUAL_MAC_ADDR(prGlueInfo->rBowInfo.arPeerAddr[i], rPeerAddr) == 0) {
			return prGlueInfo->rBowInfo.aucRole[i];
		}
	}

	return 0;
}


/*----------------------------------------------------------------------------*/
/*!
* \brief to set Bluetooth-over-Wi-Fi role
*
* \param[in]
*           prGlueInfo
*           ucRole
*                   0: Responder
*                   1: Initiator
* \return
*           none
*/
/*----------------------------------------------------------------------------*/
VOID kalSetBowRole(IN P_GLUE_INFO_T prGlueInfo, IN UINT_8 ucRole, IN PARAM_MAC_ADDRESS rPeerAddr)
{
	UINT_32 i;

	ASSERT(prGlueInfo);
	ASSERT(ucRole <= 1);

	for (i = 0; i < CFG_BOW_PHYSICAL_LINK_NUM; i++) {
		if (EQUAL_MAC_ADDR(prGlueInfo->rBowInfo.arPeerAddr[i], rPeerAddr) == 0) {
			prGlueInfo->rBowInfo.aucRole[i] = ucRole;	/* Henry, 0 : Responder, 1 : Initiator */
		}
	}
}


/*----------------------------------------------------------------------------*/
/*!
* \brief to get available Bluetooth-over-Wi-Fi physical link number
*
* \param[in]
*           prGlueInfo
* \return
*           UINT_32
*               how many physical links are aviailable
*/
/*----------------------------------------------------------------------------*/
UINT_8 kalGetBowAvailablePhysicalLinkCount(IN P_GLUE_INFO_T prGlueInfo)
{
	UINT_8 i;
	UINT_8 ucLinkCount = 0;

	ASSERT(prGlueInfo);

	for (i = 0; i < CFG_BOW_PHYSICAL_LINK_NUM; i++) {
		if (prGlueInfo->rBowInfo.aeState[i] == BOW_DEVICE_STATE_DISCONNECTED) {
			ucLinkCount++;
		}
	}

#if 0
	DBGLOG(BOW, EVENT,
	       ("kalGetBowAvailablePhysicalLinkCount, ucLinkCount, %c.\n", ucLinkCount));
#endif

	return ucLinkCount;
}


/*----------------------------------------------------------------------------*/
/*!
* @brief This inline function is to extract some packet information for BoW
*
* @param prGlueInfo         Pointer to the glue structure
* @param prNdisPacket       Packet descriptor
* @param pfgIs1X            802.1x packet or not
*
* @retval TRUE      Success to extract information
* @retval FALSE     Fail to extract correct information
*/
/*----------------------------------------------------------------------------*/
BOOLEAN
kalBowFrameClassifier(IN P_GLUE_INFO_T prGlueInfo,
		      IN P_NATIVE_PACKET prPacket, OUT PBOOLEAN pfgIs1X)
{
	UINT_32 u4PacketLen;
	UINT_16 u2EtherTypeLen;
	struct sk_buff *prSkb = (struct sk_buff *)prPacket;
	PUINT_8 aucLookAheadBuf = NULL;
	UINT_8 ucEthTypeLenOffset = ETHER_HEADER_LEN - ETHER_TYPE_LEN;
	PUINT_8 pucNextProtocol = NULL;
	UINT_8 aucLLC[] = ETH_LLC;
	UINT_8 aucSnapBtOui[] = ETH_SNAP_BT_SIG_OUI;
	UINT_8 ucMinLength = ucEthTypeLenOffset + ETHER_TYPE_LEN + ETH_LLC_LEN + ETH_SNAP_LEN;

	DEBUGFUNC("kalQoSFrameClassifierAndPacketInfo");

	u4PacketLen = prSkb->len;

	if (u4PacketLen < ETHER_HEADER_LEN) {
		DBGLOG(INIT, WARN, ("Invalid Ether packet length: %lu\n", u4PacketLen));
		return FALSE;
	}

	aucLookAheadBuf = prSkb->data;

	*pfgIs1X = FALSE;

	/* 4 <0> Obtain Ether Type/Len */
	WLAN_GET_FIELD_BE16(&aucLookAheadBuf[ucEthTypeLenOffset], &u2EtherTypeLen);

	/* 4 <1> Skip 802.1Q header (VLAN Tagging) */
	if (u2EtherTypeLen == ETH_P_VLAN) {
		ucEthTypeLenOffset += ETH_802_1Q_HEADER_LEN;
		WLAN_GET_FIELD_BE16(&aucLookAheadBuf[ucEthTypeLenOffset], &u2EtherTypeLen);
	}
	/* 4 <2> Obtain next protocol pointer */
	pucNextProtocol = &aucLookAheadBuf[ucEthTypeLenOffset + ETHER_TYPE_LEN];

	/* 4 <3> Handle ethernet format */
	if (u2EtherTypeLen > ETH_802_3_MAX_LEN) {
		/* Not BoW frame */
		return FALSE;
	}
	/* 4 <4> Check for PAL (BT over Wi-Fi) */
	/* BoW LLC/SNAP header check */
	if (u4PacketLen >= ucMinLength &&
	    !kalMemCmp(pucNextProtocol, aucLLC, ETH_LLC_LEN) &&
	    !kalMemCmp(pucNextProtocol + ETH_LLC_LEN, aucSnapBtOui, ETH_SNAP_OUI_LEN)) {
		UINT_16 u2LocalCode;

		WLAN_GET_FIELD_BE16(pucNextProtocol + ETH_LLC_LEN + ETH_SNAP_OUI_LEN, &u2LocalCode);

		if (u2LocalCode == BOW_PROTOCOL_ID_SECURITY_FRAME) {
			*pfgIs1X = TRUE;
		}

		return TRUE;
	}

	return FALSE;
}				/* end of kalBoWFrameClassifier() */

#if CFG_BOW_SEPARATE_DATA_PATH

/* Net Device Hooks */
/*----------------------------------------------------------------------------*/
/*!
 * \brief A function for net_device open (ifup)
 *
 * \param[in] prDev      Pointer to struct net_device.
 *
 * \retval 0     The execution succeeds.
 * \retval < 0   The execution failed.
 */
/*----------------------------------------------------------------------------*/
static int bowOpen(IN struct net_device *prDev)
{
	P_GLUE_INFO_T prGlueInfo = NULL;
	P_ADAPTER_T prAdapter = NULL;

	ASSERT(prDev);

	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prDev));
	ASSERT(prGlueInfo);

	prAdapter = prGlueInfo->prAdapter;
	ASSERT(prAdapter);

	/* 2. carrier on & start TX queue */
	netif_carrier_on(prDev);
	netif_tx_start_all_queues(prDev);

	return 0;		/* success */
}


/*----------------------------------------------------------------------------*/
/*!
 * \brief A function for net_device stop (ifdown)
 *
 * \param[in] prDev      Pointer to struct net_device.
 *
 * \retval 0     The execution succeeds.
 * \retval < 0   The execution failed.
 */
/*----------------------------------------------------------------------------*/
static int bowStop(IN struct net_device *prDev)
{
	P_GLUE_INFO_T prGlueInfo = NULL;
	P_ADAPTER_T prAdapter = NULL;

	ASSERT(prDev);

	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prDev));
	ASSERT(prGlueInfo);

	prAdapter = prGlueInfo->prAdapter;
	ASSERT(prAdapter);

	/* 1. stop TX queue */
	netif_tx_stop_all_queues(prDev);

	/* 2. turn of carrier */
	if (netif_carrier_ok(prDev)) {
		netif_carrier_off(prDev);
	}

	return 0;
};

#if 0
/*----------------------------------------------------------------------------*/
/*!
 * \brief This function is TX entry point of NET DEVICE.
 *
 * \param[in] prSkb  Pointer of the sk_buff to be sent
 * \param[in] prDev  Pointer to struct net_device
 *
 * \retval NETDEV_TX_OK - on success.
 * \retval NETDEV_TX_BUSY - on failure, packet will be discarded by upper layer.
 */
/*----------------------------------------------------------------------------*/
static int bowHardStartXmit(IN struct sk_buff *prSkb, IN struct net_device *prDev)
{
	P_GLUE_INFO_T prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prDev));

	P_QUE_ENTRY_T prQueueEntry = NULL;
	P_QUE_T prTxQueue = NULL;
	UINT_16 u2QueueIdx = 0;
	UINT_8 ucDSAP, ucSSAP, ucControl;
	UINT_8 aucOUI[3];
	PUINT_8 aucLookAheadBuf = NULL;
	UINT_8 ucBssIndex;

	GLUE_SPIN_LOCK_DECLARATION();

	ASSERT(prSkb);
	ASSERT(prDev);
	ASSERT(prGlueInfo);

	aucLookAheadBuf = prSkb->data;

	ucDSAP = *(PUINT_8) &aucLookAheadBuf[ETH_LLC_OFFSET];
	ucSSAP = *(PUINT_8) &aucLookAheadBuf[ETH_LLC_OFFSET + 1];
	ucControl = *(PUINT_8) &aucLookAheadBuf[ETH_LLC_OFFSET + 2];
	aucOUI[0] = *(PUINT_8) &aucLookAheadBuf[ETH_SNAP_OFFSET];
	aucOUI[1] = *(PUINT_8) &aucLookAheadBuf[ETH_SNAP_OFFSET + 1];
	aucOUI[2] = *(PUINT_8) &aucLookAheadBuf[ETH_SNAP_OFFSET + 2];

	if (!(ucDSAP == ETH_LLC_DSAP_SNAP &&
	      ucSSAP == ETH_LLC_SSAP_SNAP &&
	      ucControl == ETH_LLC_CONTROL_UNNUMBERED_INFORMATION &&
	      aucOUI[0] == ETH_SNAP_BT_SIG_OUI_0 &&
	      aucOUI[1] == ETH_SNAP_BT_SIG_OUI_1 &&
	      aucOUI[2] == ETH_SNAP_BT_SIG_OUI_2) || (prSkb->len > 1514)) {
		dev_kfree_skb(prSkb);
		return NETDEV_TX_OK;
	}

	if (prGlueInfo->ulFlag & GLUE_FLAG_HALT) {
		DBGLOG(BOW, TRACE, ("GLUE_FLAG_HALT skip tx\n"));
		dev_kfree_skb(prSkb);
		return NETDEV_TX_OK;
	}

	GLUE_SET_PKT_FLAG_PAL(prSkb);

	ucBssIndex = wlanGetBssIdxByNetInterface(prGlueInfo, NET_DEV_BOW_IDX);

	GLUE_SET_PKT_BSS_IDX(prSkb, ucBssIndex);

	prQueueEntry = (P_QUE_ENTRY_T) GLUE_GET_PKT_QUEUE_ENTRY(prSkb);
	prTxQueue = &prGlueInfo->rTxQueue;

	if (wlanProcessSecurityFrame(prGlueInfo->prAdapter, (P_NATIVE_PACKET) prSkb) == FALSE) {
		GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_TX_QUE);
		QUEUE_INSERT_TAIL(prTxQueue, prQueueEntry);
		GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_TX_QUE);


		GLUE_INC_REF_CNT(prGlueInfo->i4TxPendingFrameNum);
		GLUE_INC_REF_CNT(prGlueInfo->ai4TxPendingFrameNumPerQueue[ucBssIndex][u2QueueIdx]);

		if (prGlueInfo->ai4TxPendingFrameNumPerQueue[ucBssIndex][u2QueueIdx] >=
		    CFG_TX_STOP_NETIF_PER_QUEUE_THRESHOLD) {
			netif_stop_subqueue(prDev, u2QueueIdx);
		}
	} else {
		GLUE_INC_REF_CNT(prGlueInfo->i4TxPendingSecurityFrameNum);
	}

	kalSetEvent(prGlueInfo);

	/* For Linux, we'll always return OK FLAG, because we'll free this skb by ourself */
	return NETDEV_TX_OK;
}
#else
/*----------------------------------------------------------------------------*/
/*!
 * \brief This function is TX entry point of NET DEVICE.
 *
 * \param[in] prSkb  Pointer of the sk_buff to be sent
 * \param[in] prDev  Pointer to struct net_device
 *
 * \retval NETDEV_TX_OK - on success.
 * \retval NETDEV_TX_BUSY - on failure, packet will be discarded by upper layer.
 */
/*----------------------------------------------------------------------------*/
static int bowHardStartXmit(IN struct sk_buff *prSkb, IN struct net_device *prDev)
{
	P_NETDEV_PRIVATE_GLUE_INFO prNetDevPrivate = (P_NETDEV_PRIVATE_GLUE_INFO) NULL;
	P_GLUE_INFO_T prGlueInfo = NULL;
	UINT_8 ucBssIndex;
	BOOLEAN fgIs1x;

	ASSERT(prSkb);
	ASSERT(prDev);

#if 1
	prNetDevPrivate = (P_NETDEV_PRIVATE_GLUE_INFO) netdev_priv(prDev);
	prGlueInfo = prNetDevPrivate->prGlueInfo;
	ucBssIndex = prNetDevPrivate->ucBssIdx;
#else
	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prDev));

	ucBssIndex = wlanGetBssIdxByNetInterface(prGlueInfo, NET_DEV_BOW_IDX);
#endif

    kalResetPacket(prGlueInfo, (P_NATIVE_PACKET)prSkb);

	/* Discard frames not generated by PAL */
	/* Parsing BOW frame info */
	if (!kalBowFrameClassifier(prGlueInfo, (P_NATIVE_PACKET) prSkb, &fgIs1x)) {
		/* Cannot extract packet */
		DBGLOG(BOW, INFO, ("Invalid BOW frame, skip Tx\n"));
		dev_kfree_skb(prSkb);
		return NETDEV_TX_OK;
	}

	if (fgIs1x) {
		GLUE_SET_PKT_FLAG_1X(prSkb);
	}

	if (kalHardStartXmit(prSkb, prDev, prGlueInfo, ucBssIndex) == WLAN_STATUS_SUCCESS) {
		/* Successfully enqueue to Tx queue */
	}

	/* For Linux, we'll always return OK FLAG, because we'll free this skb by ourself */
	return NETDEV_TX_OK;
}
#endif

/* callbacks for netdevice */
static const struct net_device_ops bow_netdev_ops = {
	.ndo_open = bowOpen,
	.ndo_stop = bowStop,
	.ndo_start_xmit = bowHardStartXmit,
};

/*----------------------------------------------------------------------------*/
/*!
* \brief initialize net device for Bluetooth-over-Wi-Fi
*
* \param[in]
*           prGlueInfo
*           prDevName
*
* \return
*           TRUE
*           FALSE
*/
/*----------------------------------------------------------------------------*/
BOOLEAN kalInitBowDevice(IN P_GLUE_INFO_T prGlueInfo, IN const char *prDevName)
{
	P_ADAPTER_T prAdapter;
	P_GL_HIF_INFO_T prHif;
	PARAM_MAC_ADDRESS rMacAddr;
	P_NETDEV_PRIVATE_GLUE_INFO prNetDevPriv = (P_NETDEV_PRIVATE_GLUE_INFO) NULL;

	ASSERT(prGlueInfo);
	ASSERT(prGlueInfo->rBowInfo.fgIsRegistered == TRUE);

	prAdapter = prGlueInfo->prAdapter;
	ASSERT(prAdapter);

	prHif = &prGlueInfo->rHifInfo;
	ASSERT(prHif);

	if (prGlueInfo->rBowInfo.fgIsNetRegistered == FALSE) {
		prGlueInfo->rBowInfo.prDevHandler =
		    alloc_netdev_mq(sizeof(P_GLUE_INFO_T), prDevName, ether_setup, CFG_MAX_TXQ_NUM);

		if (!prGlueInfo->rBowInfo.prDevHandler) {
			return FALSE;
		} else {
			/* 1. setup netdev */
			/* 1.1 Point to shared glue structure */
			/* *((P_GLUE_INFO_T *) netdev_priv(prGlueInfo->rBowInfo.prDevHandler)) = prGlueInfo; */
			prNetDevPriv =
			    (P_NETDEV_PRIVATE_GLUE_INFO) netdev_priv(prGlueInfo->rBowInfo.
								     prDevHandler);
			prNetDevPriv->prGlueInfo = prGlueInfo;

			/* 1.2 fill hardware address */
			COPY_MAC_ADDR(rMacAddr, prAdapter->rMyMacAddr);
			rMacAddr[0] |= 0x2;	/* change to local administrated address */
			memcpy(prGlueInfo->rBowInfo.prDevHandler->dev_addr, rMacAddr, ETH_ALEN);
			memcpy(prGlueInfo->rBowInfo.prDevHandler->perm_addr,
			       prGlueInfo->rBowInfo.prDevHandler->dev_addr, ETH_ALEN);

			/* 1.3 register callback functions */
			prGlueInfo->rBowInfo.prDevHandler->netdev_ops = &bow_netdev_ops;

#if (MTK_WCN_HIF_SDIO == 0)
			SET_NETDEV_DEV(prGlueInfo->rBowInfo.prDevHandler, &(prHif->func->dev));
#endif

			register_netdev(prGlueInfo->rBowInfo.prDevHandler);

			/* 2. net device initialize */
			netif_carrier_off(prGlueInfo->rBowInfo.prDevHandler);
			netif_tx_stop_all_queues(prGlueInfo->rBowInfo.prDevHandler);

			/* 2.1 bind NetDev pointer to NetDev index */
			wlanBindBssIdxToNetInterface(prGlueInfo, bowInit(prAdapter),
						     (PVOID) prGlueInfo->rBowInfo.prDevHandler);
			prNetDevPriv->ucBssIdx = prAdapter->rWifiVar.rBowFsmInfo.ucBssIndex;
			/* wlanBindNetInterface(prGlueInfo, NET_DEV_BOW_IDX, (PVOID)prGlueInfo->rBowInfo.prDevHandler); */

			/* 3. finish */
			prGlueInfo->rBowInfo.fgIsNetRegistered = TRUE;
		}
	}

	return TRUE;
}


/*----------------------------------------------------------------------------*/
/*!
* \brief uninitialize net device for Bluetooth-over-Wi-Fi
*
* \param[in]
*           prGlueInfo
*
* \return
*           TRUE
*           FALSE
*/
/*----------------------------------------------------------------------------*/
BOOLEAN kalUninitBowDevice(IN P_GLUE_INFO_T prGlueInfo)
{
	P_ADAPTER_T prAdapter;

	ASSERT(prGlueInfo);

	prAdapter = prGlueInfo->prAdapter;
	ASSERT(prAdapter);
	/* ASSERT(prGlueInfo->rBowInfo.fgIsRegistered == TRUE); */

	if (prGlueInfo->rBowInfo.fgIsNetRegistered == TRUE) {

		prGlueInfo->rBowInfo.fgIsNetRegistered = FALSE;

		bowUninit(prAdapter);

		if (netif_carrier_ok(prGlueInfo->rBowInfo.prDevHandler)) {
			netif_carrier_off(prGlueInfo->rBowInfo.prDevHandler);
		}

		netif_tx_stop_all_queues(prGlueInfo->rBowInfo.prDevHandler);

		/* netdevice unregistration & free */
		unregister_netdev(prGlueInfo->rBowInfo.prDevHandler);
		free_netdev(prGlueInfo->rBowInfo.prDevHandler);
		prGlueInfo->rBowInfo.prDevHandler = NULL;

		return TRUE;

	} else {
		return FALSE;
	}
}

#endif				/* CFG_BOW_SEPARATE_DATA_PATH */
#endif				/* CFG_ENABLE_BT_OVER_WIFI */
