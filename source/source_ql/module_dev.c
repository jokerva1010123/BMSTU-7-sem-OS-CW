#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/interrupt.h>
#include <linux/timer.h>

/*for led keyboard */
#include <linux/configfs.h>
#include <linux/init.h>
#include <linux/tty.h> /* For fg_console, MAX_NR_CONSOLES */
#include <linux/kd.h>  /* For KDSETLED */
#include <linux/vt.h>
#include <linux/console_struct.h> /* For vc_cons */
#include <linux/vt_kern.h>

MODULE_AUTHOR("anhthu");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A module using proc filesystem");

struct timer_list my_timer;
struct tty_driver *my_driver;
char my_timer_func_data = 0;
// a flag indicating that the sos message is still playing
int run = 0;

#define SHORT_DELAY HZ / 5
#define LONG_DELAY 5 * HZ / 5
#define START_DELAY HZ
#define ALL_LEDS_ON 0x07
#define RESTORE_LEDS 0xFF
#define NUM_OF_LEDS 5

int sos[NUM_OF_LEDS] = {SHORT_DELAY, SHORT_DELAY, LONG_DELAY, SHORT_DELAY, SHORT_DELAY};
// current index in array sos
int pos = 0;

//https://stackoverflow.com/questions/24987822/blink-keyboard-led-one-at-a-time-driver-in-linux
static void my_timer_func(struct timer_list *timer)
{
	int *pstatus = (int *)&my_timer_func_data;

	if (pos == NUM_OF_LEDS)
	{
		pos = 0;
		del_timer(&my_timer);
		(my_driver->ops->ioctl)(vc_cons[fg_console].d->port.tty, KDSETLED, RESTORE_LEDS);
		run--;
		return;
	}

	printk(KERN_INFO "kbleds: pstatus = %d\n", *pstatus);

	int delay;

	if (*pstatus == ALL_LEDS_ON)
	{
		*pstatus = RESTORE_LEDS;
		delay = SHORT_DELAY;
	}
	else
	{
		*pstatus = ALL_LEDS_ON;
		delay = sos[pos];

		++pos;
	}

	(my_driver->ops->ioctl)(vc_cons[fg_console].d->port.tty, KDSETLED, *pstatus);

	my_timer.expires = jiffies + delay;

	add_timer(&my_timer);
}

void my_tasklet_function(unsigned long data)
{
	if (run)
	{
		printk(KERN_INFO "kbleds: already return");
		return;
	}
	++run;

	int i;
	printk(KERN_INFO "kbleds: loading\n");
	printk(KERN_INFO "kbleds: fgconsole is %x\n", fg_console);

	for (i = 0; i < MAX_NR_CONSOLES; i++)
	{
		if (!vc_cons[i].d)
			break;
		printk(KERN_INFO "poet_atkm: console[%i/%i] #%i, tty %lx\n", i,
			   MAX_NR_CONSOLES, vc_cons[i].d->vc_num,
			   (unsigned long)vc_cons[i].d->port.tty);
	}

	printk(KERN_INFO "kbleds: finished scanning consoles\n");
	my_driver = vc_cons[fg_console].d->port.tty->driver;
	printk(KERN_INFO "kbleds: tty driver magic %x\n", my_driver->magic);

	/* Set up the LED blink timer the first time */
	timer_setup(&my_timer, my_timer_func, 0);

	mod_timer(&my_timer, jiffies + msecs_to_jiffies(START_DELAY));

	return;
}

/*----------------------------------------------------------------- */

/* message from user space */
static char msg[8];
static ssize_t length_msg;
static int is_empty = 1;

/* elements procfs */
struct proc_dir_entry *kmodule_dev, *kmodule_dir;
static const char *dev = "dev";
static const char *dir = "module_dir";

// DECLARE_TASKLET(my_tasklet, my_tasklet_function, NULL);
DECLARE_TASKLET_OLD(my_tasklet, my_tasklet_function);

int module_dev_open(struct inode *sp_inode, struct file *sp_file)
{
	printk(KERN_INFO "module_dev: called open\n");
	return 0;
}

int module_dev_release(struct inode *sp_indoe, struct file *sp_file)
{
	printk(KERN_INFO "module_dev: called release\n");
	return 0;
}

ssize_t module_dev_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset)
{
	if (is_empty)
		return 0;

	is_empty = 1;

	printk(KERN_INFO "module_dev: called read\n");
	copy_to_user(buf, msg, length_msg);

	return length_msg;
}

ssize_t module_dev_write(struct file *sp_file, const char __user *buf, size_t size, loff_t *offset)
{
	printk(KERN_INFO "module_dev: called write %ld\n", size);

	copy_from_user(msg, buf, size);
	length_msg = size;

	is_empty = 0;

	int event;

	printk(KERN_INFO "debug proc: %s\n", msg);

	if (msg[0] == '1')
	{
		event = 1;
		tasklet_schedule(&my_tasklet);
	}
	else
	{
		event = 0;
	}

	printk(KERN_INFO "module_dev: write event = %d\n", event);

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
