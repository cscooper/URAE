
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
	OMNETPP_MODE=debug
	OMNETPP_SO=urae_omnetpp_debug
	OMNETPP_OUTPUT_DIR=/gcc-debug
else
	FLAGS+=-O3
	LIBNAME=urae
	LIB=$(LIB_DIR)/liburae.a
	OMNETPP_MODE=release
	OMNETPP_SO=urae_omnetpp
	OMNETPP_OUTPUT_DIR=/gcc-release
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
VEINS_DIRS=base/utils modules modules/obstacle modules/utility modules/mobility/traci base/connectionManager base/modules modules/mac/ieee80211p modules/mobility modules/messages modules/analogueModel base/phyLayer modules/phy base/messages

LIBRARY=$(LIB)

.PHONY: check_veins create_dirs

all : create_dirs Library Raytracer OMNETPP

create_dirs :
	mkdir -p $(OBJ_DIR)/UraeLib
	mkdir -p $(OBJ_DIR)/Raytracer
	mkdir -p $(OMNETPP_OBJ_DIR)

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

OMNETPP : check_veins $(LIB)
	cd $(OMNETPP_SRC_DIR); opp_makemake -f -O ../../$(OMNETPP_OBJ_DIR) $(patsubst %,-I ../../$(VEINS_ROOT)/src/%,$(VEINS_DIRS)) -I ../../include -s -o $(OMNETPP_SO); make MODE=$(OMNETPP_MODE)
	cp $(OMNETPP_OBJ_DIR)$(OMNETPP_OUTPUT_DIR)/lib$(OMNETPP_SO).so $(LIB_DIR)


check_veins:
	@if [ -z "$(VEINS_ROOT)" ]; then \
		echo "Error: Please specify a root directory for Veins if you're building for OMNeT++"; exit 2; \
	else true; fi

clean:
	cd $(OMNETPP_SRC_DIR); make clean; rm -f Makefile lib*.so
	rm -rf $(BIN_DIR)/*
	rm -rf $(OBJ_DIR)/*
	rm -rf $(LIB_DIR)/*
