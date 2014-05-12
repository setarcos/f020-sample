#PROJS=ad bus common da key led mem rec seg sin switch uart1 printf
ALLSUB=$(wildcard */Makefile)
PROJS=$(patsubst %/Makefile, %, $(ALLSUB))
FLASH=$(patsubst %/Makefile, %_prog, $(ALLSUB))

all: $(PROJS)

$(PROJS):
	make -C $@

$(FLASH):
	make -C $(patsubst %_prog, %, $@) program

clean:
	$(foreach i, $(PROJS),make -C $i clean;)

.PHONY:$(PROJS)
