CC=gcc
CFLAGS=-I. -lc-ares -lldns -lcjson
LIBS=-lcares -lldns -lcjson
DEPS = main.h dns_server_config_data_struct.h
OBJ = main.o dns_server_config.o handle_req.o
REQOBJ = test1.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CC) -o $@ $^ $(LIBS)

req: $(REQOBJ)
	$(CC) -o $@ $^ $(LIBS)

.PHONY: clean

clean:
	rm *.o main req