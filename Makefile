CC := gcc
MANDATORY_FLAGS := -Wall -Werror
OPTIONAL_FLAGS := -pedantic
LINKS := -lm -lSDL2 -lSDL2_ttf
PROGRAM_NAME := crclone
SRC_DIR := src
INC_DIR := include
OBJ_DIR := bin-int
BIN_DIR := bin

all: $(PROGRAM_NAME)

.PHONY: run

run:
	./$(BIN_DIR)/$(PROGRAM_NAME)

debug: OPTIONAL_FLAGS += -D_CRCLONE_DEBUG
debug: $(PROGRAM_NAME)

$(PROGRAM_NAME): $(addprefix $(OBJ_DIR)/,main.o app.o tile.o textures.o)
	$(CC) $^ -o $(BIN_DIR)/$@ $(LINKS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c --std=gnu11 $< -I$(INC_DIR) -o $@ $(MANDATORY_FLAGS) $(OPTIONAL_FLAGS)
