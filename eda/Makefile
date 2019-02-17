# to make a PDB starting from foo.psvn  type the command:     make pdb ss=foo
# if $(absname).txt exists it will be used as the input (abstraction commands) to build the abstraction,
#   otherwise the user will enter the abstraction commands interactively.
# if $(ss).test exists it will be used as the input (set of start states) with which to test $(ss).ida,
#   otherwise the user will enter the start states interactively.

CC = gcc
CXX = g++
#OPT = -g -Wall -O3
OPT = -Wall -O3
absname = absname

%.c: %.psvn
	../psvn2c --backwards_moves  --name=$(*F)  < $< > $@

.PRECIOUS: %.c

psvn.cpp:
	cp ../psvn.cpp ./psvn.cpp

psvn.hpp:
	cp ../psvn.hpp ./psvn.hpp

psvn2c_common.c:
	cp ../psvn2c_common.c ./psvn2c_common.c

psvn_game_so.h:
	cp ../psvn_game_so.h ./psvn_game_so.h

priority_queue.hpp:
	cp ../priority_queue.hpp ./priority_queue.hpp

%.distSummary: %.c distSummary.cpp priority_queue.hpp psvn_game_so.h psvn2c_common.c
	$(CXX) $(OPT) distSummary.cpp -include $< -o $@
	rm -f ./psvn_game_so.h ./psvn2c_common.c  ./priority_queue.hpp

%.eda: %.c eda.cpp psvn_game_so.h psvn2c_common.c
	$(CXX) $(OPT) eda.cpp -include $< -o $@
	rm -f ./psvn_game_so.h ./psvn2c_common.c

abstractor: abstractor.cpp psvn.cpp psvn.hpp
	$(CXX) $(OPT) abstractor.cpp psvn.cpp -o $@
	rm -f ./psvn.cpp ./psvn.hpp

.PRECIOUS: $(ss).ida
.PRECIOUS: $(absname).abst
.PRECIOUS: $(absname).state_map

.PHONY: pdb
testfile = $(ss).test
existstestfile = $(strip $(wildcard $(testfile)))
ifeq ($(existstestfile),$(testfile))
  infile = < $(testfile)
else
  infile =
endif
pdb: abstractor $(ss).ida
	rm -f $(absname).abst $(absname).c $(absname).distSummary $(absname).psvn $(absname).state_map
ifeq ($(strip $(wildcard $(absname).txt)),)
	./abstractor $(ss).psvn $(absname)
else
	./abstractor $(ss).psvn $(absname) < $(absname).txt
endif
	make $(absname).distSummary
	./$(absname).distSummary $(absname).state_map
	rm -f $(absname).c $(absname).distSummary $(absname).psvn
	./$(ss).ida $(absname) $(infile)

.PHONY: clean
clean:
	rm -f  *.o  psvn.cpp  psvn.hpp  psvn_game_so.h  psvn2c_common.c
