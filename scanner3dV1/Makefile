CXX = clang++
CXXFLAGS = -std=c++17 `pkg-config --cflags opencv4` -I /opt/homebrew/include/eigen3 -Wall -Wextra -O3
LDFLAGS = `pkg-config --libs opencv4`
TARGET = main
SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
