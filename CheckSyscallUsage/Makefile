#-----Macros---------------------------------



LIB = 
INCLUDES = 

# set up compiler and options
CXX = g++
CXXFLAGS = -g  $(INCLUDES)

#-----Suffix Rules---------------------------
# set up C++ suffixes and relationship between .cc and .o files

.SUFFIXES: .cpp

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

.cpp :
	$(CXX) $(CXXFLAGS) $< -o $@ -luuid -lcrypto -lssl -lm -lpthread -lpcre $(LIB) 

#-----File Dependencies----------------------

SRC = CheckSyscallUsage.cpp 

OBJ = $(addsuffix .o, $(basename $(SRC)))

CheckSyscallUsage: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ) -luuid -lcrypto -lssl -lm -lpthread -lpcre  $(LIB) 

#-----Other stuff----------------------------
clean:
	\rm *.o CheckSyscallUsage 

