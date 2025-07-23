CC = clang
CFLAGS = -Iinclude -Wall -Wextra -std=c17 `pkg-config --cflags gtk+-3.0 webkit2gtk-4.0`
LIBS = `pkg-config --libs gtk+-3.0 webkit2gtk-4.0`
TARGET = mocha-settings
SRC = src/main.c src/config.c third_party/cJSON.c framework/src/framework.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ)