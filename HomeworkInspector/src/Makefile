#-----Macros---------------------------------



LIB = 
INCLUDES = 

# set up compiler and options
CXX = g++
CXXFLAGS = -w -g3 -O3  $(INCLUDES) -DMASSIF_USE_PAGES_AS_HEAP

#-----Suffix Rules---------------------------
# set up C++ suffixes and relationship between .cc and .o files

.SUFFIXES: .cpp

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<


#-----File Dependencies----------------------

SRC = HomeworkInspector.cpp CheckSyscallUsage.cpp config.cpp 

OBJ = $(addsuffix .o, $(basename $(SRC)))

HomeworkInspector: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ) -luuid -lcrypto -lssl -lm -lpthread -lpcre  $(LIB) 

test_target: test_target.cpp
	$(CXX) -g3 -O3  -o test_target test_target.cpp

all: test_target HomeworkInspector

#-----Other stuff----------------------------
clean:
	\rm *.o a.out HomeworkInspector test_target

