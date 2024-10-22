CC = gcc
SC = glslc
LIBS = -lpolygl -lglfw -lvulkan
INCL = -I./include/

SRCS = $(shell find . -name '*.c')
OUTS = $(SRCS:.c=.out)

VERT_SHADERS_SRC = $(shell find ./shaders -name '*.vert')
VERT_SHADERS = $(VERT_SHADERS_SRC:.vert=.vert.spv)
FRAG_SHADERS_SRC = $(shell find ./shaders -name '*.frag')
FRAG_SHADERS = $(FRAG_SHADERS_SRC:.frag=.frag.spv)

all: ${OUTS} ${VERT_SHADERS} ${FRAG_SHADERS}

%.vert.spv: %.vert
	${SC} $< -o $@

%.frag.spv: %.frag
	${SC} $< -o $@

%.out: %.c /usr/local/lib/libpolygl.so
	${CC} -o $@ $< ${INCL} ${LIBS}
