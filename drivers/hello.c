#include <linux/module.h>

MODULE_AUTHOR("my_name");
MODULE_LICENSE("GPL");
    
static int my_init(void)
{
    printk(KERN_INFO "my module Hello, World!\n");
    return 0;
}

static void my_exit(void)
{
    printk(KERN_INFO "Exit my module...\n");
}

module_init(my_init);
module_exit(my_exit);


