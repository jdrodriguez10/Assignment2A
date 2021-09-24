EFILES= minershell

all: $(EFILES)

testshell: minershell.o

clean:
	rm -f *.o $(EFILES)
