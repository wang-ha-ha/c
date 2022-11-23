#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ENV_MAXLEN 0x1000

extern unsigned short crc16_ccitt(const void *buf, int len);

static int save_file(const char *fileName, const char *buf,int len)
{
    FILE *fp = NULL;

    if ((fp = fopen(fileName, "w")) == NULL) {
        return -1;
    }

    int ret = fwrite(buf, 1, len, fp);

    fclose(fp);

    if(ret != len) {
        return -1;
    }
    else {
        return 0;
    }
}

static int get_file_buf(const char *fileName,char **buf)
{
    FILE *fp = NULL;

    if ((fp = fopen(fileName, "r")) == NULL) {
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    long f_size = ftell(fp);
    long len = f_size + 1;

    *buf = (char *) malloc(len);
    if(*buf == NULL) {
        printf("%s, %d:buf is NULL\n", __func__, __LINE__);
        return -1;
    }

    rewind(fp);

    int ret = fread(*buf, 1, f_size, fp);
    fclose(fp);
    if(ret != (len - 1)){
        printf("%s, %d:Incorrect read length \n", __func__, __LINE__);
        return -1;
    }

    return len;
}

static int parse_env_line(char *line, char **name, char **value)
{
    char *p = line, *q;

    /* Replace CR and LF to NULL */
    p = strchr(line, 0x0d);
    if (p) {
        *p = 0;
    }
    p = strchr(line, 0x0a);
    if (p) {
        *p = 0;
    }

    /* Eat preceding spaces to search variable name */
    p = line;
    while (*p != 0 && *p == ' ') {
        p++;
    }
    if (*p == 0 || *p == '#') {
        return -1;
    }
    *name = p;

    /* Search '=' for variable value.  Replace = with NULL for name. */
    p = strchr(p, '=');
    if (p == 0 || *p == 0) {
        printf("Cannot find \'=\' in \"%s\"\n", line);
        return -1;
    }
    *p = 0;
    q = p - 1;
    while (*q == ' ') {
        *q-- = 0;
    }

    p++;
    /* Set value.  Eat preceding and following spaces */
    while (*p != 0 && *p == ' ') {
        p++;
    }
    *value = p;
    q = p + strlen(p) - 1;
    while (*q == ' ') {
        *q-- = 0;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 3) 
    {
        printf("Usage: fw_genenv para.txt env.bin\n\n");
        return -1;
    }

    char *para_file_buf;
    char *env_buf;
    
    int para_len = get_file_buf(argv[1],&para_file_buf);
    
    if(para_len <= 0 || para_file_buf == NULL)
    {
        return -1;
    }

    para_file_buf[para_len - 1] = '\0';

    env_buf = (char *)malloc(para_len + 6);
    if(env_buf == NULL) {
        printf("%s, %d:buf is NULL\n", __func__, __LINE__);
        return -1;
    }

    env_buf[0] = 0xA5;
    env_buf[1] = 0x5A;

    char *tmp = para_file_buf;
    char *tmp1 = env_buf + 4;
    int index = 0;
    int env_len = 0;

    for(int i = 0;i < para_len;i++)
    {
        if(tmp[i] == '\n' || tmp[i] == '\r' || tmp[i] == '\0')
        {
            tmp[i] = '\0';
            char *name, *value;
            if (parse_env_line(tmp + index, &name, &value) == 0) {
                if (strlen(value)) {
                    int len = strlen(name) + strlen(value) + 2;
                    printf("---%s=%s---\n",name,value);
                    if(env_len + len + 1 > (ENV_MAXLEN - 6))
                    {
                        printf("env is too long\n");
                        return -1;
                    }
                    else
                    {
                        sprintf(tmp1,"%s=%s",name,value);
                        tmp1 += (len);
                        env_len += len;
                    }
                }
            }
            index = i + 1;
        }
    }

    env_buf[2] = env_len >> 8;
    env_buf[3] = env_len & 0xff;

    int crc16 = crc16_ccitt(env_buf,env_len + 4);

    env_buf[env_len + 4] = crc16 >> 8;
    env_buf[env_len + 4 + 1] = crc16 & 0xff;

    save_file(argv[2],env_buf,env_len + 6);

    free(env_buf);
    free(para_file_buf);

    return 0;
}