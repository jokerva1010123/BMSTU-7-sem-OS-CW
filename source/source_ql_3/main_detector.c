#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>

#define MIN_NESSESSARY_MARKS 16
#define SIG_1 0x09
#define SIG_2 0x99
// 7F FF

void alsa_setup(snd_pcm_uframes_t *frames);

void alsa_cleanup();

snd_pcm_t *handle;

int main()
{
    snd_pcm_uframes_t frames;

    alsa_setup(&frames);

    /* 2 bytes/sample, 2 channels */
    int size = frames * 2;

    char *buffer = (char *)malloc(size);

    printf("start %d\n", frames);
    printf("%p %d\n", buffer, size);

    int flag = 0;
    while (1)
    {
        int rc = snd_pcm_readi(handle, buffer, frames);
        if (rc == -EPIPE)
        {
            // EPIPE means overrun
            fprintf(stderr, "overrun occurred\n");
            snd_pcm_prepare(handle);
        }
        else if (rc < 0)
        {
            fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
        }
        else if (rc != (int)frames)
        {
            fprintf(stderr, "short read, read %d frames\n", rc);
        }

        // printf("%d size %d\n", rc, size);

        for (int i = 0; i < size - 2; i++)
        {
            printf("%02x ", (unsigned char)buffer[i]);
            
            if (buffer[i] == (char)SIG_1 && buffer[i + 1] == (char)SIG_2)
            {
                flag++;
                i += 1;
                printf("%02x ", (unsigned char)buffer[i]);
                if (flag >= MIN_NESSESSARY_MARKS)
                {
                    printf("push\n");
                    system("echo 1 > /proc/module_dir/dev");
                    flag = 0;
                    break;
                }
            }
            else
            {
                flag = 0;
            }
        }
        printf("\n");
    }

    free(buffer);

    alsa_cleanup(handle);
}

void alsa_setup(snd_pcm_uframes_t *frames)
{
    int rc, dir;
    unsigned int val;
    snd_pcm_hw_params_t *params;

    /* Open PCM device for recording (capture). */
    rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_CAPTURE, 0);

    if (rc < 0)
    {
        fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
        exit(1);
    }

    /* Allocate a hardware parameters object. */
    snd_pcm_hw_params_alloca(&params);

    /* Fill it in with default values. */
    snd_pcm_hw_params_any(handle, params);

    /* Set the desired hardware parameters. */

    /* Interleaved mode */
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

    /* Signed 16-bit little-endian format */
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);

    /* Two channels (stereo) */
    //snd_pcm_hw_params_set_channels(handle, params, 1);
    snd_pcm_hw_params_set_channels(handle, params, 2);

    /* 44100 bits/second sampling rate (CD quality) */
    // val = 44100;
    val = 100;
    val = 400;
    snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);

    /* Set period size to 32 frames. */
    *frames = 32;
    snd_pcm_hw_params_set_period_size_near(handle, params, frames, &dir);

    /* Write the parameters to the driver */
    rc = snd_pcm_hw_params(handle, params);

    if (rc < 0)
    {
        fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
        exit(1);
    }

    /* Use a buffer large enough to hold one period */
    snd_pcm_hw_params_get_period_size(params, frames, &dir);

    /* We want to loop for 5 seconds */
    snd_pcm_hw_params_get_period_time(params, &val, &dir);
}

void alsa_cleanup()
{
    snd_pcm_drain(handle);
    snd_pcm_close(handle);
}