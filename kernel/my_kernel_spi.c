

/*
 *  74Hx164 - Generic serial-in/parallel-out 8-bits shift register GPIO driver
 *
 *  Copyright (C) 2010 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2010 Miguel Gaio <miguel.gaio@efixo.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <linux/gpio/consumer.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/spi/spi.h>
#include <asm/unaligned.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/module.h>

#define adt7310_NUMBER_GPIOS	8

struct my_sysfs_reg {
	struct device_attribute attr;
	char name[2];
};

struct my_plat_data {
	struct rtc_device *rtc;
	struct my_sysfs_reg regs[16];
};

/* 自作SPIデータ構造体 */
struct myspi_data {
	struct spi_device	*spi;
	struct mutex		lock;
	u32			registers;
	u8			buffer[];
};

struct myspi_ops {
	int (*read_byte)(struct device *, u8 reg);
	int (*write_byte)(struct device *, u8 reg, u8 data);
	int (*read_word)(struct device *, u8 reg);
	int (*write_word)(struct device *, u8 reg, u16 data);
};
static int myspi_read_byte(struct device *dev, u8 reg)
{
	struct spi_device *spi = to_spi_device(dev);

	return spi_w8r8(spi, reg);
}

static int myspi_write_byte(struct device *dev, u8 reg, u8 data)
{
	struct spi_device *spi = to_spi_device(dev);
	u8 buf[2];

	buf[0] = reg;
	buf[1] = data;

	return spi_write(spi, buf, sizeof(buf));
}

static int myspi_read_word(struct device *dev, u8 reg)
{
	struct spi_device *spi = to_spi_device(dev);

	return spi_w8r16be(spi, reg);
}

static int myspi_write_word(struct device *dev, u8 reg, u16 data)
{
	struct spi_device *spi = to_spi_device(dev);
	u8 buf[3];

	buf[0] = reg;
	put_unaligned_be16(data, &buf[1]);

	return spi_write(spi, buf, sizeof(buf));
}


static int myspi_probe(struct spi_device *spi)
{

	struct myspi_data *data;
	int ret;

	data = devm_kzalloc(&spi->dev, sizeof(*data), GFP_KERNEL);

	if(!data){
		return -ENOMEM;
	}
	mutex_init(&data->lock);
	spi_set_drvdata(spi,data);		// SPI deviceデータと自分が確保したmyspi_dataとの関連付け. spi->dev->driver_dataにdataを関連付ける.
	data->spi = spi;

	/* SPI controller setup */
	spi->mode = 0;					// SPI MODE
	spi->max_speed_hz = 20000000;	// Max clock for the device.
	spi->bits_per_word = 16;		// device bit per word.
	ret = spi_setup(spi);

	// Make some init.

	// SPIフレームワークへの登録を行う.


	return ret;
}

static int myspi_remove(struct spi_device *spi)
{
	struct myspi_data *data;

	data = spi_get_drvdata(spi);
	if(!data){
		return -ENODEV;
	}

	/* フレームワークから登録を解除する処理をここに実装する */

	/* 資源保護 解放する. */
	mutex_destroy(&data->lock);
	return 0;
}

//この構造体をspi driverとして登録する
// 
// SIMPLE_DEV_PM_OPSを用いてresumeとsuspend
const static const struct myspi_ops myspi_ops = {
	.read_word = myspi_read_word,
	.write_word = myspi_write_word,
	.read_byte = myspi_read_byte,
	.write_byte = myspi_write_byte,
};
// include/linux/mod_devicetable.h にspi_device_idが定義. 
// id_tableが一致するたびにprobe関数を呼ばれる.
// 
static const struct spi_device_id myspi_id[] = {
	{ "adt7310", 0 },
	{}
};

static const struct of_device_id myspi_dt_ids[] = {
	{ .compatible = "sample,adt7310" },
	{ .compatible = "new_device,adt7310" },
	{},
};
MODULE_DEVICE_TABLE(of, myspi_dt_ids);

// これが必要。何を持ってしてもこの構造体を使ってSPIプラットフォームモジュールは自作ドライバをアクセスする
// module_spi_driverにてspi initに入れてやるとdrv->driver.probeとして登録される
// driver_registerにて登録.
static struct spi_driver myspi_driver = {
	.driver = {
		.name	= "adt7310",
//		.pm	= adt7310_spi_ops,
	},
	.probe		= myspi_probe,
	.remove		= myspi_remove,
	/* spi_match_id(sdrv->id_table, sdev) で使用する. */
	.id_table	= myspi_id,
};

/*
module_spi_driver内部でspi_reguster_driver / spi_unregister_driverが呼ばれ、自作のprobe関数とremove関数が登録される
もしmodule_spi_driverを使わない方法を取りたい場合(spi platform driverが動き出す前に何かしたいなど)は、drivers/gpio/gpio-mc33880.cのようにsubsys_initcallで初期化関数を登録して、
初期化関数内でspi_register_driverを使うと良い
*/
module_spi_driver(myspi_driver);

MODULE_AUTHOR("Yasuhiro Yamamoto <yshr1982@gmail.com>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("sample SPI kernel driver");

