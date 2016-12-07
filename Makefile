CXX=clang++
CXXFLAGS=-g -std=c++11 -Wall -pedantic -lssl -lcrypto
BIN=imapcl

SRC=$(wildcard *.cpp)
OBJ=$(SRC:%.cpp=%.o)

all: $(OBJ)
	$(CXX) -o $(BIN) $(CXXFLAGS) $^

%.o: %.c
	$(CXX) $@ -c $<

clean:
	rm -f *.o
	rm $(BIN)