#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mtd/mtd-user.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>

#include "layout.h"

#define ENV_IOC_MAGIC    'e'
#define ENV_IOC_MAXNR    7
#define ENV_IOCGET        _IOR(ENV_IOC_MAGIC, 0, unsigned long)
#define ENV_IOCSET        _IOW(ENV_IOC_MAGIC, 1, unsigned long)
#define ENV_IOCUNSET      _IOW(ENV_IOC_MAGIC, 2, unsigned long)
#define ENV_IOCCLR        _IOW(ENV_IOC_MAGIC, 3, unsigned long)
#define ENV_IOCPRT        _IOR(ENV_IOC_MAGIC, 4, unsigned long)
#define ENV_IOCSAVE       _IOR(ENV_IOC_MAGIC, 5, unsigned long)
#define ENV_IOCLOCK       _IOR(ENV_IOC_MAGIC, 6, unsigned long)
#define ENV_IOCUNLOCK     _IOR(ENV_IOC_MAGIC, 7, unsigned long)

#define ENV_NAME_MAXLEN 64
typedef struct env_ioctl_args {
    char section_name[ENV_NAME_MAXLEN];
    char key[ENV_NAME_MAXLEN];
    char *buf;
    int maxlen;
    int overwrite;
} env_ioctl_args_t;

#ifdef CONFIG_ENV_MAXLEN
#define ENV_VALUE_MAXLEN    CONFIG_ENV_MAXLEN
#else
#define ENV_VALUE_MAXLEN    0x2000
#endif

static char env_value_buf[ENV_VALUE_MAXLEN];
static int env_value_off = 0;
static pthread_mutex_t env_value_mutex = PTHREAD_MUTEX_INITIALIZER;

static int dev_fd = -1;

static int env_open(void);
extern void usage(void);

#define CHECK_ENV_OPEN()    \
    do{ \
        if (dev_fd < 0) {    \
            env_open();    \
        }    \
    }while(0);

//FIXME:  The string pointed to by the return value of fw_env_get() is statically allocated, and can  be modified  by  a  subsequent  call.
char * fw_env_get(const char * section_name,const char * key)
{
    if (key == NULL) {
        return NULL;
    }

    CHECK_ENV_OPEN();

    pthread_mutex_lock(&env_value_mutex);

    if (env_value_off >= sizeof(env_value_buf)) {
        env_value_off = 0;
    }

    env_ioctl_args_t arg;
    memset(&arg, 0, sizeof(arg)); 
    strncpy(arg.section_name, section_name, sizeof(arg.section_name));
    strncpy(arg.key, key, sizeof(arg.key));
    arg.buf = env_value_buf + env_value_off;
    arg.maxlen = sizeof(env_value_buf) - env_value_off;

    int ret = ioctl(dev_fd, ENV_IOCGET, &arg);
    if (ret < 0 && errno != ENOBUFS) {
        pthread_mutex_unlock(&env_value_mutex);
        return NULL;
    }

    if (ret >= 0) {
        env_value_off += strlen(arg.buf) + 1;
        pthread_mutex_unlock(&env_value_mutex);
        return arg.buf;
    }

    // (errno == ENOBUFS):  space not enough
    arg.buf = env_value_buf;
    arg.maxlen = sizeof(env_value_buf);
    env_value_off = 0;

    ret = ioctl(dev_fd, ENV_IOCGET, &arg);
    if (ret != 0) {
        pthread_mutex_unlock(&env_value_mutex);
        return NULL;
    }

    env_value_off += strlen(arg.buf) + 1;
    pthread_mutex_unlock(&env_value_mutex);
    return arg.buf;
}

int fw_env_set(const char * section_name,const char *key, const char *value, int overwrite)
{
    if (key == NULL || value == NULL) {
        return -1;
    }

    CHECK_ENV_OPEN();

    env_ioctl_args_t arg;
    memset(&arg, 0, sizeof(arg)); 
    strncpy(arg.section_name, section_name, sizeof(arg.section_name));
    strncpy(arg.key, key, sizeof(arg.key));
    arg.buf = (char *)value;
    arg.maxlen = strlen(value)+1;
    arg.overwrite = overwrite;

    return ioctl(dev_fd, ENV_IOCSET, &arg);
}

int fw_env_unset(const char * section_name,const char *key)
{
    if (key == NULL) {
        return -1;
    }

    CHECK_ENV_OPEN();

    env_ioctl_args_t arg;
    memset(&arg, 0, sizeof(arg)); 
    strncpy(arg.section_name, section_name, sizeof(arg.section_name));
    strncpy(arg.key, key, sizeof(arg.key));

    return ioctl(dev_fd, ENV_IOCUNSET, &arg);
}

int fw_env_clear(const char * section_name)
{
    CHECK_ENV_OPEN();
    
    env_ioctl_args_t arg;
    memset(&arg, 0, sizeof(arg)); 
    strncpy(arg.section_name, section_name, sizeof(arg.section_name));

    return ioctl(dev_fd, ENV_IOCCLR, &arg);
}

void fw_env_print(const char * section_name)
{
    CHECK_ENV_OPEN();

    char * buf = malloc(ENV_VALUE_MAXLEN);
    if (!buf) {
        return;
    }
    memset(buf, 0, ENV_VALUE_MAXLEN);

    env_ioctl_args_t arg;
    memset(&arg, 0, sizeof(arg)); 
    if(section_name == NULL) {
        for(int i = 0; i < g_env_layout_num; i++) {
            strncpy(arg.section_name, g_env_layout[i].name, sizeof(arg.section_name));
            arg.buf = buf;
            arg.maxlen = ENV_VALUE_MAXLEN;

            int ret = ioctl(dev_fd, ENV_IOCPRT, &arg);
            if (ret == 0) {
                printf("-------%s-------\n",g_env_layout[i].name);
                printf("%s\n", arg.buf);
            }
        }
    }
    else {
        strncpy(arg.section_name, section_name, sizeof(arg.section_name));
        arg.buf = buf;
        arg.maxlen = ENV_VALUE_MAXLEN;

        int ret = ioctl(dev_fd, ENV_IOCPRT, &arg);
        if (ret == 0) {
            printf("%s\n", arg.buf);
        }
    }

    free(buf);
}

int fw_env_save(void)
{
    CHECK_ENV_OPEN();

    return ioctl(dev_fd, ENV_IOCSAVE, NULL);
}

int fw_env_lock(void)
{
    CHECK_ENV_OPEN();

    return ioctl(dev_fd, ENV_IOCLOCK, NULL);
}

int fw_env_unlock(void)
{
    CHECK_ENV_OPEN();

    return ioctl(dev_fd, ENV_IOCUNLOCK, NULL);
}

/* Parse LINE argument in "name=value" format, and set NAME and VALUE pointers.
   It removes preceding and following zeros by changing data in LINE.
   Empty line and # line will be skipped. */
int parse_env_line(char *line, char **name, char **value)
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
        fprintf(stderr, "Cannot find \'=\' in \"%s\"\n", line);
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

/* Read each line from FILENAME, and do fw_env_set() for each. */
static void setenv_from_file(char *filename,const char * section_name)
{
    char line[512];
    FILE *fp_txt = fopen(filename, "r");
    if (fp_txt == NULL) {
        fprintf(stderr, "Cannot open %s\n", filename);
        return;
    }
    while (fgets(line, sizeof(line), fp_txt)) {
        char *key, *value;
        if (parse_env_line(line, &key, &value) == 0) {
            if (strlen(value)) {
                fw_env_set(section_name,key, value, 1);
            } else {
                fw_env_unset(section_name,key);
            }
        }
    }
}

static int env_open(void)
{
    char * devname = "/dev/env";
    if (dev_fd < 0) {
        dev_fd = open(devname, O_RDWR);
        if (dev_fd < 0) {
            fprintf(stderr, "ERROR: Cannot open %s,err:%d\n", devname,dev_fd);
            exit(1);
        }
    }

    return 0;
}

/* FIXME: 
 *   The "fw_env" is very fundamental system facility, assuming that there
 *   is not a case needs to close it. 
 *   May need a change if this assumption proves to be false in the future.
 */
#if 0
int fw_env_close(void)
{
    if(dev_fd > 0){
        close(dev_fd);
        dev_fd = -1;
    }
}
#endif

void fw_printenv(int argc, char *argv[])
{
    if (argc == 1) {
        fw_env_print(NULL);
    }
    else if (argc == 2) {
        fw_env_print(argv[1]);
    }
    else if(argc == 3){
        const char *value = fw_env_get(argv[1],argv[2]);
        if (value) {
            puts(value);
        }
    } 
    else {
        usage();
    }
}

void fw_setenv(int argc, char *argv[])
{
    if (argc == 1) {
        fw_env_print(NULL);
    } else if (argc == 4) {
        if (strcmp(argv[1], "-s") == 0) {
            setenv_from_file(argv[2],argv[3]);
        } else {
            fw_env_set(argv[1], argv[2], argv[3], 1);
        }
    } else if(argc == 3) {
        if (strcmp(argv[1], "-c") == 0){
            fw_env_clear(argv[2]);
        }else{
            fw_env_unset(argv[1], argv[2]);
        }
    } else if(argc == 2) {
        if (strcmp(argv[1], "lock") == 0){
            fw_env_lock();
        } else if (strcmp(argv[1], "unlock") == 0){
            fw_env_unlock();
        }
    } else {
        usage();
        return;
    }
}

int fw_env_getint(const char * section_name,const char *key, int default_value)
{
    int value = default_value;
    const char *env = fw_env_get(section_name,key);
    if (env) {
        value = atoi(env);
    }
    return value;
}

char *fw_env_getstr(const char * section_name,const char *key, char *default_value)
{
    char *value = default_value;
    char *env = fw_env_get(section_name,key);
    if (env) {
        value = env;
    }
    return value;
}
