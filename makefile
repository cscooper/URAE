
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
	LIBNAME=uraed
	LIB=$(LIB_DIR)/liburaed.a
	OBJ_DIR=obj/debug
	BIN_DIR=bin/debug
else
	FLAGS+=-O3
	LIBNAME=urae
	LIB=$(LIB_DIR)/liburae.a
endif


URAELIB_SRC=$(patsubst %,$(SRC_DIR)/UraeLib/%,$(_SRC))
URAELIB_OBJ=$(patsubst %,$(OBJ_DIR)/UraeLib/%,$(_OBJ))
URAELIB_SRC_DIR=$(SRC_DIR)/UraeLib
URAELIB_OBJ_DIR=$(OBJ_DIR)/UraeLib

RT_SRC=$(patsubst %,$(SRC_DIR)/Raytracer/%,Raytracer.cpp main.cpp)
RT_OBJ=$(patsubst %,$(OBJ_DIR)/Raytracer/%,Raytracer.o main.o)
RT_SRC_DIR=$(SRC_DIR)/Raytracer
RT_OBJ_DIR=$(OBJ_DIR)/Raytracer
RT_BIN=$(BIN_DIR)/Raytracer
RT_LIBS=-l$(LIBNAME) -lpthread

OMNETPP_SRC_DIR=$(SRC_DIR)/OMNeT++
OMNETPP_OBJ_DIR=$(OBJ_DIR)/OMNeT++
OMNETPP_SO=$(LIB_DIR)/liburae_omnetpp.so

LIBRARY=$(LIB)

Library : $(SRC) $(LIB)

$(LIB) : $(URAELIB_OBJ)
	rm -f $@
	ar cq $@ $(URAELIB_OBJ)

$(URAELIB_OBJ_DIR)/%.o : $(URAELIB_SRC_DIR)/%.cpp 
	$(CC) $(FLAGS) -c $< -o $@ $(INCLUDE)

Raytracer : $(RT_SRC) $(RT_BIN)

$(RT_BIN) : $(RT_OBJ)
	$(CC) $(RT_OBJ) -o $(RT_BIN) -L$(LIB_DIR) $(RT_LIBS)

$(RT_OBJ_DIR)/%.o : $(RT_SRC_DIR)/%.cpp
	$(CC) $(FLAGS) -c $< -o $@ $(INCLUDE)

OMNETPP : $(LIB)
	cd $(OMNETPP_SRC_DIR)
	opp_makemake

clean:
	rm -f $(RT_BIN)
	rm -f $(URAELIB_OBJ)
	rm -f $(RT_OBJ)
	rm -f $(LIB)

