CXX=g++
CXXFLAGS= -Wall -g -std=c++11

ROOTd = ~/projects/memmgr-cpp-poc
BINd = $(ROOTd)/bin
OBJd = $(ROOTd)/obj
SRCd = $(ROOTd)/src
BIN_NAME = memmgr-poc

$(OBJd)/%.o : %.cc
	$(CXX) -c $< -o $(OBJd)/$@


.SILENT : $(BIN_NAME)
$(BIN_NAME): $(SRCd)/*.cc $(SRCd)/*.h
	$(CXX) $(CXXFLAGS) $? -o $(BINd)/$@
	chmod +x $(BINd)/$(BIN_NAME)
	echo "making '$(BIN_NAME)' is complete"


.SILENT : clean
.PHONY : clean
clean:
	rm -f $(OBJd)/*.o
	rm -f $(SRCd)/*~
	rm -f $(BINd)/*
