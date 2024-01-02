#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/interrupt.h>
#include <linux/timer.h>

/* for led keyboard */
#include <linux/tty.h>            /* For fg_console, MAX_NR_CONSOLES */
#include <linux/kd.h>             /* For KDSETLED */
#include <linux/console_struct.h> /* For vc_cons */
#include <linux/vt_kern.h>

MODULE_AUTHOR("anhthu");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Toggle keyboard LED when '1' is written in file proc");
// Переключить светодиод клавиатуры, когда в файл proc записывается '1'

struct tty_driver *my_driver;

#define ALL_LEDS_ON 0x07
#define RESTORE_LEDS 0xFF

int is_led_on = 0;

void tasklet_fn_toggle_led(unsigned long data)
{
    unsigned long status;

    if (is_led_on)
    {
        is_led_on = 0;
        status = ALL_LEDS_ON;
    }
    else
    {
        is_led_on = 1;
        status = RESTORE_LEDS;
    }

    (my_driver->ops->ioctl)(vc_cons[fg_console].d->port.tty, KDSETLED, status);
}

struct tty_driver *load_keyboard_led_driver(void)
{
    int i;
    struct tty_driver *driver;

    printk(KERN_INFO "kbleds: loading\n");
    printk(KERN_INFO "kbleds: fgconsole is %x\n", fg_console);
    printk(KERN_INFO "kbleds: MAX_NR_CONSOLES %i", MAX_NR_CONSOLES);
    
    for (i = 0; i < MAX_NR_CONSOLES; i++)
    {
        if (!vc_cons[i].d)
            break;
        printk(KERN_INFO "poet_atkm: console[%i/%i] #%i, tty %lx\n", i,
               MAX_NR_CONSOLES, vc_cons[i].d->vc_num,
               (unsigned long)vc_cons[i].d->port.tty);
    }

    printk(KERN_INFO "kbleds: finished scanning consoles\n");

    driver = vc_cons[fg_console].d->port.tty->driver;
    printk(KERN_INFO "kbleds: tty driver magic %x\n", driver->magic);

    return driver;
}

/*----------------------------------------------------------------- */

/* message from user space */
static char msg[8];
static ssize_t msg_length;
static int is_empty = 1;

/* elements procfs */
struct proc_dir_entry *kmodule_dev, *kmodule_dir;
static const char *dev = "dev";
static const char *dir = "module_dir";

DECLARE_TASKLET_OLD(my_tasklet, tasklet_fn_toggle_led);

int module_dev_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "module_dev: called open\n");
    return 0;
}

int module_dev_release(struct inode *indoe, struct file *file)
{
    printk(KERN_INFO "module_dev: called release\n");
    return 0;
}

ssize_t module_dev_read(struct file *file, char __user *buf, size_t size, loff_t *offset)
{
    if (is_empty)
        return 0;

    is_empty = 1;

    printk(KERN_INFO "module_dev: called read\n");
    copy_to_user(buf, msg, msg_length);

    return msg_length;
}

ssize_t module_dev_write(struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
    printk(KERN_INFO "module_dev: called write %ld\n", size);

    copy_from_user(msg, buf, size);
    msg_length = size;

    is_empty = 0;

    if (msg[0] == '1')
    {
        tasklet_schedule(&my_tasklet);
    }
    printk(KERN_INFO "module_dev: write msg '%s'\n", msg);

    return size;
}

struct proc_ops fops = {
    .proc_open = module_dev_open,
    .proc_read = module_dev_read,
    .proc_write = module_dev_write,
    .proc_release = module_dev_release};

static int __init init_module_dev(void)
{
    printk(KERN_INFO "module_dev: init\n");

    kmodule_dir = proc_mkdir(dir, NULL);
    kmodule_dev = proc_create(dev, 0666, kmodule_dir, &fops);

    my_driver = load_keyboard_led_driver();

    return 0;
}

static void __exit exit_module_dev(void)
{
    tasklet_kill(&my_tasklet);

    remove_proc_entry(dev, kmodule_dir);
    remove_proc_entry(dir, NULL);

    printk(KERN_INFO "module_dev: exit\n");
}

module_init(init_module_dev);
module_exit(exit_module_dev);