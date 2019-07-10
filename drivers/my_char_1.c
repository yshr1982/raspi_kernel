#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/cdev.h>

MODULE_AUTHOR("Yasuhiro Yamamoto");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("my char kernel module");
    
#define MINOR_BASE      3
#define MINOR_NUM       10
#define DRIVER_NAME     "my_device"
#define CLASS_NAME     "my_class"

static struct class *my_class = NULL;
static struct cdev *my_char = NULL;

// vvv system callに対応する関数の実装 vvv
/* open時に呼ばれる関数 */
static int my_open(struct inode *inode, struct file *file)
{
    printk("%s¥n",__func__);
    return 0;
}
/* close時に呼ばれる関数 */
static int my_close(struct inode *inode, struct file *file)
{
    printk("%s¥n",__func__);
    return 0;
}
/* read時に呼ばれる関数 */
static ssize_t my_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    printk("%s¥n",__func__);
    return 1;
}
/* write時に呼ばれる関数 */
static ssize_t my_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    printk("%s¥n",__func__);
    return 1;
}
/* llseek時に呼ばれる関数 */
static loff_t my_llseek(struct file *filp, loff_t offset,int whence)
{
    printk("%s¥n",__func__);
    return 1;
}
static unsigned int my_poll(struct file *filp, struct poll_table_struct *poll_tbl)
{
    printk("%s¥n",__func__);
    return 1;
}
/* unlocked ioctl時に呼ばれる関数 */
static long my_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    printk("%s¥n",__func__);
    return 1;
}
/* compat ioctl時に呼ばれる関数 */
static long my_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    printk("%s¥n",__func__);
	return my_unlocked_ioctl(filp, cmd, arg);
}
// ^^^ system callに対応する関数 ^^^

/* System Callに対応するAPIの登録 */
struct file_operations my_fops = {
    .open    = my_open,
    .release = my_close,
    .read    = my_read,
    .write   = my_write,
    .llseek  = my_llseek,
    .poll    = my_poll,
	.unlocked_ioctl = my_unlocked_ioctl,
	.compat_ioctl = my_compat_ioctl,
};

static int __init my_init(void)
{
    int ret;
    dev_t device_id;
    size_t size;

    ret = alloc_chrdev_region(&device_id, MINOR_BASE, MINOR_NUM, DRIVER_NAME);      // 自動でメジャー番号を取得する.                                                 // メジャー番号の値を取得
    my_class = class_create(THIS_MODULE,CLASS_NAME);                                // Driver Classの構造体を取得する.
    size = sizeof(struct cdev);
    my_char =  (struct cdev*)kmalloc(size, GFP_KERNEL);                             // charactor device管理データ領域の確保.
    cdev_init(my_char,&my_fops);                                                    // キャラクタ型デバイスを登録する.
    my_char->owner = THIS_MODULE;
    device_id = MKDEV(MAJOR(device_id), MINOR(device_id));                          // デバイスノードの設定を行う.
    if( cdev_add(my_char,device_id,1) < 0 ) {                                       // 
        /* 登録に失敗した */
        printk(KERN_ERR "cdev_add failed minor = %d\n", device_id );
    } else {
        /* デバイスノードの作成 /dev/以下にDRIVER_NAMEで登録される */
        device_create( my_class,
                NULL,
                device_id,
                NULL,
                DRIVER_NAME"%u",MINOR(device_id));
    }
    printk(KERN_INFO "%s \n",__func__);
    return 0;
}

static void __exit my_exit(void)
{
    printk(KERN_INFO "%s \n",__func__);
}

module_init(my_init);
module_exit(my_exit);

