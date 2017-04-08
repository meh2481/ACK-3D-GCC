SHELL=C:/Windows/System32/cmd.exe
objects = ACKDLG.o ACKEDIT.o ACKSUP.o ACKWIN.o EditorConfig.o WINPAL.o
resources = ACKEDIT.res
LIBPATH = -L./..
LIB = -lack -lgdi32 -luser32 -lkernel32 -lcomdlg32
HEADERPATH = -I./..
STATICGCC = -static-libgcc -static-libstdc++

all : AckEdit.exe
 
AckEdit.exe : $(objects) $(resources)
	g++ -Wall -O2 -o $@ $(objects) $(LIBPATH) $(LIB) $(STATICGCC) $(HEADERPATH) $(resources)
	
%.o: %.CPP
	g++ -O2 -g -ggdb -c -MMD -o $@ $< $(HEADERPATH)

%.res: %.RC
	windres $< -O coff -o $@

-include $(objects:.o=.d)

.PHONY : clean
clean :
	rm -rf AckEdit.exe *.o *.d *.res
