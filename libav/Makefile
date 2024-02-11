CC = gcc
cflags = -Ofast -Wall
ldflags = -lm -lavformat -lavcodec -lavutil -lswscale

simple_prores_writer : simple_prores_writer.c
	$(CC) $(cflags) simple_prores_writer.c -o simple_prores_writer $(ldflags)

clean :
	rm -f simple_prores_writer
