#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
#define KBD_DATA_REG        0x60    /* I/O port for keyboard data */
#define KBD_CONTROL_REG        0x64 
#define KBD_SCANCODE_MASK   0x7f
#define KBD_STATUS_MASK     0x80
#define DELAY do { mdelay(1); if (++delay > 10) break; } while(0)
static void set_kbd_led_on(void)
{
	long delay = 0;
	//wait till the input buffer is empty
	while (inb(KBD_CONTROL_REG) & 0x02)
		DELAY;
	outb(0xED, KBD_DATA_REG);
	DELAY;
	while (inb(KBD_CONTROL_REG) & 0x02)
		DELAY;
	outb(0x07, KBD_DATA_REG);
	DELAY;
}
static void set_kbd_led_off(void)
{
	long delay = 0;
	//wait till the input buffer is empty
	while (inb(KBD_CONTROL_REG) & 0x02)
		DELAY;
	outb(0xED, KBD_DATA_REG);
	DELAY;
	while (inb(KBD_CONTROL_REG) & 0x02)
		DELAY;
	outb(0x00, KBD_DATA_REG);
	DELAY;
}

static int test_kbd_led_init(void)
{
        pr_info("%s: In init\n", __func__);
	set_kbd_led_on();
	return 0;
}

static void test_kbd_led_exit(void)
{
        pr_info("%s: In exit\n", __func__);
	set_kbd_led_off();
}

module_init(test_kbd_led_init);
module_exit(test_kbd_led_exit);