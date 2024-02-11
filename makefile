CC = g++
CFLAGS = -Wall #-g -fsanitize=address -fsanitize=undefined
LDFLAGS = -lSDL2
OBJ_DIR = obj
SRC_DIR = src

NAME = main

OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(wildcard $(SRC_DIR)/*.cpp))

$(NAME): $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	./$(NAME)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)
	
.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/*.o $(NAME)
