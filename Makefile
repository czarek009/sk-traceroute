PRG 	 	 = main.c traceroute.c
OUT      = traceroute
CFLAGS   = -std=gnu17 -Wall -Wextra
ADDR1    = 156.17.4.1
ADDR2    = 94.23.242.48

all: compile

compile: 
	gcc ${CFLAGS} -o ${OUT}.exe ${PRG}

run:
	sudo ./${OUT}.exe ${ADDR1}

run2:
	sudo ./${OUT}.exe ${ADDR2} > /dev/null & sudo ./${OUT}.exe ${ADDR1}

clean:
	rm -rf ${OUT}.exe

distclean:
	rm -rf *.o
	rm -rf ${OUT}.exe
