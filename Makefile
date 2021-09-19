modextract: modextract.c
	$(CC) -o modextract modextract.c

install: modextract
	install -m 755 modextract /usr/local/bin/
