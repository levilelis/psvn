/*
Copyright (C) 2011-2013 by the PSVN Research Group, University of Alberta
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "misc.h"

char* print_bytes(size_t bytes)
{
    static char str[128];
    if (bytes < 1024)
        sprintf(str, "%lub", bytes);
    else if (bytes < 1024*1024)
        sprintf(str, "%lukb", bytes / 1024);
    else if (bytes < 1024*1024*1024)
        sprintf(str, "%lumb", bytes / (1024*1024));
    else
        sprintf(str, "%.2lfgb", (double)bytes / (1024*1024*1024));
    return str;
}
                                       
