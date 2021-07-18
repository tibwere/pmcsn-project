INCLUDE_DIR		= ./include
OBJ_DIR 		= ./obj
BIN_DIR 		= ./bin
SRC_DIR 		= ./src
CC			= gcc
CFLAGS			= -I$(INCLUDE_DIR) -Wall -Wextra
VAL_FLAG		= -DVALIDATE
VER_FLAG		= -DVERIFY
LIBS 			= -lm
DEPS 			= $(INCLUDE_DIR)/rngs.h $(INCLUDE_DIR)/rvgs.h
BIN_OBJ 		= $(OBJ_DIR)/simul.o
DEPS_OBJ 		= $(OBJ_DIR)/rngs.o $(OBJ_DIR)/rvgs.o
UVS_OBJ		= $(OBJ_DIR)/uvs.o
VAL_BIN_OBJ		= $(OBJ_DIR)/simul-val.o 
VAL_DEPS_OBJ		= $(OBJ_DIR)/rngs-val.o $(OBJ_DIR)/rvgs-val.o
VER_BIN_OBJ		= $(OBJ_DIR)/simul-ver.o 
VER_DEPS_OBJ		= $(OBJ_DIR)/rngs-ver.o $(OBJ_DIR)/rvgs-ver.o
BIN			= simul
VAL			= simul-val
VER			= simul-ver
UVS			= uvs

# N.B.	$@ è la variabile contenente il nome del target da generare, 
#	$< è la prima dipendenza (in questo caso il file sorgente).
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS) create-directories
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ_DIR)/%-val.o: $(SRC_DIR)/%.c $(DEPS) create-directories
	$(CC) -c -o $@ $< $(CFLAGS) $(VAL_FLAG)

$(OBJ_DIR)/%-ver.o: $(SRC_DIR)/%.c $(DEPS) create-directories
	$(CC) -c -o $@ $< $(CFLAGS) $(VER_FLAG)

# N.B: $^ è la lista di dipendenze
simulator: $(BIN_OBJ) $(DEPS_OBJ)
	$(CC) -o $(BIN_DIR)/$(BIN) $^ $(CFLAGS) $(LIBS)

validate: $(VAL_BIN_OBJ) $(VAL_DEPS_OBJ)
	$(CC) -o $(BIN_DIR)/$(VAL) $^ $(CFLAGS) $(LIBS) $(VAL_FLAGS)

verify: $(VER_BIN_OBJ) $(VER_DEPS_OBJ)
	$(CC) -o $(BIN_DIR)/$(VER) $^ $(CFLAGS) $(LIBS) $(VER_FLAGS)
	
uvs: $(UVS_OBJ) $(DEPS_OBJ)
	$(CC) -o $(BIN_DIR)/$(UVS) $^ $(CFLAGS) $(LIBS)

all: simulator validate verify uvs

.PHONY: create-directories clean clean-all

create-directories:
	mkdir -p $(OBJ_DIR)
	mkdir -p $(BIN_DIR)
	
clean:
	rm -f $(OBJ_DIR)/*.o
	
clean-all:
	rm -f $(OBJ_DIR)/*.o $(BIN_DIR)/*
