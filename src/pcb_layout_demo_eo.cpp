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
	size_t steps=40000;
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
	
	return 0;
}
