PROGRAM:=iawlb

all : src doc program


.PHONY: all src doc program clean

src :
	$(MAKE) -s -C src

doc :
	$(MAKE) -s -C doc


program : src
	test -d bin || mkdir bin
	g++ -std=c++11 -pthread -Isrc src/$(PROGRAM).cpp -o bin/$(PROGRAM)


clean :
	$(MAKE) -s -C src clean
	$(MAKE) -s -C doc clean
	rm -rf bin
	rm -f *~

