CC:=gcc

SERVER:=server server2
CLIENT:=client

COMMON:=writen.o readline.o
ONLY_SERVER:=signal.o
ONLY_CLIENT:=
BIN:=bin
OBJECT:=obj

all: ${addsuffix .o,${SERVER}} ${addsuffix .o,${CLIENT}} ${COMMON} ${ONLY_SERVER} ${ONLY_CLIENT}
	for server in ${SERVER};do ${CC} $$server.o ${COMMON} ${ONLY_SERVER} -o ${BIN}/$$server; done
	${CC} ${addsuffix .o,${CLIENT}} ${COMMON} ${ONLY_CLIENT} -o ${BIN}/${CLIENT}
	mv *.o ${OBJECT}
%.o: %.c
	${CC} -c $< -o $@
clean:
	rm -f *.o bin/* obj/*
