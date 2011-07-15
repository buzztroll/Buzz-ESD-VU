#include <stdlib.h>
#include <stdio.h>
#include <esd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT            2348
#define N_SAMPLES       1024
#define SAMPLE_RATE     44100
#define WIDTH           5

/* The format of the esd data to monitor.  STEREO is interleaved channels */
#define SOUND_FORMAT (ESD_BITS16 | ESD_STEREO | ESD_STREAM | ESD_PLAY)

typedef struct
    {
    short       left,
                right;
    }
SoundSample;

static int g_done = 0;
static void cnt_c(int x)
{
    g_done = 1;
}




int
main(
    int                                 argc,
    char **                             argv)
{
    int                                 fd;
	int                                 count;
    SoundSample *                       ss;
    int                                 buf_len;
    int                                 l;
    int                                 r;
    int                                 ml = 1;
    int                                 mr = 1;
    int                                 x;
    int                                 i;
    char                                VUr[WIDTH+2];
    char                                VUl[WIDTH+2];
    char                                packet[128];
    float                               xf;
    struct sockaddr_in                  si_other;
    int                                 s;
    int                                 slen=sizeof(si_other);
    int                                 rc;
    int                                 xr;
    char *                              srv_ip;

    srv_ip = argv[1];

    slen = sizeof(si_other);
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
    {
        printf("failed to make the socket\n");
        exit(1);
    }
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
    if (inet_aton(srv_ip, &si_other.sin_addr)==0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    memset(VUl, '\0', WIDTH+1);
    memset(VUr, '\0', WIDTH+1);
    memset(packet, '\0', 128);
    fd = esd_monitor_stream(
        SOUND_FORMAT, SAMPLE_RATE, NULL, "gkrellmss");
    if (fd < 0)
    {
        printf("vouldnt open it succa\n");
		esd_close(fd);
        return;
    }

    signal(2, cnt_c);
    count = 1024;
    buf_len = sizeof(SoundSample) * count;
    ss = (SoundSample *) calloc(sizeof(SoundSample), count);
    while(!g_done)
    {
	    count = read(fd, ss, buf_len);
    	if (count < 0)
        {
            goto err;
        }
        l = abs(ss[0].left);
        r = abs(ss[0].right);
        l = l * 707 / 1000;
        r = r * 707 / 1000;

        if(l > ml) ml = l;
        if(r > mr) mr = r;

        memset(VUr, '0', WIDTH);
        xf = (float)WIDTH * (float)r/(float)mr;
        x = (int)xf;
        if (xf - (float)x > 0.5)
        {
            x++;
        } 
        xr = x;
        for(i = 0; i < x; i++)
        {
            VUr[i] = 'F';
        }

        memset(VUl, '0', WIDTH);
        xf = (float)WIDTH * (float)l/(float)ml;
        x = (int)xf;
        if (xf - (float)x > 0.5)
        {
            x++;
        } 
        for(i = 0; i < x; i++)
        {
            VUl[WIDTH-i] = 'F';
        }
        VUl[WIDTH+1] = '\0';
        printf("%d| %s||%s |%d\r", xr, VUl, VUr, x);

        buf_len = strlen(packet);
        sprintf(packet, "uid 0FFFFFFFFF 0%s%s", VUl, VUr);
        rc = sendto(s, packet, buf_len, 0, &si_other, slen);
        if(rc < 0)
        {
            printf("failed to send a packet");
            exit(3);
        }
        
        sprintf(packet, "cab 0FFFFFFFFF 0%s%s", VUl,VUr);
        rc = sendto(s, packet, buf_len, 0, &si_other, slen);
        if(rc < 0)
        {
            printf("failed to send a packet");
            exit(3);
        }

        usleep(200);
    }
    close(s);
    esd_close(fd);

    printf("\n");
    printf("done\n");
    return 0;

err:
    esd_close(fd);

    return 1;
}
