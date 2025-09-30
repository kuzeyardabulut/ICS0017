CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -I.
SRCS := main.cpp utils.cpp currency_manager.cpp
OBJDIR := build
OBJS := $(SRCS:%.cpp=$(OBJDIR)/%.o)
TARGET := $(OBJDIR)/exchange_store_cp1

all: $(TARGET)

$(TARGET): $(OBJS) | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR)

.PHONY: all clean
