all: program1 program2

program1: 	#compile P1.c
	gcc  P1.c SemFunctions.c -o P1 -lcrypto

program2:	#compile P2.c
	gcc  P2.c SemFunctions.c -o P2 -lcrypto

