CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS =

# Source files
SRCS = athoscat.c listen.c client.c

# Object files
OBJS = $(SRCS:.c=.o)

# Final executable name
TARGET = athoscat

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
