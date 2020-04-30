/*
Copyright (C) 2011-2013 by the PSVN Research Group, University of Alberta
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <ctype.h>
#include "so_util.h"

static void get_state_space_name(const char* in, char* out)
{
    const char* str = strrchr(in, '/');
    if (*str == 0)
        str = in;
    str++;
    while (*str && (isalpha(*str) || isdigit(*str) || *str=='_')) {
        *out++ = *str++;
    }
    *out = 0;
}

compiled_game_so_t* load_psvn_so_object(const char* filename)
{
    char name[1024];
    compiled_game_so_t* game = NULL;
    void* so_handle = dlopen(filename, RTLD_LAZY);
    if( so_handle == NULL ) {
        fprintf(stderr, "could not open shared object %s: %s\n",
                filename, dlerror());
        exit(EXIT_FAILURE);
    }
    /* attempt to load game object by guessing its name from the filename. */
    get_state_space_name(filename, name);
    game = dlsym(so_handle, name);
    if (game == NULL) {
        /* no such named object found: try the default name. */
        game = dlsym(so_handle, "psvn_state_space");
        if(game == NULL) {
            fprintf( stderr, "could not read game object from %s\n", filename );
            exit(EXIT_FAILURE);
        }
    }
    return game;
}
                                        
