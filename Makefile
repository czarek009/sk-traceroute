PRG 	 	 = main.c traceroute.c
OUT      = traceroute
CFLAGS   = -std=gnu17 -Wall -Wextra
ADDR     = 8.8.8.8

all: compile run

compile: 
	gcc ${CFLAGS} -o ${OUT}.exe ${PRG}

run:
	sudo ./${OUT}.exe ${ADDR}

clean:
	rm -rf ${OUT}.exe