CC = gcc
LIBS = -lglfw -lvulkan
INCL = -I./include/
ARGS = -fPIC

SRCS = $(shell find ./src -name '*.c')
OBJS = $(SRCS:.c=.o)
INCLS = $(shell find ./include -name '*.h')

all: libpolygl.so

libpolygl.so: ${OBJS}
	${CC} ${LIBS} --shared -o $@ $^ ${ARGS}

%.o: %.c ${INCLS}
	${CC} -c -o $@ $< ${INCL} ${ARGS}

clean: ${OBJS}
	rm $^

dlt: libpolygl.so clean
	rm $<

install: libpolygl.so ${INCLS}
	sudo cp $< /usr/local/lib/
	sudo cp include/* /usr/local/include/