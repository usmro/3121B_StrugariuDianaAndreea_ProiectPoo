# Makefile pentru Linux/Ubuntu
# Folosire: make       -> compileaza
#           make run   -> compileaza si ruleaza
#           make clean -> sterge binarele

CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET   = cinema
SRCDIR   = CinemaBookingSystem
SRC      = $(SRCDIR)/main.cpp

all: $(TARGET)

$(TARGET): $(SRC) $(SRCDIR)/CinemaModels.h
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)
	@echo "==> Build reusit! Ruleaza cu: ./$(TARGET)"

run: $(TARGET)
	@cd $(SRCDIR) && ../$(TARGET)

clean:
	rm -f $(TARGET) cinema_test
	@echo "==> Curatat."

.PHONY: all run clean
