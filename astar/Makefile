CC = gcc
CXX = g++
#OPT = -g
#OPT = -g -Wall -O3
OPT = -Wall -O3

# Determine the OS we're running on
OSTYPE := $(shell uname)

#
# LINUX SECTION
#
ifneq (,$(findstring Linux,$(OSTYPE)))
DL = -ldl
DLFLAGS = -fpic -shared -Wl,--export-dynamic
endif

#
# MAC OSX SECTION
#
ifneq (,$(findstring Darwin,$(OSTYPE)))
DL = -ldl
DLFLAGS = -fPIC -bundle -undefined dynamic_lookup
endif

#
# CYGWIN SECTION
#
ifneq (,$(findstring cygwin,$(OSTYPE))$(findstring CYGWIN,$(OSTYPE)))
DL =
DLFLAGS = -shared -Wl,--export-dynamic
endif

psvn2c: psvn2c.cpp psvn2c.hpp psvn.cpp psvn.hpp
	$(CXX) $(OPT) psvn2c.cpp psvn.cpp -o $@

misc.o: misc.c misc.h 
	$(CC) $(OPT) -c $<

so_util.o: so_util.c so_util.h psvn_game_so.h
	$(CC) $(OPT) -c $< 

astar_data.o: astar_data.cpp astar_data.hpp hash_map.hpp priority_queue.hpp reservoir.hpp
	$(CXX) $(OPT) -c $<

%.c: %.psvn psvn2c psvn2c_common.c
	./psvn2c --name=$(*F) --backwards_moves --fwd_history_len=0 < $< > $@

%.so: %.c psvn_game_so.h
	$(CC) $(OPT) $(DLFLAGS) $< $(DL) -o $@
	
%.astar_so: %.c astar_so.cpp
	@echo "const compiled_game_so_t* game = &$(*F);" > $@.inc
	$(CXX) $(OPT) astar_so.cpp astar_data.o so_util.o misc.o -include $< -include $@.inc -o $@
	@rm $@.inc
