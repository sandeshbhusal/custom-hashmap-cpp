CXX = g++
CXXFLAGS = -O3 -g -fno-pie -no-pie
LDFLAGS = -lbenchmark

TARGET = bench
SRC = bench.cc

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

run: $(TARGET)
	setarch `uname -m` -R ./$(TARGET)

clean:
	rm -f $(TARGET)
