#include "ncgic.h"

 /*
 * Duplicate string
 */

static char * FP_strdup (char *string)
{
  char *result;

  if (string == NULL)
    return NULL;

  if ((result = (char *) malloc (strlen (string) + 1)) == NULL) {
    fprintf (stderr, "proccgi -- out of memory dupping %d bytes\n",
	     (int) strlen (string));
    return NULL;
  }

  strcpy (result, string);
  return result;
}

/*
 * Parse + and %XX in CGI data
 */

static char * ParseString (char *instring)
{
  char *ptr1=instring, *ptr2=instring;

  if (instring == NULL)
    return instring;

  while (isspace (*ptr1))
    ptr1++;

  while (*ptr1) {
    if (*ptr1 == '+') {
      ptr1++; *ptr2++=' ';
    }
    else if (*ptr1 == '%' && isxdigit (*(ptr1+1)) && isxdigit (*(ptr1+2))) {
      ptr1++;
      *ptr2    = ((*ptr1>='0'&&*ptr1<='9')?(*ptr1-'0'):((char)toupper(*ptr1)-'A'+10)) << 4;
      ptr1++;
      *ptr2++ |= ((*ptr1>='0'&&*ptr1<='9')?(*ptr1-'0'):((char)toupper(*ptr1)-'A'+10));
      ptr1++;
    }
    else 
      *ptr2++ = *ptr1++;
  }
  while (ptr2>instring && isspace(*(ptr2-1)))
    ptr2--;

  *ptr2 = '\0';

  return instring;
}

/*
 * Read CGI input
 */

static int LoadInput (char** content)
{
  char *result = NULL, *method, *p;
  int length = 0, ts;

  *content = NULL;

  if ((method = getenv ("REQUEST_METHOD")) == NULL) {
    return 0;
  }

  if (strcmp (method, "GET") == 0) {
    if ((p = getenv ("QUERY_STRING")) == NULL)
      return 0;
    else
      result = FP_strdup (p);
      length = strlen(result);
  }
  else if (strcmp (method, "POST") == 0) {
    if ((length = atoi (getenv ("CONTENT_LENGTH"))) == 0)
      return 0;

    if ((result = malloc (length + 1)) == NULL) {
      fprintf (stderr, "proccgi -- out of memory allocating %d bytes\n",
	       length);
      return 0;
    }

    if ((ts = fread (result, sizeof (char), length, stdin)) < length) {
      fprintf (stderr, "proccgi -- error reading post data, %d bytes read, %d expedted\n",
	       ts, length);
      length = ts;
    }
    result[length] = '\0';
  }
  else {
    return 0;
  }

  *content = result;
  return length;
}


/*
 * break into attribute/value pair. Mustn't use strtok, which is
 * already used one level below. We assume that the attribute doesn't
 * have any special characters.
 */

static char *ParseGetData (char* data, struct postget_data * postgetdata)
{
    char *ptr1,*ptr2, *ptr3 ; 
    int len;
    memset(postgetdata, 0, sizeof(struct postget_data));
    ptr3 = NULL;
    ptr1 = data;
    if( (ptr2 = strstr(ptr1, "&")) != NULL || strlen(ptr1) != 0 )
    {
        if(ptr2)
        {
          *ptr2 = '\0';
          ptr3 = ptr2 + 1;
        }

        ptr2 = strstr(ptr1, "=");
        if(ptr2)
        {
            *ptr2++ = '\0';
            postgetdata->name = ParseString(ptr1);
            postgetdata->data = ParseString(ptr2);
            postgetdata->data_len = strlen(postgetdata->data);
        }else{
            postgetdata->name = ParseString(ptr1);
        }
    }

    return ptr3;
}


static char * ParsePostData(char* data, int *length, struct postget_data * postgetdata)
{
    char *content_type;
    char *boundary;
    char *ptr1,*ptr2, *ptr3 ; 
    int len = *length;
    memset(postgetdata, 0, sizeof(struct postget_data));
    content_type = getenv ("CONTENT_TYPE");
    ptr1 = strstr(content_type, "boundary=");
    ptr1 += strlen("boundary=");
    boundary = FP_strdup(ptr1);
    ptr1 = data;
    while( (ptr2 = strstr(ptr1, "\r\n")) != NULL )
    {
        *ptr2 = '\0';
        if(strlen(ptr1)==0)
        {
            ptr1+=2;
            ptr2 = (char*)memmem(ptr1, len - (int)(ptr1- data) ,  boundary, strlen(boundary));
            ptr2 -= 4;
            *ptr2 = 0;
            postgetdata->data = ptr1;
            postgetdata->data_len = (int)(ptr2 - ptr1);
            ptr2 += 2;
            *length = len - (int)(ptr2- data);
            break;
        }else{
            *ptr2 = 0;
            if(ptr3 = strstr(ptr1, "name=\""))
            {
              ptr3 = ptr3 + strlen("name=\"");
              postgetdata->name = ptr3;
            }
            if( ptr3 = strstr(ptr1, "filename=\""))
            {
              ptr3 = ptr3 + strlen("filename=\"");
              postgetdata->filename = ptr3;
            }

            if(postgetdata->name)
            {
                ptr3 = strstr(postgetdata->name, "\"");
                if(ptr3)
                  *ptr3 = '\0';
            }
            if(postgetdata->filename)
            {
                ptr3 = strstr(postgetdata->filename, "\"");
                if(ptr3)
                  *ptr3 = '\0';
            }

            ptr1 = ptr2+2;
        }
    }

    return ptr2;
}

int system_exec( char *path, char *arg)
{
    int ret;
    char cmdbuf[128];
    
    strncpy(cmdbuf, path, sizeof(cmdbuf)-1);
    strncat(cmdbuf, " ", sizeof(cmdbuf)-1);
    if(arg)
      strncat(cmdbuf, arg, sizeof(cmdbuf)-1);
    printf("<pre>");
    fflush(stdout);
    ret = system(cmdbuf);
    fflush(stdout);
    printf("</pre>");
    return ret;
}

static void print_html_head(char* title)
{
  printf("Content-type: text/html\r\n\r\n<title>%s</title>", title?title:"");  
}

static struct postget_data *postgetdata = NULL;
static int postgetdata_index = 0;
static int postgetdata_max_num = 10;

static struct postget_data * postgetdata_init(void)
{
    if(postgetdata)
      free(postgetdata);
    postgetdata = calloc(sizeof(struct postget_data), postgetdata_max_num);
    postgetdata_index = 0;
    return postgetdata;
}

static void postgetdata_free(void)
{
    if(postgetdata)
      free(postgetdata);
    return;
}

static struct postget_data * postgetdata_new(void)
{
    postgetdata_index++;
    if(postgetdata_index >= postgetdata_max_num)
    {
      postgetdata_max_num += 1;
      postgetdata = realloc(postgetdata, sizeof(struct postget_data)*postgetdata_max_num);
    }    
    return &postgetdata[postgetdata_index];
}

struct postget_data * postgetdata_find(char *name)
{
    int i;
    for(i=0; i< postgetdata_index; i++)
    {
        if(!strcmp(postgetdata[i].name,name))
        {
            return &postgetdata[i];
        }
    }
    return NULL;
}

struct postget_data * postgetdata_get(int index)
{
    if(index < postgetdata_index && index >= 0)
      return &postgetdata[index];
    return NULL;
}

int postgetdata_get_max(void)
{
    return postgetdata_index;
}

extern int cgimain(void);
int main(void)
{
    char *ptr1,*ptr2, *data = NULL ; 
    char *method;
    int len, length;
    struct postget_data *postgetdata;
    int max_num = 1;
    int index = 0;
    print_html_head(NULL);

    postgetdata = postgetdata_init();

    len = LoadInput(&data);
    length = atoi (getenv ("CONTENT_LENGTH"));
    method = getenv ("REQUEST_METHOD");
    if(!strcmp(method, "GET") )
    {
      ptr1 = data;
      do{
          ptr1 = ParseGetData(ptr1, postgetdata);
          if(postgetdata->name)
          {
              postgetdata = postgetdata_new();
          }
      }while(ptr1);
    }else if(!strcmp(method, "POST"))
    {
      if(len != length)
      {
        printf("<h3>upload length error</h3>");
        return 0;
      }
      ptr1 = data;
      do{
        ptr1 = ParsePostData(ptr1, &len, postgetdata);
        if(postgetdata->name)
        {
            postgetdata = postgetdata_new();
        }
      }while(ptr1);
    }

    cgimain();

    postgetdata_free();

    if(data)
        free (data);
    return 0;
}
