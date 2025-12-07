# COPT_HOME = /opt/copt72
GUROBI_HOME = /home/zhuocfan/LLM/gurobi1203/linux64

CXX       = g++
CXXFLAGS  = -std=c++11 -O2 -DNDEBUG
# CXXFLAGS  = -std=c++11 -g -DNDEBUG
INCLUDES  = -I"$(GUROBI_HOME)/include" -I"$(GUROBI_HOME)/include"
LIBS      = -L"$(GUROBI_HOME)/lib" -lgurobi_c++ -lgurobi120

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

TARGET = main

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f *.o $(TARGET)