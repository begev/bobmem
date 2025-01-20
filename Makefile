CC = gcc
CFLAGS = -g
TARGET = bobmem
SRCS = main.c bobmem.c 
OBJS = $(SRCS:.c=.o)

all: $(TARGET) 

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS) 
