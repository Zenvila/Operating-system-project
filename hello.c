#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A Simple Hello World Kernel Module");

static int __init hello_init(void)
{
    printk(KERN_INFO "Hello, BCS_4A! Project Present to Sir Amin\n");
    return 0;
}

static void __exit hello_exit(void)
{
    printk(KERN_INFO "Goodbye, BCS_4A!\n");
}

module_init(hello_init);
module_exit(hello_exit);
