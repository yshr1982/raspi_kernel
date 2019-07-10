 /*
 * Copyright (C) 2019 Yasuhiro Yamamoto
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/stringify.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/of.h>
#include <linux/spi/spi.h>
#include <linux/property.h>

#define READ_BIT		(0b01000000)
#define MAKE_COMMAND(reg)	((reg) << 3)

#define SPI_MODE_MASK		(SPI_CPHA | SPI_CPOL | SPI_CS_HIGH \
				| SPI_LSB_FIRST | SPI_3WIRE | SPI_LOOP \
				| SPI_NO_CS | SPI_READY | SPI_TX_DUAL \
				| SPI_TX_QUAD | SPI_RX_DUAL | SPI_RX_QUAD)
				
//Register
#define STATUS			(0x00)
#define CONFIG			(0x01)
#define TEMP_VALUE		(0x02)
#define ID_VALUE		(0x03)

struct my_spi_data {
	struct spi_device	*spi;
	struct mutex		mutex;
	int			my_id;
	char			tx[16];
	char			rx[16];
};

static int spi_read_byte(struct device *dev, u8 reg)
{
	struct spi_device *spi = to_spi_device(dev);
	printk("%s %d\n",__func__,reg);
	reg = MAKE_COMMAND(reg)|READ_BIT;
	return spi_w8r8(spi, reg);
}

static int spi_write_byte(struct device *dev, u8 reg, u8 data)
{
	struct spi_device *spi = to_spi_device(dev);
	u8 buf[2];
	printk("%s %d %d\n",__func__,reg,data);

	buf[0] = MAKE_COMMAND(reg);
	buf[1] = data;

	return spi_write(spi, buf, sizeof(buf));
}

ssize_t my_temp_show(struct device *dev, struct device_attribute *da, char *buf)
{
	int ret;
	ret = spi_read_byte(dev,STATUS);
	return sprintf(buf, "%d\n", ret);
}

static ssize_t my_temp_set(struct device *dev,struct device_attribute *da, const char *buf, size_t count)
{
	char kdata[10];

	struct my_spi_data *data = dev_get_drvdata(dev);
	int ret;
	ret = kstrtol(buf, 10, &kdata);
	if (ret)
		return ret;

	mutex_lock(&data->mutex);
	ret = spi_write_byte(dev,CONFIG,kdata[0]);
	if (ret)
		count = ret;
	mutex_unlock(&data->mutex);
	return count;		
}

static struct kobj_attribute add_file_attr = __ATTR(my_temp, (S_IWUSR | S_IRUGO), spi_read_byte, spi_write_byte);
static struct kobj_attribute add_file_attr2 = __ATTR(adt7310, (S_IWUSR | S_IRUGO), my_temp_show, my_temp_set);
static struct attribute *my_attrs[] = {
	&add_file_attr.attr,
	&add_file_attr2.attr,
	NULL,
};
static const struct attribute_group my_attr_group = {
	.attrs = my_attrs,
};

static int my_spi_probe(struct spi_device *spi)
{
	int ret= 0;
	struct my_spi_data *data;
	
	data = devm_kzalloc(&spi->dev,sizeof(struct my_spi_data),GFP_KERNEL);	// allocate my spi data memory.

	if(!data)
		return -ENOMEM;
	mutex_init(&data->mutex);						// 資源管理フラグの初期化.
	spi_set_drvdata(spi,data);						// spi->dev->drvdata = data SPIのドライバーにプライベートデータを関連付ける.
	data->spi = spi;							// プライベートデータ内にSPIドライバーデータを関連付ける.
	spi->mode = (spi->mode&~SPI_MODE_MASK) | SPI_MODE_0;
	spi->max_speed_hz = 20000000;
	ret = spi_setup(spi);
	if(ret < 0)
	{
		return ret;
	}
	/* my driver initialize  */
	/* Register my driver */
        ret = sysfs_create_group(&spi->dev.kobj , &my_attr_group);
	if (ret < 0) {
		printk("%s Can't create file %d\n", __func__,ret);
		return ret;
	}
	/* ADT7310 Get ID*/
	ret = spi_read_byte(&spi->dev, ID_VALUE);
	if (ret < 0) {
		printk("%s Can't read ID? %d\n", __func__,ret);
		return ret;
	}
#if 0
	ret = sysfs_create_file(&(spi->dev.kobj), &add_file_attr.attr);
	if (ret < 0) {
		printk("%s Can't create file %d\n", __func__,ret);
		return ret;
	}

	/* ADT7310 Get ID*/
	ret = spi_read_byte(&spi->dev, ID_VALUE);
	if (ret < 0) {
		printk("%s Can't read ID? %d\n", __func__,ret);
		return ret;
	}

#endif

	printk("my_adt7310 driver probe done. ");
	return 0;
}

static int my_spi_remove(struct spi_device *spi)
{
        struct my_spi_data *data;
	data = spi_get_drvdata(spi);

	printk("my spi driver remove done. ");
	if(data == NULL)
	{
		return -ENODEV;
	}
	/* my driver finish */
	/*Unregister my driver*/
	mutex_destroy(&data->mutex);
	return 0;
}

/*-------------------------------------------------------------------------*/
static const struct spi_device_id my_spi_id[] = {
	{"my_adt7310", 0},
	{ }
};
MODULE_DEVICE_TABLE(spi, my_spi_id);

static const struct of_device_id my_spi_of_match[] = {
	{ .compatible = "spi,adt7310", },
	{ }
};
MODULE_DEVICE_TABLE(of, my_spi_of_match);

static struct spi_driver my_spi_driver = {
	.driver = {
		.name		= "my_adt7310",
		.of_match_table = of_match_ptr(my_spi_of_match),
	},
	.probe		= my_spi_probe,
	.remove		= my_spi_remove,
	.id_table	= my_spi_id,
};

module_spi_driver(my_spi_driver);

MODULE_DESCRIPTION("Driver for SPI sample");
MODULE_AUTHOR("Yamamoto Yasuhiro");
MODULE_LICENSE("GPL");
MODULE_ALIAS("spi:my_sample");

