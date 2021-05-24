//
//  main.cpp
//  pcb_layout_demo_eo
//
//  Created by Magnus Sethson on 2021-04-06.
//

#include <iostream>

#include "board.hpp"

using namespace geometry2d;
using namespace std;

int main(int argc, const char * argv[]) {
		
	size_t testnr=0;
	size_t steps=500000;
	size_t pinc=50;
	if(argc>1) testnr=atoll(argv[1]);
	if(argc>2) steps=atoll(argv[2]);
	if(argc>3) pinc=atoll(argv[3]);

	typedef float real_t;
	typedef pcbeo::board<real_t> board_t;

	board_t brd;

	if(0==testnr) {
		brd.configuration_default();
		brd.run_steps(steps,pinc);
	}

//	if(1==testnr) {
//		brd.configuration_simple1();
//		brd.run_steps(steps,pinc);
//	}
//	if(2==testnr) {
//		brd.configuration_simple2();
//		brd.run_steps(steps,pinc);
//	}
//	if(3==testnr) {
//		brd.configuration_simple3();
//		brd.run_steps(steps,pinc);
//	}
//	if(4==testnr) {
//		size_t asteps=0;
//		do {
//			asteps=0;
//			brd.configuration_add_LED();
//			asteps=brd.run_steps(steps,pinc);
//		} while(asteps!=steps);
//	}
	
	return 0;
}
