/* quirc -- QR-code recognition library
 * Copyright (C) 2010-2012 Daniel Beer <dlbeer@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include "Demuxing.h"

#include <SDL.h>
#include <SDL_gfxPrimitives.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <quirc.h>
#include <time.h>
#include <getopt.h>

#include "camera.h"
#include "mjpeg.h"
extern "C" {
#include "convert.h"
#include "dthash.h"
}

#include "demoutil.h"
#include <sys/time.h>

/* Collected command-line arguments */
static const char *camera_path = "/dev/video0";
static const char *qr_file_name = "qryuv.yuv";
static int  g_warpeyes = 0;
static int  g_contrast = 0;

static int video_width = 640;
static int video_height = 480;
static int want_verbose = 0;
static int printer_timeout = 2;

// static int main_loop(struct camera *cam,
// 		     struct quirc *q, struct mjpeg_decoder *mj)
// {
// 	return 0;
	// struct dthash dt;

	// dthash_init(&dt, printer_timeout);

	// for (;;) {
	// 	int w, h;
	// 	int i, count;
	// 	// uint8_t *buf = quirc_begin(q, &w, &h);
	// 	// const struct camera_buffer *head;
	// 	const struct camera_parms *parms = camera_get_parms(cam);

	// 	if (camera_dequeue_one(cam) < 0) {
	// 		perror("camera_dequeue_one");
	// 		return -1;
	// 	}

	// 	// head = camera_get_head(cam);

	// 	switch (parms->format) {
	// 	case CAMERA_FORMAT_MJPEG:
	// 		// mjpeg_decode_gray(mj, head->addr, head->size,
	// 		// 		  buf, w, w, h);
	// 		break;

	// 	case CAMERA_FORMAT_YUYV:
	// 		// yuyv_to_luma(sshead->addr, w * 2, w, h, buf, w);
	// 		break;

	// 	default:
	// 		fprintf(stderr, "Unknown frame format\n");
	// 		return -1;
	// 	}

	// 	if (camera_enqueue_all(cam) < 0) {
	// 		perror("camera_enqueue_all");
	// 		return -1;
	// 	}

	// 	quirc_end(q);

	// 	count = quirc_count(q);
	// 	for (i = 0; i < count; i++) {
	// 		struct quirc_code code;
	// 		struct quirc_data data;

	// 		quirc_extract(q, i, &code);
	// 		if (!quirc_decode(&code, &data))
	// 			print_data(&data, &dt, want_verbose);
	// 	}
	// }
// }

// static int run_scanner(void)
// {
// 	struct quirc *qr;
// 	struct camera cam;
// 	struct mjpeg_decoder mj;
// 	const struct camera_parms *parms;

// 	camera_init(&cam);
// 	if (camera_open(&cam, camera_path, video_width, video_height,
// 			25, 1) < 0) {
// 		perror("camera_open");
// 		goto fail_qr;
// 	}

// 	if (camera_map(&cam, 8) < 0) {
// 		perror("camera_map");
// 		goto fail_qr;
// 	}

// 	if (camera_on(&cam) < 0) {
// 		perror("camera_on");
// 		goto fail_qr;
// 	}

// 	if (camera_enqueue_all(&cam) < 0) {
// 		perror("camera_enqueue_all");
// 		goto fail_qr;
// 	}

// 	parms = camera_get_parms(&cam);

// 	qr = quirc_new();
// 	if (!qr) {
// 		perror("couldn't allocate QR decoder");
// 		goto fail_qr;
// 	}

// 	if (quirc_resize(qr, parms->width, parms->height) < 0) {
// 		perror("couldn't allocate QR buffer");
// 		goto fail_qr_resize;
// 	}

// 	mjpeg_init(&mj);
// 	if (main_loop(&cam, qr, &mj) < 0)
// 		goto fail_main_loop;
// 	mjpeg_free(&mj);

// 	quirc_destroy(qr);
// 	camera_destroy(&cam);

// 	return 0;

// fail_main_loop:
// 	mjpeg_free(&mj);
// fail_qr_resize:
// 	quirc_destroy(qr);
// fail_qr:
// 	camera_destroy(&cam);

// 	return -1;
// }

static void fat_text(SDL_Surface *screen, int x, int y, const char *text)
{
	int i, j;

	for (i = -1; i <= 1; i++)
		for (j = -1; j <= 1; j++)
			stringColor(screen, x + i, y + j, text, 0xffffffff);
	stringColor(screen, x, y, text, 0x008000ff);
}

static void fat_text_cent(SDL_Surface *screen, int x, int y, const char *text)
{
	x -= strlen(text) * 4;

	fat_text(screen, x, y, text);
}

static void draw_qr(SDL_Surface *screen, struct quirc *q, struct dthash *dt)
{
	int count = quirc_count(q);
	int i;

	for (i = 0; i < count; i++) {
		struct quirc_code code;
		struct quirc_data data;
		quirc_decode_error_t err;
		int j;
		int xc = 0;
		int yc = 0;
		char buf[128];

		quirc_extract(q, i, &code);

		for (j = 0; j < 4; j++) {
			struct quirc_point *a = &code.corners[j];
			struct quirc_point *b = &code.corners[(j + 1) % 4];

			xc += a->x;
			yc += a->y;
			lineColor(screen, a->x, a->y, b->x, b->y, 0x008000ff);
		}

		xc /= 4;
		yc /= 4;

		if (want_verbose) {
			snprintf(buf, sizeof(buf), "Code size: %d cells",
				 code.size);
			fat_text_cent(screen, xc, yc - 20, buf);
		}

		err = quirc_decode(&code, &data);
		if(err)
		{
			printf("decode failed: %s\n", quirc_strerror(err));
		}
		else
		{
			printf("data: %s\n", data.payload);
		}
		
		printf(
					 "Ver: %d, ECC: %c, Mask: %d, Type: %d\n\n",
					 data.version, "MLHQ"[data.ecc_level],
					 data.mask, data.data_type);

		if (err) 
		{
			if (want_verbose)
				fat_text_cent(screen, xc, yc,
						quirc_strerror(err));
		} else 
		{
			fat_text_cent(screen, xc, yc, (char *)data.payload);
			print_data(&data, dt, want_verbose);

			if (want_verbose) 
			{
				snprintf(buf, sizeof(buf),
					 "Ver: %d, ECC: %c, Mask: %d, Type: %d",
					 data.version, "MLHQ"[data.ecc_level],
					 data.mask, data.data_type);
				fat_text_cent(screen, xc, yc + 20, buf);
			}
		}
	}
}

static int qrimage_from_sd(unsigned char *addr, unsigned int w, unsigned int h)
{
	FILE * fp = fopen(qr_file_name, "r");

	if(fp == NULL){
		printf("couldn't open file!\n");
		return -1;
	}

	int count = fread((void *)addr, 1, w * h * 3 / 2, fp);
	printf("read size:%d --- %d \n",count,w * h * 3 / 2);
	fclose(fp);

	return 0;	
}

/**
 * YUV420P转RGB24
 * @param data
 * @param rgb
 * @param width
 * @param height
 */
void YUV420P_TO_RGB24(unsigned char *data, unsigned char *rgb, int width, int height,int flag) {
    int index = 0;
    unsigned char *ybase = data;
    unsigned char *ubase = &data[width * height];
    unsigned char *vbase = &data[width * height * 5 / 4];
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            //YYYYYYYYUUVV
            u_char Y = ybase[x + y * width];
            u_char U = ubase[y / 2 * width / 2 + (x / 2)];
            u_char V = vbase[y / 2 * width / 2 + (x / 2)];

			if(flag == 0)
			{
				U = 128;
				V = 128;
			}
			
            rgb[index++] = Y + 1.772 * (U - 128); //B
            rgb[index++] = Y - 0.34413 * (U - 128) - 0.71414 * (V - 128); //G
            rgb[index++] = Y + 1.402 * (V - 128); //R
			rgb[index++] = 0;
        }
    }
}

int warpeyes(unsigned char *src, unsigned char *dst,int width,int height)
{
    const float K1 = 0.555f;
    const float K2 = 0.0002f;
    float img_size_x = 1;
    float img_size_y = 0.5625;
    float center_x = img_size_x / 2;
    float center_y = img_size_y / 2;
    float scaler = 1.05;

	// printf("----------------width:%d,height:%d,width*height:%d\n", width, height ,width*height);

    for (int y = 0; y < height; y++) 
    {
        for (int x = 0; x < width; x++) 
        {
            float cur_x = (float)x / (float)width;
            float cur_y = (float)y / (float)width;

            float d = sqrt(pow(cur_x - center_x,2.0) + pow(cur_y - center_y,2.0)) * scaler;

            float x1 = (cur_x - center_x) * (1.0 - (K1 * pow(d, 1.0) + K2 * pow(d, 2.0)));
            float y1 = (cur_y - center_y) * (1.0 - (K1 * pow(d, 1.0) + K2 * pow(d, 2.0)));

            x1 = (x1 * scaler + center_x) * width;
            y1 = (y1 * scaler + center_y) * width;

			int index = (int)x1 + ((int)y1) * width;
            
            if(dst != NULL && src != NULL)
            {	
				if(g_contrast == 0)
				{
					dst[x + width * y] = src[index];
				}
				else
				{
					if(src[index] > g_contrast)
					{
						dst[x + width * y] = 255;
					}
					else
					{
						dst[x + width * y] = 0;
					}
				}
            }

            if((index < 0) || (index > width * height) || (x < 0) || (x > width) || (y < 0) || (y > height) || (x1 < 0) || (x1 > width) || (y1 < 0) || (y1 > height))
            {
                printf("x:%d y:%d index:%d\ncur_x:%f,cur_y:%f\nx1:%f,y1:%f d:%f\n\n",x,y,index,cur_x,cur_y,x1,y1,d);
            }
        }
    }

	return 0;
}

static void *qrcode_main()
{
	int frame_count = 0;
	SDL_Event ev;
	int width, height;
	time_t last_rate = 0;
	SDL_Surface *screen;
	struct dthash dt;
	char rate_text[64];
	int file_type = 1;

	dthash_init(&dt, printer_timeout);
	
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		perror("couldn't init SDL");
	}
	screen = SDL_SetVideoMode(video_width, video_height, 32,
				  SDL_SWSURFACE | SDL_DOUBLEBUF);
	if (!screen) {
		perror("couldn't init video mode");
	}
	struct quirc * qr;
	qr = quirc_new();
	if(!qr)
	{
		printf("faild to alloc memory\n");
		return NULL;
	}

	width = video_width;
	height = video_height;
	// struct quirc_code pqcode;
	// struct quirc_data pqdata;

	printf("W:H = %d,%d\n", width, height);	
	
	if (quirc_resize(qr, width, height) < 0) 
	{
		printf( "faild to alloc memory");
		return NULL;
	}

	if(strstr(qr_file_name,"yuv") != NULL )
	{
		file_type = 0;
	}

	char *tmp_d = (char *)malloc( width * height * 3 / 2);
	
	Demuxing demuxing;
	if(file_type == 1)
	{
		demuxing.Initialize(qr_file_name);
		demuxing.Start();
	}

	while(1)
	{
		time_t now = time(NULL);
		// int done = 0;
		uint8_t *buf = quirc_begin(qr, NULL, NULL);

		if(buf == NULL)
		{
			printf("----------------width*height:%d\n", width*height);
		}
		
		char *d;
		
		if(file_type == 0)
		{
			d = (char *)malloc( width * height * 3 / 2);
			qrimage_from_sd((uint8_t *)d, width, height);
		}
		else
		{
			int s;
			d = demuxing.ReadVideoFrameData(&s);
		}
		
		// FILE *file;
		// file = fopen("test.yuv", "w+");
		// fwrite( d, 1,  video_width*video_height * 3 / 2, file);
		// fclose(file);

		// printf("----------------width:%d,height:%d,width*height:%d\n", width, height ,width*height);
		struct timeval tBegin, tEnd;
		gettimeofday(&tBegin, NULL);
		if(g_warpeyes == 1)
		{
			warpeyes((uint8_t *)d,(uint8_t *)tmp_d,width,height);
		}

		if(file_type == 0)
		{
			free(d);
		}
		else
		{
			demuxing.RemoveVideoFrameData();
		}
		if(g_warpeyes == 1)
		{
			d = tmp_d;
		}

		memcpy(buf, d, width*height);

		SDL_LockSurface(screen);
		YUV420P_TO_RGB24((uint8_t *)d,(uint8_t *)screen->pixels,width, height,0);
		SDL_UnlockSurface(screen);
		
		quirc_end(qr);
		
		SDL_UnlockSurface(screen);

		draw_qr(screen, qr, &dt);
		gettimeofday(&tEnd, NULL);
		long deltaTime = 1000000L * (tEnd.tv_sec - tBegin.tv_sec ) + (tEnd.tv_usec - tBegin.tv_usec);
		printf("Time spent:%ldms\n",deltaTime);
		// if (want_frame_rate)
		// 	fat_text(screen, 5, 5, rate_text);
		SDL_Flip(screen);

		while (SDL_PollEvent(&ev) > 0) {
			if (ev.type == SDL_QUIT)
				return 0;

			if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == 'q')
				return 0;
		}

		if (now != last_rate) {
			snprintf(rate_text, sizeof(rate_text),
				 "Frame rate: %d fps", frame_count);
			frame_count = 0;
			last_rate = now;
		}

		frame_count++;

		// int num_codes = quirc_count(qr);
		// printf("code num: %d\n", num_codes);
		
		// // FILE *file;
		// // file = fopen("test.yuv", "w+");
		// // fwrite( dst, 1,  width*height, file);
		// // fclose(file);

		// if(num_codes == 0)
		// {
		// 	sleep(1);
		// 	continue;
		// }

		// for(int i = 0; i < num_codes; i++)
		// {
		// 	quirc_decode_error_t err;
			
		// 	quirc_extract(qr, i, &pqcode);
		// 	err = quirc_decode(&pqcode, &pqdata);
		// 	if(err)
		// 	{
		// 		printf("decode failed: %s\n", quirc_strerror(err));
		// 		printf("    Version: %d, ECC: %c, Mask: %d, Type: %d\n\n",
		// 		pqdata.version, "MLHQ"[pqdata.ecc_level],
		// 		pqdata.mask, pqdata.data_type);
		// 		sleep(1);
		// 	}
		// 	else
		// 	{
		// 		FILE *file;
		// 		file = fopen("test---1.yuv", "w+");
		// 		fwrite( buf, 1,  width*height, file);
		// 		fclose(file);
		// 		printf("data: %s\n", pqdata.payload);
		// 		printf("    Version: %d, ECC: %c, Mask: %d, Type: %d\n\n",
		// 		pqdata.version, "MLHQ"[pqdata.ecc_level],
		// 		pqdata.mask, pqdata.data_type);
		// 		done++;		
		// 	}
		// }

		// if(done != 0)
		// {
		// 	break;
		// }
	}

	quirc_destroy(qr);

	return NULL;
}

static void usage(const char *progname)
{
	printf("Usage: %s [options]\n\n"
"Valid options are:\n\n"
"    -v             Show extra data for detected codes.\n"
"    -d <device>    Specify camera device path.\n"
"    -s <WxH>       Specify video dimensions.\n"
"    -p <timeout>   Set printer timeout (seconds).\n"
"    --help         Show this information.\n"
"    --version      Show library version information.\n",
	progname);
}

int main(int argc, char **argv)
{
	static const struct option longopts[] = {
		{"help",		0, 0, 'H'},
		{"version",		0, 0, 'V'},
		{NULL,			0, 0, 0}
	};
	int opt;

	printf("quirc scanner demo\n");
	printf("Copyright (C) 2010-2012 Daniel Beer <dlbeer@gmail.com>\n");
	printf("\n");

	while ((opt = getopt_long(argc, argv, "d:s:vg:p:f:wc:",
				  longopts, NULL)) >= 0)
		switch (opt) {
		case 'V':
			printf("Library version: %s\n", quirc_version());
			return 0;

		case 'H':
			usage(argv[0]);
			return 0;
		case 'w':
			g_warpeyes = 1;
			break;
		case 'c':
			g_contrast = atoi(optarg);
			break;
		case 'v':
			want_verbose = 1;
			break;
		case 'f':
			qr_file_name = optarg;
			break;
		case 's':
			if (parse_size(optarg, &video_width, &video_height) < 0)
				return -1;
			break;

		case 'p':
			printer_timeout = atoi(optarg);
			break;

		case 'd':
			camera_path = optarg;
			break;

		case '?':
			fprintf(stderr, "Try --help for usage information\n");
			return -1;
		}

	// return run_scanner();
	qrcode_main();
	return 0;
}
