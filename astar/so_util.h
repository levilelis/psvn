/*
Copyright (C) 2011-2013 by the PSVN Research Group, University of Alberta
*/

#ifndef _SO_UTIL_H_
#define _SO_UTIL_H_

#include "psvn_game_so.h"

/* Tries to load game object from .so file.
   Program exits on failure.
   Returns pointer to game object on success. */
compiled_game_so_t* load_psvn_so_object(const char* filename);

#endif /* _SO_UTIL_H_ */
