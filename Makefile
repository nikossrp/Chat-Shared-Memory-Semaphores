all: program1 program2

program1: 	#compile P1.c
	# gcc -Wall P1.c SemFunctions.c -o P1 -lcrypto
	gcc  P1.c SemFunctions.c -o P1 -lcrypto

program2:	#compile P2.c
	# gcc -Wall P2.c SemFunctions.c -o P2 -lcrypto
	gcc  P2.c SemFunctions.c -o P2 -lcrypto

