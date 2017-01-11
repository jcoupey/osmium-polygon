# This file is part of osmium-polygon.

# Copyright (c) 2016-2017, Julien Coupey.
# All rights reserved (see LICENSE).

# Variables.
CC = g++
FLAGS = -std=c++14 -Wpedantic -Wall -Wextra -O3 -I../include/
LDLIBS = -lpthread -lz -lbz2 -lexpat

MAIN = ../bin/osmium-polygon
# Using all cpp files in current directory.
SRC = $(wildcard *.cpp)
OBJ = $(filter-out main.o,$(SRC:.cpp=.o))

all : $(MAIN)

$(MAIN) : $(OBJ) main.o
	$(CC) $(FLAGS) -o $@ $^ $(LDLIBS)

# Building .o files.
%.o : %.cpp %.h
	$(CC) $(FLAGS) -c $< -o $@

# Specific target for main (no .h file).
main.o : main.cpp
	$(CC) $(FLAGS) -c $< -o $@

# Tests
TEST = ../bin/osmium-polygon-tests

TEST_SRC = $(wildcard ./test/*.cpp)
TEST_OBJ = $(TEST_SRC:.cpp=.o)

test : $(TEST)

$(TEST) : $(TEST_OBJ) $(OBJ)
	$(CC) $(FLAGS) -o $@ $^ $(LDLIBS) -lboost_unit_test_framework

test/%.o : test/%.cpp
	$(CC) $(FLAGS) -c $< -o $@

clean :
	rm $(OBJ)
	rm $(TEST_OBJ)
	rm $(MAIN)
	rm $(TEST)
