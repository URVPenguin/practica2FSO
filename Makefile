all : winsuport.o winsuport2.o missatge.o semafor.o memoria.o tennis0 tennis1 tennis2 tennis3

winsuport.o : winsuport.c winsuport.h
	gcc -Wall -c winsuport.c -o winsuport.o

winsuport2.o : winsuport2.c winsuport2.h
	gcc -Wall -c winsuport2.c -o winsuport2.o

memoria.o : memoria.c memoria.h
	gcc -c -Wall memoria.c -o memoria.o 

semafor.o : semafor.c semafor.h
	gcc -c -Wall semafor.c -o semafor.o 

missatge.o : missatge.c missatge.h
	gcc -c -Wall missatge.c -o missatge.o

tennis0 : tennis0.c winsuport.o winsuport.h
	gcc -Wall tennis0.c winsuport.o -o tennis0 -lcurses

tennis1 : tennis1.c winsuport.o winsuport.h
	gcc -Wall tennis1.c winsuport.o -o tennis1 -lcurses -lpthread

tennis2 : tennis2.c winsuport.o winsuport.h
	gcc tennis2.c winsuport.o -o tennis2 -lcurses -lpthread

tennis3 : tennis3.c winsuport2.o memoria.o
	gcc -Wall tennis3.c winsuport2.o memoria.o -o tennis3 -lcurses -lpthread
	gcc pal_ord3.c winsuport2.o memoria.o -o pal_ord3 -lcurses

clean: 
	rm winsuport.o missatge.o memoria.o semafor.o winsuport2.o tennis0 tennis1 tennis2 tennis3