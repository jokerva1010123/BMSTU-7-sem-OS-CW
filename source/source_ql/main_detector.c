#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>

#define MIN_NESSESSARY_MARKS 32

int main()
{
    int rc;
    int size;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    unsigned int val;
    int dir;
    snd_pcm_uframes_t frames;
    char *buffer;

    int i;
    int flag;

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
    snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);

    /* Set period size to 32 frames. */
    frames = 32;
    snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);

    /* Write the parameters to the driver */
    rc = snd_pcm_hw_params(handle, params);

    if (rc < 0)
    {
        fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
        exit(1);
    }

    /* Use a buffer large enough to hold one period */
    snd_pcm_hw_params_get_period_size(params, &frames, &dir);

    /* 2 bytes/sample, 2 channels */
    size = frames * 2;

    buffer = (char *)malloc(size);

    /* We want to loop for 5 seconds */
    snd_pcm_hw_params_get_period_time(params, &val, &dir);

    printf("start\n");
    flag = 0;
    while (1)
    {
        rc = snd_pcm_readi(handle, buffer, frames);
        if (rc == -EPIPE)
        {
            // EPIPE means overrun
            // fprintf(stderr, "overrun occurred\n");
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

        for (i = 0; i < size - 1; i++)
        {
            // if (buffer[i] == 127)
            printf("%02X ", (unsigned char)buffer[i]);

            // 7F FF
            if (buffer[i] == 0x3A && buffer[i + 1] == (char)0x08)
            {
                flag++;
                i += 1;
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
    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    free(buffer);
}