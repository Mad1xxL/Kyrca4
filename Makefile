SOURCES = $(wildcard src/*.cpp)
HEADERS = $(wildcard src/*.h)


CXXFLAGS = -std=c++17 -Wall -Wextra

ifeq ($(OS),Windows_NT)
    PLATFORM = Windows
else
    UNAME = $(shell uname -s)
    ifeq ($(UNAME),Darwin)
        PLATFORM = macOS
    else
        PLATFORM = Linux
    endif
endif

ifeq ($(PLATFORM),Windows)
    CXX = g++
    CXXFLAGS += -finput-charset=UTF-8 -fexec-charset=CP1251
    TARGET = student_activity.exe
    RUN = $(TARGET)
    REMOVE = del /Q
endif

ifeq ($(PLATFORM),macOS)
    CXX = clang++
    TARGET = student_activity
    RUN = ./$(TARGET)
    REMOVE = rm -f
endif

ifeq ($(PLATFORM),Linux)
    CXX = g++
    TARGET = student_activity
    RUN = ./$(TARGET)
    REMOVE = rm -f
endif

$(TARGET): $(SOURCES) $(HEADERS)
	@echo Build for $(PLATFORM): $(CXX)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

run: $(TARGET)
	$(RUN)

clean:
	$(REMOVE) $(TARGET)

.PHONY: run clean
