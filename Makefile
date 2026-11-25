# Makefile — сборка программы на macOS, Linux и Windows (MinGW).
#
#   make        — собрать программу
#   make run    — собрать и запустить
#   make clean  — удалить собранную программу

# Все исходные файлы и заголовки проекта.
SOURCES = $(wildcard src/*.cpp)
HEADERS = $(wildcard src/*.h)

# Общие флаги: стандарт C++17 и все основные предупреждения.
CXXFLAGS = -std=c++17 -Wall -Wextra

# Определяем операционную систему.
# В Windows переменная окружения OS всегда равна Windows_NT.
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
    # MinGW: исходники в UTF-8, а консоль Windows работает в кодировке 1251,
    # поэтому строки программы переводятся в 1251 при компиляции.
    CXX = g++
    CXXFLAGS += -finput-charset=UTF-8 -fexec-charset=CP1251
    TARGET = student_activity.exe
    RUN = $(TARGET)
    REMOVE = del /Q
endif

ifeq ($(PLATFORM),macOS)
    # Компилятор из Xcode Command Line Tools.
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

# Цель по умолчанию — собрать программу.
# Программа пересобирается, если изменился любой .cpp или .h файл.
$(TARGET): $(SOURCES) $(HEADERS)
	@echo Build for $(PLATFORM): $(CXX)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

run: $(TARGET)
	$(RUN)

clean:
	$(REMOVE) $(TARGET)

# Эти цели — команды, а не имена файлов.
.PHONY: run clean
