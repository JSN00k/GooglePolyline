CC=gcc
CFLAGS = -std=c99 -Wall -O2 -g
SRCS = polylineFunctions.c PolylineTool.c
OBJ = $(SRCS:.c=.o)
EXECUTABLE=PolylineTool

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)
	cc -o $(EXECUTABLE) $(OBJ)

.c.o:
	cc $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *~ $(EXECUTABLE)
