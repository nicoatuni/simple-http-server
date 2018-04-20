CC	= gcc

# CHANGE NEW TO SERVER
SRC	= new.c
OBJ	= new.o
EXE = new

# Creating the executable
$(EXE): $(OBJ)
	$(CC) -o $(EXE) $(OBJ)

clean:
	rm -f $(OBJ)

clobber: clean
	rm $(EXE)