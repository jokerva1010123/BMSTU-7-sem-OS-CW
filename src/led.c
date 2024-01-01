#include <linux/module.h>
#include <linux/keyboard.h>
#include <linux/leds.h>

MODULE_LICENSE("GPL");

static struct notifier_block kb_led_nb;
static struct led_classdev kb_led;

static int kb_led_notifier(struct notifier_block *nb, unsigned long code, void *param)
{
    struct keyboard_notifier_param *kp = param;

    if (code == KBD_KEYCODE && kp->value == 30 && kp->down) {
        // Bật đèn LED khi bấm phím 'a'
        //led_set_brightness(&kb_led, LED_FULL);
        printk("al\n");
    } else if (code == KBD_KEYCODE && kp->value == 48 && kp->down) {
        // Tắt đèn LED khi bấm phím 'b'
        //led_set_brightness(&kb_led, LED_OFF);
        printk("b\n");
    }

    return NOTIFY_OK;
}

static int __init kb_led_init(void)
{
    kb_led.name = "kb_led";
    kb_led.brightness = LED_OFF;
    kb_led.blink_set = NULL;
    kb_led.brightness_set = NULL;

    kb_led_nb.notifier_call = kb_led_notifier;

    //led_classdev_register(NULL, &kb_led);
    register_keyboard_notifier(&kb_led_nb);
    printk("initl\n");
    return 0;
}

static void __exit kb_led_exit(void)
{
    printk("exitl\n");
    unregister_keyboard_notifier(&kb_led_nb);
    //led_classdev_unregister(&kb_led);
}

module_init(kb_led_init);
module_exit(kb_led_exit);