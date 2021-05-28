//
//  main.cpp
//  pcb_layout_demo_eo
//
//  Created by Magnus Sethson on 2021-04-06.
//

#include <iostream>
#include <unistd.h>

#include "board.hpp"

using namespace geometry2d;
using namespace std;

int help(const int ret,ostream& f=cout) {
	f << "no help yet" << endl;
	return ret;
}

int version(const int ret,ostream& f=cout) {
	f << "no version yet" << endl;
	return ret;
}


int main(int argc, const char * argv[]) {
		
	size_t testnr=0;
	size_t steps=10000;
	int a=1;
	do {
		if(a<argc && (0==strcmp(argv[a],"-T") || 0==strcmp(argv[a],"--testnr"))) {
			a++;
			if(a<argc) testnr=atoll(argv[a]);
		}
		if(a<argc && (0==strcmp(argv[a],"-N") || 0==strcmp(argv[a],"--steps"))) {
			a++;
			if(a<argc) steps=atoll(argv[a]);
		}
		if(a<argc && (0==strcmp(argv[a],"-h") || 0==strcmp(argv[a],"--help"))) {
			return help(0);
		}
		if(a<argc && (0==strcmp(argv[a],"-V") || 0==strcmp(argv[a],"--version"))) {
			return version(0);
		}
		a++;
	} while(a<argc);
	
	typedef float real_t;
	typedef pcbeo::board<real_t> board_t;

	if(0==testnr) {
		board_t brd;
		brd.configuration_default();
		brd.run_steps(steps);
	}
	
	if(1==testnr) {
		board_t* brd=nullptr;
		brd=new board_t;
		brd->add_parts(pcbeo::LED3,6);
		brd->add_parts(pcbeo::LED5,3);
		brd->add_parts(pcbeo::LED8,2);
		brd->add_parts(pcbeo::LEDX,1);
		bool res=true;
		do {
			res=false;
			brd->balance_parts();
			if(steps==brd->run_steps(steps)) {
				delete brd;
				return 0;
			}
			res=true;
			auto oldbrd=brd;
			sleep(2);
			brd=new board_t;
			brd->copy_from(*oldbrd);
			brd->add_parts(pcbeo::LED3,6);
			brd->add_parts(pcbeo::LED5,3);
			brd->add_parts(pcbeo::LED8,2);
			brd->add_parts(pcbeo::LEDX,1);
			delete oldbrd;
		} while(res);
	}
	
	return 0;
}
