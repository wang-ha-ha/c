#include "ncgic.h"

int cgimain(void)
{
    struct postget_data *data;
    char filename[256], *mcu_content, *content;
    int content_len;
    FILE * fp;

    strcpy(filename, "/userfs/");
    data = postgetdata_find("mcu");
    strcat(filename, "mcu.bin");

    content = data->data;
    content_len = data->data_len;

    if(strlen(filename) != 0 && content_len != 0)
    {
        printf("write file %s, len %d<br>", filename, content_len);
        fp = fopen(filename, "w+");
        fwrite(content, content_len, 1, fp);
        fclose(fp);
    }

    strcpy(filename, "/userfs/");
    data = postgetdata_find("mcuapp");
    strcat(filename, "mcu_app.bin");
    
    content = data->data;
    content_len = data->data_len;

    if(strlen(filename) != 0 && content_len != 0)
    {
        printf("write file %s, len %d<br>", filename, content_len);
        fp = fopen(filename, "w+");
        fwrite(content, content_len, 1, fp);
        fclose(fp);
    }

    printf("upgrade mcu, reboot...<br>", filename);
    system_exec("wifiCli mcu mcuctrl_mcu_upgrade", "");
//    system_exec("reboot", "");

    return 0;
}
