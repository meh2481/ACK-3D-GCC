SHELL=C:/Windows/System32/cmd.exe
objects = ackse.o AckWrapper.o
LIBPATH = -L./.. -L./../../SDL-1.2.15/build/.libs
LIB = -lack -lSDL -lSDLmain
HEADERPATH = -I./include -I./../../SDL-1.2.15/include
STATICGCC = -static-libgcc -static-libstdc++

all : StationEscape.exe
 
StationEscape.exe : $(objects)
	g++ -Wall -O2 -o $@ $(objects) $(LIBPATH) $(LIB) $(STATICGCC) $(HEADERPATH)
	
%.o: %.cpp
	g++ -O2 -g -ggdb -c -MMD -o $@ $< $(HEADERPATH)

-include $(objects:.o=.d)

.PHONY : clean
clean :
	rm -rf StationEscape.exe *.o *.d
