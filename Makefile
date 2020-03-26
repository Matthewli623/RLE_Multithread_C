CC:=gcc
CFLAGS:= -Wall -Werror -pthread -O
TARGETS:=pzip

all: $(TARGETS)

test:
	tests/bin/test-pzip.csh

clean:
	rm -f $(TARGETS)

