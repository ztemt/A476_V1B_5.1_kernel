/****************************************************************

Siano Mobile Silicon, Inc.
MDTV receiver kernel modules.
Copyright (C) 2006-2008, Uri Shkolnik

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

 This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

****************************************************************/

#ifndef __SMS_SPI_PHY_H__
#define __SMS_SPI_PHY_H__

 
typedef enum SmsSpiDataWidth_E
{ SPI_WIDTH_8BIT,
  SPI_WIDTH_16BIT,
  SPI_WIDTH_32BIT,
  SPI_MAX_WID
}SmsSpiDataWidth_ET;
 
 
typedef enum IoControlSlew_E
{
      IOC_SLEW_0_45     = (0 << 2),       // Slew rate 0.45 v/ns
      IOC_SLEW_0_9      = (1 << 2),       // Slew rate 0.9 v/ns
      IOC_SLEW_1_7      = (2 << 2),       // Slew rate 1.7 v/ns
      IOC_SLEW_3_3      = (3 << 2),       // Slew rate 3.3 v/ns
} IoControlSlew_ET;
 
typedef enum IoControlDrive_E
{
      IOC_DRV_1_5 = (0 << 4),       // Drive 1.5 mA
      IOC_DRV_2_8 = (1 << 4),       // Drive 2.8 mA
      IOC_DRV_4   = (2 << 4),       // Drive 4 mA
      IOC_DRV_7   = (3 << 4),       // Drive 7 mA
      IOC_DRV_10  = (4 << 4),       // Drive 10 mA
      IOC_DRV_11  = (5 << 4),       // Drive 11 mA
      IOC_DRV_14  = (6 << 4),       // Drive 14 mA
      IOC_DRV_16  = (7 << 4),       // Drive 16 mA
} IoControlDrive_ET;

void smsspibus_xfer(void *context, unsigned char *txbuf,
		    unsigned long txbuf_phy_addr, unsigned char *rxbuf,
		    unsigned long rxbuf_phy_addr, int len);
void *smsspiphy_register(void *context, void (*smsspi_interruptHandler) (void *),
		     void *intr_context);
void smsspiphy_unregister(void *context);
int smsspiphy_init(void *context);
void smsspiphy_deinit(void *context);
int smsspiphy_is_device_exists(void);
void smschipreset(void *context);
int smsspiphy_set_chip_reset(void *context, int value);
void WriteFWtoStellar(void *pSpiPhy, unsigned char *pFW, unsigned long Len);
void prepareForFWDnl(void *pSpiPhy);
void fwDnlComplete(void *context, int App);
void sms_chip_set_exists(int isexits);
int sms_chip_get_exists(void);


#endif /* __SMS_SPI_PHY_H__ */
