
CC=g++

FLAGS=-Wall -fPIC

BIN_DIR=bin
OBJ_DIR=obj
SRC_DIR=src
LIB_DIR=lib

INCLUDE=-Iinclude/ -I/usr/include

_SRC=UraeData.cpp Classifier.cpp VectorMath.cpp Fading.cpp
_OBJ=UraeData.o Classifier.o VectorMath.o Fading.o
LIB=

ifeq ($(DEBUGMODE),1)
	FLAGS+=-g -DDEBUG=1
	LIB=$(LIB_DIR)/liburaed.a
	OBJ_DIR=obj/debug
else
	FLAGS+=-O3
	LIB=$(LIB_DIR)/liburae.a
endif


URAELIB_SRC=$(patsubst %,$(SRC_DIR)/UraeLib/%,$(_SRC))
URAELIB_OBJ=$(patsubst %,$(OBJ_DIR)/UraeLib/%,$(_OBJ))

URAELIB_SRC_DIR=$(SRC_DIR)/UraeLib
URAELIB_OBJ_DIR=$(OBJ_DIR)/UraeLib

LIBRARY=$(LIB)

Library : $(SRC) $(LIB)

$(LIB) : $(URAELIB_OBJ)
	rm -f $@
	ar cq $@ $(URAELIB_OBJ)

$(URAELIB_OBJ_DIR)/%.o : $(URAELIB_SRC_DIR)/%.cpp 
	$(CC) $(FLAGS) -c $< -o $@ $(INCLUDE)


clean:
	rm -f $(URAELIB_OBJ)
	rm -f $(LIB)

