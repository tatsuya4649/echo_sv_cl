CC:=gcc

SERVER:=server
CLIENT:=client

COMMON:=writen.o
ONLY_SERVER:=
ONLY_CLIENT:=
BIN:=bin
OBJECT:=obj

all: ${addsuffix .o,${SERVER}} ${addsuffix .o,${CLIENT}} ${COMMON} ${ONLY_SERVER} ${ONLY_CLIENT}
	${CC} ${addsuffix .o,${SERVER}} ${COMMON} ${ONLY_SERVER} -o ${BIN}/${SERVER}
	${CC} ${addsuffix .o,${CLIENT}} ${COMMON} ${ONLY_CLIENT} -o ${BIN}/${CLIENT}
	mv *.o ${OBJECT}

%.o: %.c
	${CC} -c $< -o $@
clean:
	rm -f *.o bin/*
