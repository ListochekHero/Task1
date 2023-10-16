CC=gcc
CFLAGS=-I. -lc-ares -lldns -lcjson
LIBS=-lcares -lldns -lcjson
DEPS = main.h
OBJ = main.o test1.o
REQOBJ = dns_request.o test1.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CC) -o $@ $^ $(LIBS)

req: $(REQOBJ)
	$(CC) -o $@ $^ $(LIBS)

.PHONY: clean

clean:
	rm *.o main req