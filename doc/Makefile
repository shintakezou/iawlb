PROGRAM:=iawlb
CWEAVE:=cweave

all : $(PROGRAM).pdf

.PHONY : all clean

$(PROGRAM).pdf: ../src/$(PROGRAM).w
	$(CWEAVE) ++ $<
	xetex $(PROGRAM).tex && xetex $(PROGRAM).tex
	find . -type f ! \( -name Makefile -o -name $@ -o -name cwebmac.tex \) -delete

clean :
	find . -type f ! -name Makefile -delete


