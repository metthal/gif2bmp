CXX=g++
CXXFLAGS=-Wall -Wextra -std=c++14
LDFLAGS=

RM=rm -rf
CWD=$(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

LIB_NAME=libgif2bmp.so
LIB_CXXFLAGS=$(CXXFLAGS) -shared
LIB_LDFLAGS=$(LDFLAGS)
LIB_SRC_FILES= \
		   gif2bmp.cpp
LIB_OBJ_FILES=$(patsubst %.cpp, %.o, $(LIB_SRC_FILES))

APP_NAME=gif2bmp
APP_CXXFLAGS=$(CXXFLAGS)
APP_LDFLAGS=$(LDFLAGS) -L. -Wl,-rpath,$(CWD) -lgif2bmp
APP_SRC_FILES= \
			   main.cpp
APP_OBJ_FILES=$(patsubst %.cpp,%.o,$(APP_SRC_FILES))


all: lib app

lib: $(LIB_OBJ_FILES)
	$(CXX) $(LIB_CXXFLAGS) -o $(LIB_NAME) $^ $(LIB_LDFLAGS)

app: $(APP_OBJ_FILES)
	$(CXX) $(APP_CXXFLAGS) -o $(APP_NAME) $^ $(APP_LDFLAGS)

clean:
	$(RM) $(LIB_OBJ_FILES) $(APP_OBJ_FILES) $(LIB_NAME) $(APP_NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: all lib app clean
