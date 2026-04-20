CXX = g++
CXXFLAGS = -std=c++17 -pthread -Wall

threadpool: main.cpp
	$(CXX) $(CXXFLAGS) -o threadpool main.cpp

clean:
	rm -f threadpool