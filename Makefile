CC 			= g++
EXE			= match_pair
CFLAGS		= -g -Wall 
CXXFLAGS	= $(CFLAGS)
LDFLAGS		= 
SOURCES		= main.cpp cpa.c match_pair.cpp mersenne.cpp
OBJS		= main.o cpa.o match_pair.o mersenne.o

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(EXE)

main.o: cpa.h match_pair.h

cpa.o: cpa.h

match_pair.o: match_pair.h cpa.h

mersenne.o: randomc.h

release: 
	rm $(OBJS)
	rm $(EXE)
	$(CC) -Wall -O3 $(SOURCES) -o $(EXE)

clean:
	rm -f $(OBJS) $(EXECUTABLE)
