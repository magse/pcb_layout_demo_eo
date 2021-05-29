//
//  main.cpp
//  pcb_layout_demo_eo
//
//  Created by Magnus Sethson on 2021-04-06.
//

#include <iostream>
#include <unistd.h>

#include <cstring>

#include "board.hpp"

using namespace geometry2d;
using namespace std;

string job_prefix(uint64_t& sd,const uint32_t cnt=0,const char* tag=nullptr,uint32_t jobnr=0) {
	if(!sd) sd=chrono::system_clock::now().time_since_epoch().count();
	ostringstream fn;
	if(tag) fn << tag << "_";
	if(jobnr) fn << "J" << setfill('0') << setw(11) << jobnr << "_";
	fn << "T" << setfill('0') << setw(18) << sd << "_";
	fn << setfill('0') << setw(9) << cnt;
	string p=fn.str();
	return p;
}

int help(const int ret,ostream& f=cout) {
	f << "no help yet" << endl;
	return ret;
}

int version(const int ret,ostream& f=cout) {
	f << "no version yet" << endl;
	return ret;
}

int main(int argc, const char * argv[]) {
	
	uint64_t sd=0;
	
	size_t testnr=0;
	size_t steps=10000;
	uint32_t jobnumber=0;
	
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
		if(a<argc && (0==strcmp(argv[a],"-j") || 0==strcmp(argv[a],"--jobnumber"))) {
			a++;
			if(a<argc) jobnumber=static_cast<uint32_t>(atoll(argv[a]));
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
		auto prefix=job_prefix(sd,0,"BRD",jobnumber);
		board_t brd(prefix);
		brd.configuration_default();
		brd.run_steps(steps);
	}
	
	if(1==testnr) {
		cout << "Running test 1" << endl;
		board_t* brd=nullptr;
		uint32_t cnt=0;
		auto prefix=job_prefix(sd,cnt++,"PCBEO",jobnumber);
		brd=new board_t(prefix);
		uint32_t n3=1;
		uint32_t n5=1;
		uint32_t n8=1;
		uint32_t nX=1;
		bool res=true;
		do {
			if(!(--n3)) brd->add_part(pcbeo::LED3);
			if(!(--n5)) brd->add_part(pcbeo::LED5);
			if(!(--n8)) brd->add_part(pcbeo::LED8);
			if(!(--nX)) brd->add_part(pcbeo::LEDX);
			if(!n3) n3=1;
			if(!n5) n5=3;
			if(!n8) n8=5;
			if(!nX) nX=12;
			cout << brd->parts.size() << " parts" << endl;
			res=false;
			if(steps==brd->run_steps(steps)) break;
			res=true;
			auto oldbrd=brd;
			prefix=job_prefix(sd,cnt++,"PCBEO",jobnumber);
			brd=new board_t(prefix);
			brd->copy_from(*oldbrd);
			delete oldbrd;
			if(brd->parts.size()>300) {
				cout << "max parts reached" << endl;
				break;
			}
		} while(res);
		delete brd;
	}
	
	return 0;
}
