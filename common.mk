PKULIB=../common/pku.rel

$(PROG).ihx: $(PROG).c $(PKULIB)
	sdcc --nogcse -I../common $(PROG).c $(PKULIB)

$(PKULIB): ../common/pku.c ../common/pku.h
	( cd ../common && make ) 

program: $(PROG).ihx
	sudo /home/yangyj/bin/ec2writeflash --port USB --mode JTAG --hex $(PROG).ihx --eraseall --run

clean:
	rm -rf *.asm *.cdb *.ihx *.lk *.lst *.map *.mem *.omf *.rel *.rst *.sym

.PHONY:clean program

