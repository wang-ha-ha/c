
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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
                dst[x + width * y] = src[index];
            }

            if((index < 0) || (index > width * height) || (x < 0) || (x > width) || (y < 0) || (y > height) || (x1 < 0) || (x1 > width) || (y1 < 0) || (y1 > height))
            {
                printf("x:%d y:%d index:%d\ncur_x:%f,cur_y:%f\nx1:%f,y1:%f d:%f\n\n",x,y,index,cur_x,cur_y,x1,y1,d);
            }
        }
    }

    return 0;
}

//gcc main.c -lm

// static int g_w = 2624;
// static int g_h = 1472;

static int g_w = 1920;
static int g_h = 1080;

int main()
{
    unsigned char *s,*d;
    FILE *file;

    s = (unsigned char *)malloc(g_w * g_h * 3 / 2);
    d = (unsigned char *)malloc(g_w * g_h * 3 / 2);

    // file = fopen("test1.yuv", "r");
   	// fread((void *)s, 1, g_w * g_h * 3 / 2, file);
	// fclose(file);

    // warpeyes(s,d,g_w,g_h);
    warpeyes(NULL,NULL,g_w,g_h);

    // file = fopen("test1_out.yuv", "w+");
    // fwrite((void *)d, 1,  g_w * g_h * 3 / 2, file);
    // fclose(file);

    // free(s);
    // free(d);

    return 0;
}