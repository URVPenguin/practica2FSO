all : build/winsuport.o build/winsuport2.o build/missatge.o build/semafor.o build/memoria.o tennis0 tennis1 tennis2 tennis3

dirs:
	@mkdir -p bin 2> /dev/null 
	@mkdir -p build 2> /dev/null

build/winsuport.o : src/winsuport.c include/winsuport.h dirs
	gcc -Wall -c src/winsuport.c -o build/winsuport.o

build/winsuport2.o : src/winsuport2.c include/winsuport2.h dirs
	gcc -Wall -c src/winsuport2.c -o build/winsuport2.o

build/memoria.o : src/memoria.c include/memoria.h dirs
	gcc -c -Wall src/memoria.c -o build/memoria.o 

build/semafor.o : src/semafor.c include/semafor.h dirs
	gcc -c -Wall src/semafor.c -o build/semafor.o 

build/missatge.o : src/missatge.c include/missatge.h dirs
	gcc -c -Wall src/missatge.c -o build/missatge.o

tennis0 : src/tennis0.c build/winsuport.o include/winsuport.h dirs
	gcc -Wall src/tennis0.c build/winsuport.o -o bin/tennis0 -lcurses

tennis1 : src/tennis1.c build/winsuport.o include/winsuport.h dirs
	gcc -Wall src/tennis1.c build/winsuport.o -o bin/tennis1 -lcurses -lpthread

tennis2 : src/tennis2.c build/winsuport.o include/winsuport.h dirs
	gcc -Wall src/tennis2.c build/winsuport.o -o bin/tennis2 -lcurses -lpthread

tennis3 : src/tennis3.c build/winsuport2.o build/memoria.o build/semafor.o dirs
	gcc -Wall src/tennis3.c build/winsuport2.o build/semafor.o build/memoria.o -o bin/tennis3 -lcurses -lpthread
	gcc -Wall src/pal_ord3.c build/winsuport2.o build/semafor.o build/memoria.o -o bin/pal_ord3 -lcurses

clean: dirs
	rm bin/*
	rm build/*