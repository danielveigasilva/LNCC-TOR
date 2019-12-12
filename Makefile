LDFLAGS = -lssl -lcrypto -lm

.c:
	$(CC) -o $@.out $< $(LDFLAGS)
