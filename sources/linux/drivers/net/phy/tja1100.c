/*
 * drivers/net/phy/tja1100.c
 *
 * Driver for BroadR-Reach PHYs
 *
 * Support : BroadR-Reach Phys: TJA1100
 */

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/unistd.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/mii.h>
#include <linux/phy.h>

#define TJA1100_BASIC_STATUS	0x01
#define TJA1100_EXTEND_STATUS	0x0F
#define TJA1100_COMMUNICATION_STATUS	0x17
#define TJA_BASIC_STATUS_EXTENDED_STATUS  BIT(8)
#define TJA_EXTENDED_STATUS_100BASE_BR_R  BIT(7)
#define TJA_COMM_STATUS_LINK_UP         BIT(15)

static int tja1100_read_status(struct phy_device *phydev)
{
	int regval;

	regval = phy_read(phydev, TJA1100_BASIC_STATUS);
	if (regval & TJA_BASIC_STATUS_EXTENDED_STATUS) {
		regval = phy_read(phydev, TJA1100_EXTEND_STATUS);
		if (regval & TJA_EXTENDED_STATUS_100BASE_BR_R)
			phydev->speed = SPEED_100;
		else
			phydev->speed = SPEED_10;
	}
	phydev->duplex = DUPLEX_FULL;

	regval = phy_read(phydev, TJA1100_COMMUNICATION_STATUS);
	if (regval & TJA_COMM_STATUS_LINK_UP)
		phydev->link = 1;
	else
		phydev->link = 0;

	phydev->pause = 0;
	phydev->asym_pause = 0;

	return 0;
}

static int tja1100_config_aneg(struct phy_device *phydev)
{
	int err;

	/* Configure the new settings */
	err = genphy_config_aneg(phydev);

	if (err < 0)
		return err;

	return 0;
}

static int tja1100_config_init(struct phy_device *phydev)
{
	int err;

	/* Isolate the PHY */
	err = phy_write(phydev, MII_BMCR, BMCR_RESET);

	return err;
}

static struct phy_driver tja1100_driver[] = {
{
	.phy_id		= 0x0180dc48,
	.name		= "BroadR-Reach TJA1100",
	.phy_id_mask	= 0x0ffffff0,
	.features	= PHY_BASIC_FEATURES,
	.flags		= PHY_HAS_INTERRUPT,
	.config_init	= tja1100_config_init,
	.config_aneg	= tja1100_config_aneg,
	.read_status	= tja1100_read_status,
} };

module_phy_driver(tja1100_driver);

MODULE_DESCRIPTION("BroadR-Reach PHY driver");
MODULE_AUTHOR("Ma Ning");
MODULE_LICENSE("GPL");
