#ifndef __NCGIC_H__
#define __NCGIC_H__

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <memory.h>
#include <unistd.h>

struct postget_data{
  char *name;
  char *filename;
  char *data;
  int data_len;
};

int system_exec( char *path, char *arg);

struct postget_data * postgetdata_find(char *name);
struct postget_data * postgetdata_get(int index);
int postgetdata_get_max(void);
extern int cgimain(void);
#endif