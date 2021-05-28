
all : pcb_layout_demo_eo

pcb_layout_demo_eo : pcb_layout_demo_eo.cpp board.hpp config.hpp math.hpp part.hpp
	g++ -Wall -pedantic -O3 -std=c++17 -o pcb_layout_demo_eo pcb_layout_demo_eo.cpp -lm

clean :
	rm -fr *~
	rm -fr *.o

distclean : clean
	rm -fr pcb_layout_demo_eo


install : pcb_layout_demo_eo
	make -p ${HOME}/bin
	install -c pcb_layout_demo_eo ${HOME}/bin 
