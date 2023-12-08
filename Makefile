CC=gcc
CFLAGS=-I.
DEPS = d4.h
OBJ = d4.o
LIBS= -pthread

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

d4: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
clean:
	rm -f d4 $(OBJ) $(OBJ:.o=.d)