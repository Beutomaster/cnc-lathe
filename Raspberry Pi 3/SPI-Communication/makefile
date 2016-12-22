CC = gcc
OBJECTS = spi_cnc-lathe.o
LIBS = -lcurl
CFLAGS = -Wall -O2
BINDIR = $(DESTDIR)/usr/bin
NAME = spi_cnc-lathe

spi_cnc-lathe: $(OBJECTS)
        $(CC) -o $(NAME) $(OBJECTS) $(LIBS)

%.o: %.c
        $(CC) -c $(CFLAGS) $<

install:
        install --mode=755 $(NAME) $(BINDIR)/

clean:
        rm *.o $(NAME)

uninstall:
        rm $(BINDIR)/$(NAME)