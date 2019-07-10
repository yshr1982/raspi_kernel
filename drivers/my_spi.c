/*
* Copyright (C) 2019 Yasuhiro Yamamoto
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/of.h>
#include <linux/spi/spi.h>
#include <linux/property.h>

#define SPI_MODE_MASK		(SPI_CPHA | SPI_CPOL | SPI_CS_HIGH \
				| SPI_LSB_FIRST | SPI_3WIRE | SPI_LOOP \
				| SPI_NO_CS | SPI_READY | SPI_TX_DUAL \
				| SPI_TX_QUAD | SPI_RX_DUAL | SPI_RX_QUAD)

struct my_spi_data {
        struct spi_device       *spi;
        int                     my_id;
        char                    tx[16];
        char                    rx[16];
};


static int my_spi_probe(struct spi_device *spi)
{
        int ret;

        spi->mode = (spi->mode&~SPI_MODE_MASK) | SPI_MODE_0;
        
        spi->max_speed_hz = 10000000;
        spi->bits_per_word = 16;
        ret = spi_setup(spi);
        if(ret < 0)
        {
		dev_err(&spi->dev,"setup error. \n");
                return ret;
        }
        /* my driver initialize  */
        /* Register my driver */

        return 0;
}

static int my_spi_remove(struct spi_device *spi)
{
        struct my_spi_data *data;
        data = spi_get_drvdata(spi);

        if(data == NULL)
        {
                return -ENODEV;
        }
        /* my driver finish */
        /*Unregister my driver*/
        return 0;
}

/*-------------------------------------------------------------------------*/

static const struct of_device_id my_spi_of_match[] = {
        { .compatible = "spi,my_sample", },
        { }
};
MODULE_DEVICE_TABLE(of, my_spi_of_match);

static struct spi_driver my_spi_driver = {
        .driver = {
                .name           = "my_sample",
                .of_match_table = of_match_ptr(my_spi_of_match),
        },
        .probe          = my_spi_probe,
        .remove         = my_spi_remove,
};

module_spi_driver(my_spi_driver);

MODULE_DESCRIPTION("Driver for SPI sample");
MODULE_AUTHOR("Yamamoto Yasuhiro");
MODULE_LICENSE("GPL");
MODULE_ALIAS("spi:my_sample");
