#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Keyboard LED control module");

static int __init inita(void)
{
    printk("inita\n");
    return 0;
}

static void __exit exita(void)
{
    printk("exita\n");
}

module_init(inita);
module_exit(exita);