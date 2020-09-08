
FLAGS  = -Wall -g -pthread #-DDEBUG
CC     = gcc
PROG   = SimulationManager.out
OBJS   = drone_movement.o linkedlists.o files.o central.o SimulationManager.o 

all:	${PROG}

clean:
	rm ${OBJS} ${PROG}
  
${PROG}:	${OBJS}
	${CC} ${FLAGS} ${OBJS} -o $@ -lm 

.c.o:
	${CC} ${FLAGS} $< -c

##########################

drone_movement.o: drone_movement.c drone_movement.h
files.o: files.c files.h
linkedlists.o: linkedlists.c linkedlists.h
central.o: central.c central.h
SimulationManager.o: SimulationManager.c structs.h extern.h
SimulationManager: SimulationManager.o drone_movement.o files.o central.o linkedlists.o
