CC = clang

CFLAGS = -Wall -Wextra -std=c11
LDFLAGS = -lraylib -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

TARGET = tetris
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)