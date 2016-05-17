PKULIB=pku.rel

$(PROG).ihx: $(PROG).c $(PKULIB) $(OTHERS)
	sdcc --nogcse $(DEFINES) -I../common $(PROG).c $(PKULIB) $(OTHERS)

$(PKULIB): ../common/pku.c ../common/pku.h
	sdcc --nogcse -c ../common/pku.c $(DEFINES)

program: $(PROG).ihx
	sudo /home/yangyj/bin/ec2writeflash --port USB --mode JTAG --hex $(PROG).ihx --eraseall --run

clean:
	rm -rf *.asm *.cdb *.ihx *.lk *.lst *.map *.mem *.omf *.rel *.rst *.sym

.PHONY:clean program

