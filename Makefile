CFLAGS = -O2 -Wall -Wextra -Wpedantic
LDFLAGS = -lssl -lcrypto -lm

.c:
	$(CC) -o $@.out $(CFLAGS) $< $(LDFLAGS) && ./$@.out
