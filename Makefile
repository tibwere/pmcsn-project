INCLUDE_DIR	= ./include
OBJ_DIR 	= ./obj
BIN_DIR 	= ./bin
SRC_DIR 	= ./src
CC		= gcc
CFLAGS		= -I$(INCLUDE_DIR) -Wall -Wextra
LIBS 		= -lm
DEPS 		= $(INCLUDE_DIR)/rngs.h $(INCLUDE_DIR)/rvgs.h
BIN_OBJ 	= $(OBJ_DIR)/poste_italiane.o 
DEPS_OBJ 	= $(OBJ_DIR)/rngs.o $(OBJ_DIR)/rvgs.o
BIN		= simul

# N.B.	$@ è la variabile contenente il nome del target da generare, 
#	$< è la prima dipendenza (in questo caso il file sorgente).
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS) create-directories
	$(CC) -c -o $@ $< $(CFLAGS)

# N.B: $^ è la lista di dipendenze
simulator: $(BIN_OBJ) $(DEPS_OBJ)
	$(CC) -o $(BIN_DIR)/$(BIN) $^ $(CFLAGS) $(LIBS)

.PHONY: create-directories clean clean-all

create-directories:
	mkdir -p $(OBJ_DIR)
	mkdir -p $(BIN_DIR)
	
clean:
	rm -f $(OBJ_DIR)/*.o
	
clean-all:
	rm -f $(OBJ_DIR)/*.o $(BIN_DIR)/*
