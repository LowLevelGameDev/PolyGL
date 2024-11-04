CC = gcc
CCP = g++
SC = glslc
LIBS = -lpolygl -lglfw -lvulkan
INCL = -I./include/

SRCS = $(shell find . -name '*.c')
SRCSPP = $(shell find . -name '*.cpp')
OUTS = $(SRCS:.c=.out) $(SRCSPP:.cpp=.outpp)

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

%.outpp: %.cpp /usr/local/lib/libpolygl.so
	${CCP} -o $@ $< ${INCL} ${LIBS}
