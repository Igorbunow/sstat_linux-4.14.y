/*
 * drivers/net/phy/realtek.c
 *
 * Driver for Realtek PHYs
 *
 * Author: Johnson Leung <r58129@freescale.com>
 *
 * Copyright (c) 2004 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */
#include <linux/phy.h>
#include <linux/module.h>

/* Interrupt Indicators and SNR Display Register */
#define RTL8201F_INSR		0x1e
/* Page Select Register */
#define RTL8201F_PGSR		0x1f
/* Interrupt, WOL Enable, and LEDs Function Registers */
#define RTL8201F_INER		0x13
#define RTL8201F_INER_MASK	0x3800
#define RTL8201F_INER_LED_MASK	0x0030
#define RTL8201F_INER_LED_VAL	0x0000
/* Fiber Mode and Loopback Register */
#define RTL8201F_FMLR		0x1c
#define RTL8201F_FMLR_INIT	0x04
#define RTL8201F_FMLR_MASK	0x0026

#define RTL821x_PHYSR		0x11
#define RTL821x_PHYSR_DUPLEX	0x2000
#define RTL821x_PHYSR_SPEED	0xc000
#define RTL821x_INER		0x12
#define RTL821x_INER_INIT	0x6400
#define RTL821x_INSR		0x13
#define RTL8211E_INER_LINK_STATUS 0x400

#define RTL8211F_INER_LINK_STATUS 0x0010
#define RTL8211F_INSR		0x1d
#define RTL8211F_PAGE_SELECT	0x1f
#define RTL8211F_TX_DELAY	0x100

MODULE_DESCRIPTION("Realtek PHY driver");
MODULE_AUTHOR("Johnson Leung");
MODULE_LICENSE("GPL");

static void rtl8201f_select_page(struct phy_device *phydev, int page)
{
	phy_write(phydev, RTL8201F_PGSR, page);
}

static int rtl8201f_config_init(struct phy_device *phydev)
{
	int ret;
	u16 reg_val;

	ret = genphy_config_init(phydev);
	if (ret < 0)
		return ret;
	
	/* Enable Enable Auto MDIX Function and Force auto MDI-X */
	phy_write(phydev, RTL8201F_FMLR, RTL8201F_FMLR_INIT & RTL8201F_FMLR_MASK);
	
	
	/* Switch LED Functions */
	rtl8201f_select_page(phydev, 7);
	
	reg_val = phy_read(phydev, RTL8201F_INER);
	
	reg_val = reg_val & ( ~RTL8201F_INER_LED_MASK | RTL8201F_INER_LED_VAL);
	reg_val = reg_val | (  RTL8201F_INER_LED_MASK & RTL8201F_INER_LED_VAL);
	
	phy_write(phydev, RTL8201F_INER, reg_val);
	
	
	rtl8201f_select_page(phydev, 0);
	
	
	return 0;
}

static int rtl8201f_ack_interrupt(struct phy_device *phydev)
{
	int err;

	err = phy_read(phydev, RTL8201F_INSR);

	return (err < 0) ? err : 0;
}

static int rtl821x_ack_interrupt(struct phy_device *phydev)
{
	int err;

	err = phy_read(phydev, RTL821x_INSR);

	return (err < 0) ? err : 0;
}

static int rtl8211f_ack_interrupt(struct phy_device *phydev)
{
	int err;

	phy_write(phydev, RTL8211F_PAGE_SELECT, 0xa43);
	err = phy_read(phydev, RTL8211F_INSR);
	/* restore to default page 0 */
	phy_write(phydev, RTL8211F_PAGE_SELECT, 0x0);

	return (err < 0) ? err : 0;
}

static int rtl8201f_config_intr(struct phy_device *phydev)
{
	int err;
	u16 reg_val;
	
	rtl8201f_select_page(phydev, 7);
	
	reg_val = phy_read(phydev, RTL8201F_INER);
	
	if (phydev->interrupts == PHY_INTERRUPT_ENABLED)
		err = phy_write(phydev, RTL8201F_INER, RTL8201F_INER_MASK |
				reg_val);
	else
		err = phy_write(phydev, RTL8201F_INER, ~RTL8201F_INER_MASK &
				reg_val);

	rtl8201f_select_page(phydev, 0);

	return err;
}

static int rtl8211b_config_intr(struct phy_device *phydev)
{
	int err;

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED)
		err = phy_write(phydev, RTL821x_INER,
				RTL821x_INER_INIT);
	else
		err = phy_write(phydev, RTL821x_INER, 0);

	return err;
}

static int rtl8211e_config_intr(struct phy_device *phydev)
{
	int err;

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED)
		err = phy_write(phydev, RTL821x_INER,
				RTL8211E_INER_LINK_STATUS);
	else
		err = phy_write(phydev, RTL821x_INER, 0);

	return err;
}

static int rtl8211f_config_intr(struct phy_device *phydev)
{
	int err;

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED)
		err = phy_write(phydev, RTL821x_INER,
				RTL8211F_INER_LINK_STATUS);
	else
		err = phy_write(phydev, RTL821x_INER, 0);

	return err;
}

static int rtl8211f_config_init(struct phy_device *phydev)
{
	int ret;
	u16 reg;

	ret = genphy_config_init(phydev);
	if (ret < 0)
		return ret;

	phy_write(phydev, RTL8211F_PAGE_SELECT, 0xd08);
	reg = phy_read(phydev, 0x11);

	/* enable TX-delay for rgmii-id and rgmii-txid, otherwise disable it */
	if (phydev->interface == PHY_INTERFACE_MODE_RGMII_ID ||
	    phydev->interface == PHY_INTERFACE_MODE_RGMII_TXID)
		reg |= RTL8211F_TX_DELAY;
	else
		reg &= ~RTL8211F_TX_DELAY;

	phy_write(phydev, 0x11, reg);
	/* restore to default page 0 */
	phy_write(phydev, RTL8211F_PAGE_SELECT, 0x0);

	return 0;
}

static struct phy_driver realtek_drvs[] = {
	{	/* RTL8201F */
		.phy_id		= 0x001cc816,
		.name		= "RTL8201F 10/100Mbps Ethernet",
		.phy_id_mask	= 0x001fffff,
		.features	= PHY_BASIC_FEATURES,
		.flags		= PHY_HAS_INTERRUPT,
		.config_aneg	= &genphy_config_aneg,
		.config_init	= &rtl8201f_config_init,
		.read_status	= &genphy_read_status,
		.ack_interrupt	= &rtl8201f_ack_interrupt,
		.config_intr	= &rtl8201f_config_intr,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
	},
	{
		.phy_id         = 0x00008201,
		.name           = "RTL8201CP Ethernet",
		.phy_id_mask    = 0x0000ffff,
		.features       = PHY_BASIC_FEATURES,
		.flags          = PHY_HAS_INTERRUPT,
		.config_aneg    = &genphy_config_aneg,
		.read_status    = &genphy_read_status,
	}, {
		.phy_id		= 0x001cc912,
		.name		= "RTL8211B Gigabit Ethernet",
		.phy_id_mask	= 0x001fffff,
		.features	= PHY_GBIT_FEATURES,
		.flags		= PHY_HAS_INTERRUPT,
		.config_aneg	= &genphy_config_aneg,
		.read_status	= &genphy_read_status,
		.ack_interrupt	= &rtl821x_ack_interrupt,
		.config_intr	= &rtl8211b_config_intr,
		.read_mmd	= &genphy_read_mmd_unsupported,
		.write_mmd	= &genphy_write_mmd_unsupported,
	}, {
		.phy_id		= 0x001cc914,
		.name		= "RTL8211DN Gigabit Ethernet",
		.phy_id_mask	= 0x001fffff,
		.features	= PHY_GBIT_FEATURES,
		.flags		= PHY_HAS_INTERRUPT,
		.config_aneg	= genphy_config_aneg,
		.read_status	= genphy_read_status,
		.ack_interrupt	= rtl821x_ack_interrupt,
		.config_intr	= rtl8211e_config_intr,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
	}, {
		.phy_id		= 0x001cc915,
		.name		= "RTL8211E Gigabit Ethernet",
		.phy_id_mask	= 0x001fffff,
		.features	= PHY_GBIT_FEATURES,
		.flags		= PHY_HAS_INTERRUPT,
		.config_aneg	= &genphy_config_aneg,
		.read_status	= &genphy_read_status,
		.ack_interrupt	= &rtl821x_ack_interrupt,
		.config_intr	= &rtl8211e_config_intr,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
	}, {
		.phy_id		= 0x001cc916,
		.name		= "RTL8211F Gigabit Ethernet",
		.phy_id_mask	= 0x001fffff,
		.features	= PHY_GBIT_FEATURES,
		.flags		= PHY_HAS_INTERRUPT,
		.config_aneg	= &genphy_config_aneg,
		.config_init	= &rtl8211f_config_init,
		.read_status	= &genphy_read_status,
		.ack_interrupt	= &rtl8211f_ack_interrupt,
		.config_intr	= &rtl8211f_config_intr,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
	},
};

module_phy_driver(realtek_drvs);

static struct mdio_device_id __maybe_unused realtek_tbl[] = {
	{ 0x001cc816, 0x001fffff },
	{ 0x001cc912, 0x001fffff },
	{ 0x001cc914, 0x001fffff },
	{ 0x001cc915, 0x001fffff },
	{ 0x001cc916, 0x001fffff },
	{ }
};

MODULE_DEVICE_TABLE(mdio, realtek_tbl);
