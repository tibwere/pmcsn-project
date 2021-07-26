INCLUDE_DIR		= ./include
OBJ_DIR 		= ./obj
BIN_DIR 		= ./bin
SRC_DIR 		= ./src
CC			= gcc
CFLAGS			= -I$(INCLUDE_DIR) -Wall -Wextra
LIBS 			= -lm
DEPS 			= $(INCLUDE_DIR)/rngs.h $(INCLUDE_DIR)/rvgs.h $(INCLUDE_DIR)/rvms.h
BIN_OBJ 		= $(OBJ_DIR)/simul.o
DEPS_OBJ 		= $(OBJ_DIR)/rngs.o $(OBJ_DIR)/rvgs.o $(OBJ_DIR)/rvms.o
IMP_OBJ		= $(OBJ_DIR)/improved_simul.o
EST_OBJ		= $(OBJ_DIR)/estimate.o
UVS_OBJ		= $(OBJ_DIR)/uvs.o
BIN			= simul
IMP			= improved_simul
EST			= estimate
UVS			= uvs

# N.B.	$@ è la variabile contenente il nome del target da generare, 
#	$< è la prima dipendenza (in questo caso il file sorgente).
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS) create-directories
	$(CC) -c -o $@ $< $(CFLAGS)

# N.B: $^ è la lista di dipendenze
simulator: $(BIN_OBJ) $(DEPS_OBJ)
	$(CC) -o $(BIN_DIR)/$(BIN) $^ $(CFLAGS) $(LIBS)
	
improved_simulator: $(IMP_OBJ) $(DEPS_OBJ)
	$(CC) -o $(BIN_DIR)/$(IMP) $^ $(CFLAGS) $(LIBS)
	
estimate: $(EST_OBJ) $(DEPS_OBJ)
	$(CC) -o $(BIN_DIR)/$(EST) $^ $(CFLAGS) $(LIBS)

uvs: $(UVS_OBJ) $(DEPS_OBJ)
	$(CC) -o $(BIN_DIR)/$(UVS) $^ $(CFLAGS) $(LIBS)

all: simulator improved_simulator estimate uvs

.PHONY: create-directories clean clean-all

create-directories:
	mkdir -p $(OBJ_DIR)
	mkdir -p $(BIN_DIR)
	
clean:
	rm -f $(OBJ_DIR)/*.o
	
clean-all:
	rm -f $(OBJ_DIR)/*.o $(BIN_DIR)/*
