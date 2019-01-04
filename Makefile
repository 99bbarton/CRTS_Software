#Makefile to build CSC processing and analysis programs

COMP_FLAGS = -std=c++11 -O
ROOT_FLAGS = `root-config --cflags --glibs`
ROOT_INC = `root-config --incdir`
ROOT_LINK = `root-config  --libdir --libs`

cppFiles = EventData.cpp Position.cpp Track.cpp
baseClasses = EventData.cpp EventData.h Constants.h Position.h Position.cpp Track.h Track.cpp

default: ProcessBinary ReadTree

ProcessBinary: ProcessBinary.cpp ProcessBinary.h $(baseClasses)
	g++ $(COMP_FLAGS) -o ProcessBinary ProcessBinary.cpp $(cppFiles) $(ROOT_FLAGS) 

ReadTree: ReadTree.cpp ReadTree.h $(baseClasses)
	g++ $(COMP_FLAGS) -o ReadTree ReadTree.cpp $(cppFiles) $(ROOT_FLAGS)

clean:
	rm ParseBinary ReadTree
