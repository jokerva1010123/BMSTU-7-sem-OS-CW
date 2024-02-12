#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>

void write_signal_to_proc()
{
    FILE *fp = fopen("/proc/module_dir/dev", "w");
    if (fp == NULL)
        printf("unable to open proc file\n");

    fprintf(fp, "1");
    fclose(fp);
}

snd_pcm_t *pcm_device;

int main()
{
    int count = 1;
    while(1)
    {
        int totalCards = 0;   // No cards found yet
        int cardNum = -1;     // Start with first card
        int err;

        for (;;) {
            // Get next sound card's card number.
            if ((err = snd_card_next(&cardNum)) < 0) {
                fprintf(stderr, "Can't get the next card number: %s\n",
                                snd_strerror(err));
                break;
            }
            if (cardNum < 0)
                // No more cards
                break;
            ++totalCards;   // Another card found, so bump the count
        }
        
        if(count < totalCards)
        {
            //printf("%d %d\n", count, totalCards);
            printf("Headset connected\n");
            write_signal_to_proc();
            write_signal_to_proc();
        }
        else if(count > totalCards)
        {
            //printf("%d %d\n", count, totalCards);
            printf("Headset disconnected\n");
            write_signal_to_proc();
            write_signal_to_proc();
        }
        count = totalCards;
        // ALSA allocates some memory to load its config file when we call
        // snd_card_next. Now that we're done getting the info, tell ALSA
        // to unload the info and release the memory.
        snd_config_update_free_global();
    }
}
