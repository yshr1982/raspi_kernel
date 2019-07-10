#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
//#include <linux/kernel.h>
#include <linux/fs.h>
//#include <linux/cdev.h>
//#include <linux/sched.h>
//#include <asm/current.h>
//#include <asm/uaccess.h>

#define DRIVER_NAME "my_kernel_NAME"
#define DRIVER_MAJOR 63

/* open時に呼ばれる関数 */
static int my_kernel_open(struct inode *inode, struct file *file)
{
    printk("my_kernel_open");
    return 0;
}

/* close時に呼ばれる関数 */
static int my_kernel_close(struct inode *inode, struct file *file)
{
    printk("my_kernel_close");
    return 0;
}

/* read時に呼ばれる関数 */
static ssize_t my_kernel_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    printk("my_kernel_read");
    buf[0] = 'A';
    return 1;
}

/* write時に呼ばれる関数 */
static ssize_t my_kernel_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    printk("my_kernel_write");
    return 1;
}

/* 各種システムコールに対応するハンドラテーブル */
struct file_operations s_my_kernel_fops = {
    .open    = my_kernel_open,
    .release = my_kernel_close,
    .read    = my_kernel_read,
    .write   = my_kernel_write,
};

/* ロード(insmod)時に呼ばれる関数 */
static int my_kernel_init(void)
{
    printk("my_kernel_init\n");
    /* ★ カーネルに、本ドライバを登録する */
    register_chrdev(DRIVER_MAJOR, DRIVER_NAME, &s_my_kernel_fops);
    return 0;
}

/* アンロード(rmmod)時に呼ばれる関数 */
static void my_kernel_exit(void)
{
    printk("my_kernel_exit\n");
    unregister_chrdev(DRIVER_MAJOR, DRIVER_NAME);
}

module_init(my_kernel_init);
module_exit(my_kernel_exit);