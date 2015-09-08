CC=gcc
CFLAGS = -std=c99 -Wall -O2 -g
LDFLAGS=-lm
SRCS = polylineFunctions.c PolylineTool.c AppendableDataStore.c
OBJ = $(SRCS:.c=.o)
EXECUTABLE=PolylineTool

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)
	cc $(CFLAGS) -o $(EXECUTABLE) $(OBJ) $(LDFLAGS)

.c.o:
	cc $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *~ $(EXECUTABLE)
