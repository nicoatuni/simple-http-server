# COMP30023 Sem 1 2018 Assignment 1
# Nico Eka Dinata < n.dinata@student.unimelb.edu.au >
# @ndinata

CC	= gcc

# CHANGE NEW TO SERVER
SRC	= server.c
OBJ	= server.o
EXE = server

# Creating the executable
$(EXE): $(OBJ)
	$(CC) -o $(EXE) $(OBJ)

clean:
	rm -f $(OBJ)

clobber: clean
	rm $(EXE)