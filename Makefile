SHELL=C:/Windows/System32/cmd.exe
objects = ACKBBM.o ACKBKGD.o ACKDATA.o ACKDISP.o ACKDOOR.o ACKFLOOR.o ACKGIF.o ACKINIT.o ACKLDBMP.o ACKOBJ.o ACKOVER.o ACKPCX.o ACKPOV.o ACKRAY.o ACKRTNRecode.o ACKUTIL.o ACKVIEW.o ACKWRAP.o
STATICGCC = -static-libgcc -static-libstdc++

all : liback.a
 
liback.a : $(objects)
	ar rcs $@ $(objects)
	
%.o: %.C
	gcc -O2 -g -ggdb -c -MMD -o $@ $< $(HEADERPATH)

-include $(objects:.o=.d)

.PHONY : clean
clean :
	rm -rf liback.a *.o *.d
