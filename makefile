
CC=g++

FLAGS=-Wall -fPIC

BIN_DIR=bin
OBJ_DIR=obj
SRC_DIR=src
LIB_DIR=lib

INCLUDE=-Iinclude/ -I/usr/include

_SRC=UraeData.cpp Raytracer.cpp Classifier.cpp VectorMath.cpp Fading.cpp
_OBJ=UraeData.o Raytracer.o Classifier.o VectorMath.o Fading.o
LIB=

ifeq ($(DEBUGMODE),1)
	FLAGS+=-g -DDEBUG=1
	LIB=$(LIB_DIR)/liburaed.a
	OBJ_DIR=obj/debug
else
	FLAGS+=-O3
	LIB=$(LIB_DIR)/liburae.a
endif


SRC=$(patsubst %,$(SRC_DIR)/%,$(_SRC))
OBJ=$(patsubst %,$(OBJ_DIR)/%,$(_OBJ))

LIBRARY=$(LIB)

Library : $(SRC) $(LIB)

$(LIB) : $(OBJ)
	rm -f $@
	ar cq $@ $(OBJ)

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp 
	$(CC) $(FLAGS) -c $< -o $@ $(INCLUDE)


clean:
	rm -f $(OBJ)
	rm -f $(LIB)

