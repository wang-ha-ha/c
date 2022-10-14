#include "ncgic.h"

int cgimain(void)
{
    struct postget_data *data;
    char filename[256], *content;
    int content_len;
    FILE * fp;

    data = postgetdata_find("where");
    strcpy(filename, data->data);
    data = postgetdata_find("file");
    strcat(filename, data->filename);
    content = data->data;
    content_len = data->data_len;
    
    if(strlen(filename) != 0 && content_len != 0)
    {
        printf("write file %s<br>", filename);
        fp = fopen(filename, "w+");
        fwrite(content, content_len, 1, fp);
        fclose(fp);
    }

    return 0;
}
