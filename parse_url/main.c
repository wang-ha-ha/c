#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define LOG_ID URL
#include "log.h"

char* dup_str(const char*source,int len)
{
    if(source==NULL)
           return  NULL;

    char *dest = (char*)malloc(len+1);
    memcpy(dest,source,len);
    dest[len] = 0;
    return dest;
}

int parse_url(char *url,char** protocl,char** host,unsigned int *port,char** abs_path)
{
    const char *parseptr1;
    const char *parseptr2;
    int len;
    int i;
    int is_https = 0;
    if(url == NULL)
    {
        DEBUG_ERR("url is NULL!\r\n");
        return -1;
    }
    parseptr2 = url;
    parseptr1 = strchr(parseptr2, ':');
    if ( NULL == parseptr1 )
    {
        DEBUG_ERR("URL format error!\r\n");
        return 1;
    }
    len = parseptr1 - parseptr2;
    for ( i = 0; i < len; i++ )
    {
        if ( !isalpha(parseptr2[i]) )
        {
            DEBUG_ERR("URL format error!The first character is not a letter\r\n");
            return 2;
        }
    }

    //protocol
    if(strstr(url,"https") != NULL)
    {
        is_https = 1;
    }

    if(protocl != NULL)
    {
        *protocl =  dup_str(parseptr2,len);
    }

    parseptr1++;
    parseptr2 = parseptr1;
    for ( i = 0; i < 2; i++ )
    {
        if ( '/' != *parseptr2 )
        {
            DEBUG_ERR("URL format error!no // \r\n");
            return 3;
        }
        parseptr2++;
    }

    parseptr1 = strchr(parseptr2, ':');
    if ( NULL == parseptr1 )//判断有无端口号
    {
        parseptr1 = strchr(parseptr2, '/');
        if ( NULL == parseptr1 )
        {
            DEBUG_ERR("URL format error!\r\n");
            return 4;
        }
        len = parseptr1 - parseptr2;

        //host:
        if(host != NULL)
        {
           *host =  dup_str(parseptr2,len);
        }

        if(is_https == 1)
        {
            *port = 443;
        }
        else
        {
            *port = 80;
        }
    }
    else
    {
        len = parseptr1 - parseptr2;
        //host:
        if(host != NULL)
        {
            *host =  dup_str(parseptr2,len);
        }
        parseptr1++;
        parseptr2 = parseptr1;
        parseptr1 = strchr(parseptr2, '/');
        if ( NULL == parseptr1 )
        {
            DEBUG_ERR("URL format error! \r\n");
            return 5;
        }
        len = parseptr1 - parseptr2;
        
        //port
        char* str_port = NULL;
        str_port = dup_str(parseptr2,len);
        if(str_port != NULL)
        {
            
            *port = atoi(str_port);
            free(str_port);
        }
        else
        {
            DEBUG_ERR("malloc error! \r\n");
            return 6;
        }
    }


    parseptr1++;
    parseptr2 = parseptr1;
    while ( '\0' != *parseptr1 && '?' != *parseptr1  && '#' != *parseptr1 ) 
    {
        parseptr1++;
    }
    len = parseptr1 - parseptr2;
    
    //path:
    if(abs_path != NULL)
    {
        *abs_path =  dup_str(parseptr2,len);
    }
        
    parseptr2=parseptr1;
    if ( '?' == *parseptr1 )
    {
        parseptr2++;
        parseptr1 = parseptr2;
        while ( '\0' != *parseptr1 && '#' != *parseptr1 )
        {
            parseptr1++;
        }
        len = parseptr1 - parseptr2;
        printf("query: ");
        for(i=0;i<len;i++)
        {
            printf("%c",parseptr2[i]);//判断有无询问并解析
        }
        printf("\n");
    }
    parseptr2=parseptr1;
    if ( '#' == *parseptr1 )
    {
        parseptr2++;
        parseptr1 = parseptr2;
        while ( '\0' != *parseptr1 ) 
        {
            parseptr1++;
        }
        len = parseptr1 - parseptr2;
        printf("fragment: ");
        for(i=0;i<len;i++)
        {
            printf("%c",parseptr2[i]);
        }
        printf("\n");//判断有无片段并解析

    }

    return 0;
}



int main()
{
	char* url1 = "https://www.baidu.com/s?ie=utf8&oe=utf8&tn=98010089_dg&ch=11&wd=strstr";
	char* url2 = "http://blog.csdn.net/cuishumao/article/details/10284463";
	char* url3 = "https://blog.csdn.net:8080/cuishumao/article/details/10284463";
	char* url4 = "http://www.google.com:80/wiki/Search?search=train&go=Go#steammachine";

	char *p ;
    char *h ;
    char *path;

	int port;

	int ret = 0;
	
	printf("---------------------\r\n%s\r\n",url1);


	ret = parse_url(url1,&p,&h,&port,&path);
	if(ret != 0)
	{
		printf("parse url1 error:%d \r]n",ret);
	}
	else
	{
		printf( \
				"protocl:%s\r\n" \
				"host:%s\r\n"    \
				"port:%d\r\n"    \
				"path:%s\r\n",   \
				p,h,port,path    \
				);	
	}

	free(p);
	free(h);
	free(path);

	printf("---------------------\r\n%s\r\n",url2);

	ret = parse_url(url2,&p,&h,&port,&path);
	if(ret != 0)
	{
		printf("parse url2 error:%d \r]n",ret);
	}
	else
	{
		printf( \
				"protocl:%s\r\n" \
				"host:%s\r\n"    \
				"port:%d\r\n"    \
				"path:%s\r\n",   \
				p,h,port,path    \
				);	
	}

	free(p);
	free(h);
	free(path);

	printf("---------------------\r\n%s\r\n",url3);

	ret = parse_url(url3,&p,&h,&port,&path);
	if(ret != 0)
	{
		printf("parse url3 error:%d \r]n",ret);
	}
	else
	{
		printf( \
				"protocl:%s\r\n" \
				"host:%s\r\n"    \
				"port:%d\r\n"    \
				"path:%s\r\n",   \
				p,h,port,path    \
				);	
	}

	free(p);
	free(h);
	free(path);
	
	printf("---------------------\r\n%s\r\n",url4);

	ret = parse_url(url4,&p,&h,&port,&path);
	if(ret != 0)
	{
		printf("parse url4 error:%d \r]n",ret);
	}
	else
	{
		printf( \
				"protocl:%s\r\n" \
				"host:%s\r\n"    \
				"port:%d\r\n"    \
				"path:%s\r\n",   \
				p,h,port,path    \
				);	
	}

	free(p);
	free(h);
	free(path);


	return 0;
}
