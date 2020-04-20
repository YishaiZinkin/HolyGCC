CXXFLAGS = -std=c++11 -Wall -fno-rtti

# Determine the plugin-dir and add it to the flags
PLUGINDIR=$(shell $(CXX) -print-file-name=plugin)
CXXFLAGS += -I$(PLUGINDIR)/include

# top level goal: build our plugin as a shared library
all: holy-gcc.so

holy-gcc.so: holy-gcc.o
	$(CXX) -shared -o $@ $<

holy-gcc.o : holy-gcc.cc debug-utils.h
	$(CXX) $(CXXFLAGS) -fPIC -c -o $@ $<

clean:
	rm -f holy-gcc.o holy-gcc.so test.out

test: holy-gcc.so test.c
	$(CC) -fplugin=./holy-gcc.so test.c -o test.out

.PHONY: all clean check
