PROGRAM:=iawlb
CWEAVE:=cweave

all : $(PROGRAM).pdf

.PHONY : all clean

$(PROGRAM).pdf: ../src/$(PROGRAM).w
	$(CWEAVE) ++ $<
	pdftex $(PROGRAM).tex && pdftex $(PROGRAM).tex
	find . -type f ! \( -name Makefile -o -name $@ \) -delete

clean :
	find . -type f ! -name Makefile -delete


