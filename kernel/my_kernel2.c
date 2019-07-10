#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <asm/current.h>
#include <asm/uaccess.h>

/* /proc/devices等で表示されるデバイス名 */
#define DRIVER_NAME "prac"

struct something *item1; struct somethingelse *item2; int stuff_ok;

/* このデバイスドライバで使うマイナー番号の開始番号と個数(=デバイス数) */
static const unsigned int MINOR_BASE = 0;
static const unsigned int MINOR_NUM  = 2;   /* マイナー番号は 0 ~ 1 */

/* このデバイスドライバのメジャー番号(動的に決める) */
static unsigned int prac_major;

/* キャラクタデバイスのオブジェクト */
static struct cdev prac_cdev;
static struct class *prac_class = NULL;

/* open時に呼ばれる関数 */
static int prac_open(struct inode *inode, struct file *file)
{
    printk("prac_open");
    return 0;
}

/* close時に呼ばれる関数 */
static int prac_close(struct inode *inode, struct file *file)
{
    printk("prac_close");
    return 0;
}

/* read時に呼ばれる関数 */
static ssize_t prac_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    printk("prac_read");
    buf[0] = 'A';
    return 1;
}

/* write時に呼ばれる関数 */
static ssize_t prac_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    printk("prac_write");
    return 1;
}

/* 各種システムコールに対応するハンドラテーブル */
struct file_operations s_prac_fops = {
    .open    = prac_open,
    .release = prac_close,
    .read    = prac_read,
    .write   = prac_write,
};

/* ロード(insmod)時に呼ばれる関数 */
static int prac_init(void)
{

    int alloc_ret = 0;
    int cdev_err = 0;
    int minor = 0;
    dev_t dev;

    printk("prac_init\n");
    /* 1. 空いているメジャー番号を確保する */
    alloc_ret = alloc_chrdev_region(&dev, MINOR_BASE, MINOR_NUM, DRIVER_NAME);
    if (alloc_ret != 0) {
        printk(KERN_ERR  "alloc_chrdev_region = %d\n", alloc_ret);
        return -1;
    }

    /* 2. 取得したdev( = メジャー番号 + マイナー番号)からメジャー番号を取得して保持しておく */
    prac_major = MAJOR(dev);
    dev = MKDEV(prac_major, MINOR_BASE);    /* 不要? */

    /* 3. cdev構造体の初期化とシステムコールハンドラテーブルの登録 */
    cdev_init(&prac_cdev, &s_prac_fops);
    prac_cdev.owner = THIS_MODULE;

    /* 4. このデバイスドライバ(cdev)をカーネルに登録する */
    cdev_err = cdev_add(&prac_cdev, dev, MINOR_NUM);
    if (cdev_err != 0) {
        printk(KERN_ERR  "cdev_add = %d\n", alloc_ret);
        unregister_chrdev_region(dev, MINOR_NUM);
        return -1;
    }

    /* 5. カーネルに登録したデバイスをより上位の概念であるクラスに登録する */
    prac_class = class_create(THIS_MODULE, "prac_class");
    if (IS_ERR(prac_class)) {
        printk(KERN_ERR  "class_create\n");
        cdev_del(&prac_cdev);
        unregister_chrdev_region(dev, MINOR_NUM);
        return -1;
    }

    /* 6. カーネルに登録したデバイスをdevフォルダに登録する.  */
    for (minor = MINOR_BASE; minor < MINOR_BASE + MINOR_NUM; minor++) {
        device_create(prac_class, NULL, MKDEV(prac_major, minor), NULL, "prac_dev%d", minor);
    }


    return 0;
}

/* アンロード(rmmod)時に呼ばれる関数 */
static void prac_exit(void)
{
    dev_t dev = MKDEV(prac_major, MINOR_BASE);
    printk("prac_exit\n");

    /* 5. このデバイスドライバ(cdev)をカーネルから取り除く */
    cdev_del(&prac_cdev);

    /* 6. このデバイスドライバで使用していたメジャー番号の登録を取り除く */
    unregister_chrdev_region(dev, MINOR_NUM);
}

MODULE_DESCRIPTION("sample kernel driver");
MODULE_LICENSE("GPL");

module_init(prac_init);
module_exit(prac_exit);

