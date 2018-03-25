TARGET = cs300_test
OBJS = cs300_test.o
CFLAGS = -std=c99 -D _GNU_SOURCE -static
CC_C = gcc

all: clean $(TARGET) transfer

%.o : %.c
	$(CC) -c $(CFLAGS) $<

$(TARGET): $(OBJS)
	$(CC_C) $(CFLAGS) $(OBJS) -o $@


clean:
	rm -f $(TARGET)
	rm -f $(OBJS)

transfer:
	scp -P 2222 cs300_test root@localhost:~