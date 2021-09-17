EFILES= minershell

all: $(EFILES)

minershell: minershell.o

clean:
	rm -f *.o $(EFILES)
