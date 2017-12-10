PROGRAM:=iawlb
CTANGLE:=ctangle
SRCS:=$(PROGRAM).cpp SyncedList.hpp

all : $(SRCS)

.PHONY : all clean

$(SRCS) : $(PROGRAM).w
	$(CTANGLE) ++ $< - $(PROGRAM).cpp

clean :
	rm -f *.cpp *.hpp *~
