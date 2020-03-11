#
# Disassembler makefile for Linux
#

OBJDIR = ./obj

CC = gcc
CFLAGS = -Wall -O2
LIBS =
D52OBJS = $(OBJDIR)/d52.o $(OBJDIR)/common.o $(OBJDIR)/d52pass1.o $(OBJDIR)/d52pass2.o $(OBJDIR)/d52table.o $(OBJDIR)/analyze52.o

all: d52

install: d52
	cp -f d52 /usr/local/bin

d52: $(D52OBJS)
	$(CC) $(CFLAGS) $(D52OBJS) -o d52 $(LIBS)
	strip d52

$(OBJDIR)/d52.o: d52.c defs.h d52.h dispass0.c d52pass1.h d52pass2.h dispass3.c d52table.h analyze.h common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/common.o: common.c defs.h d52.h common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/d52pass1.o:  d52pass1.c defs.h d52.h d52pass1.h d52pass2.h d52table.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/d52pass2.o:  d52pass2.c defs.h d52.h d52pass1.h d52pass2.h d52table.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/d52table.o: d52table.c defs.h d52.h d52table.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/analyze52.o: analyze52.c defs.h d52.h analyze.h d52pass2.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJDIR)/*.o
	rm -f d52

# end of file

