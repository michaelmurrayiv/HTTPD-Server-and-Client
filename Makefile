CC := gcc
CFLAGS := -g
TARGETS := httpd

all: $(TARGETS)

httpd: httpd.c functions.c functions.h kvclient.c kvclient.h
	$(CC) $(CFLAGS) -o httpd httpd.c functions.c kvclient.c
clean:
	rm -f $(TARGETS)


