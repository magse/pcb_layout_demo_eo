#pragma once

#include "part.hpp"

namespace pcbeo {

template<typename T> struct board {
	typedef T real_t;
	typedef geometry2d::vec2<T> point_t;
	typedef geometry2d::circle2<T> circle_t;
	typedef part<T> part_t;
	typedef vector<part_t> parts_t;
	typedef pair<size_t,real_t> flaw_t;
	typedef vector<flaw_t> flaws_t;
	typedef vector<circle_t> targets_t;
	parts_t parts;
	circle_t outeredge={0,0,OUTEREDGERADIUS};
	circle_t inneredge={0,0,INNEREDGERADIUS};
	RANDOMENGINE re;
	size_t filecounter=0;
	targets_t targets;
	ofstream resfile;
	board() {
		uint64_t sd=chrono::system_clock::now().time_since_epoch().count();
		re.seed(static_cast<default_random_engine::result_type>(sd));
		ostringstream fn;
		fn << "BRD" << setfill('0') << setw(8) << sd << ".csv";
		cout << fn.str() << endl;
		resfile.open(fn.str());
		for(int i=0;i<6;i++) {
			T z=T(-2.5)*T(CHANNEL_DISTANCE)+i*T(CHANNEL_DISTANCE);
			circle_t c1(z,T(0),T(TARGET_RADIUS));
			targets.push_back(c1);
		}
	}
	~board() {
		if(resfile.is_open()) resfile.close();
	}
	size_t size() {return parts.size();}
	void balance_parts() {
		size_t nLEDS=0;
		for(auto& p:parts) if(p.is_LED()) nLEDS++;
		size_t nDrivers=nLEDS/8+1;
		for(auto& p:parts) if(DRIVER==p.typ) nDrivers--;
		add_parts(DRIVER,nDrivers);
		size_t nResistors=nLEDS;
		for(auto& p:parts) if(RESISTOR==p.typ) nResistors--;
		add_parts(RESISTOR,nResistors);
	}
	size_t add_part(part_type t) {
		uniform_real_distribution<T> dist(-WORLD_SIZE,WORLD_SIZE);
		parts.push_back(part_t(dist(re),dist(re),t));
		balance_parts();
		return size();
	}
	size_t add_parts(part_type t,size_t n) {
		for(size_t i=0;i<n;i++) add_part(t);
		return size();
	}
	size_t configuration_default() {
		add_parts(LED3,12);
		add_parts(LED5,8);
		add_parts(LED8,4);
		return size();
	}
	real_t flaw_overlay(part_t& p) {
		real_t A=0;
		for(auto& q:parts) {
			if(&p!=&q) {
				A+=p.border.intersection_area(q.border);
			}
		}
		assert(p.border.area());
		real_t f=A/p.border.area();
		return f;
	}
	real_t flaw(part_t& p) {
		real_t f=0;
		bool seqflaws=false;
		f+=p.flaw_outside(outeredge);
		if(seqflaws && f) return f;
		f+=p.flaw_inside(inneredge);
		if(seqflaws && f) return f;
		f+=p.flaw_bottom(SIDEDISTANCE);
		if(seqflaws && f) return f;
		f+=p.flaw_side(SIDEDISTANCE);
		if(seqflaws && f) return f;
		f+=flaw_overlay(p);
		if(seqflaws && f) return f;
		return f;
	}
	real_t flaws(flaws_t& fl) {
		fl.clear();
        size_t n=0;
		long double F=0;
		for(auto& p:parts) {
			auto f=flaw(p);
			fl.push_back(make_pair(n++,f));
			F+=f;
		}
#ifdef DEBUG
		assert(fl[0].first==0);
		assert(fl.size()==parts.size());
#endif
		return static_cast<real_t>(F);
	}
	bool improve(part_t& p) {
		uniform_real_distribution<real_t> dist(-2.0,2.0);
		if(0<p.flaw_outside(outeredge)) {
			p.border.x=p.border.x*real_t(0.98)+dist(re);
			p.border.y=p.border.y*real_t(0.98)+dist(re);
            return true;
		}
		if(0<p.flaw_inside(inneredge)) {
			p.border.x=p.border.x*real_t(1.02)+dist(re);
			p.border.y=p.border.y*real_t(1.02)+dist(re);
            return true;
		}
		if(0<p.flaw_bottom(SIDEDISTANCE)) {
			p.border.x=p.border.x;
			p.border.y=p.border.y+real_t(1)+dist(re);
            return true;
		}
		if(0<p.flaw_side(SIDEDISTANCE)) {
			p.border.x=p.border.x+real_t(1)+dist(re);
			p.border.y=p.border.y;
            return true;
		}
		if(0<flaw_overlay(p)) {
			// This flaw differs from the ones above in that the other parts have to move when this part scores a flaw.
			real_t A=0;
			vector<real_t> pA(parts.size(),0);
			size_t n=0;
			for_each(begin(parts),end(parts),[&p,&A,&n,&pA](auto& q){
				if(&p!=&q) {
					A+=pA[n++]=p.border.intersection_area(q.border);
				}
			});
			if(0==A) return false;
			for(auto& a:pA) a=a/(A+p.border.area())*real_t(0.01)*p.border.r;
			for(n=0;n<parts.size();n++) {
				assert(!isnan(pA[n])&&!isinf(pA[n]));
				vec2<real_t> d=parts[n].border.center()-p.border.center(); // Direction away from part p
				d*=pA[n];
				d+=(vec2<real_t>(dist(re),dist(re))*real_t(0.001));
				assert(!isnan(d.x)&&!isinf(d.x));
				assert(!isnan(d.y)&&!isinf(d.y));
				parts[n].border+=d;
				p.border-=d;
				limit_to<real_t>(parts[n].border.x,-WORLD_SIZE,+WORLD_SIZE);
				limit_to<real_t>(parts[n].border.y,-WORLD_SIZE,+WORLD_SIZE);
				limit_to<real_t>(p.border.x,-WORLD_SIZE,+WORLD_SIZE);
				limit_to<real_t>(p.border.y,-WORLD_SIZE,+WORLD_SIZE);
			}
			return true;
		}
		return false;
	}
	void save_state() {
		for(auto& p:parts) {
			assert(!isnan(p.border.x)&&!isinf(p.border.x));
			assert(!isnan(p.border.y)&&!isinf(p.border.y));
		}
		for(auto& p:parts) {
			resfile << CSV << p.border.x << CSV << p.border.y << CSV << p.border.r;
		}
		resfile << endl;
        filecounter++;
	}
	size_t select_worst(flaws_t& flaws,const real_t lambda,size_t& n) {
		n=random_exp(re,flaws.size(),lambda);
		return flaws[n].first;
	}
	bool one_step(flaws_t& fl) {
		auto F=flaws(fl);
        sort(begin(fl),end(fl),[](auto& a,auto& b){
            return a.second>b.second;
        });
		size_t i=0;
		size_t w=select_worst(fl,real_t(0.2),i);
		bool res=improve(parts[w]);
		resfile << filecounter << CSV << F << CSV << i;
		save_state();
		return res;
	}
	size_t run_steps(const size_t stp=1000,const size_t pinc=10000) {
		cout << "running " << stp << " more steps" << endl;
		flaws_t fl;
		for(size_t n=0;n<stp;n++) {
			one_step(fl);
		}
		cout << "all steps consumed" << endl;
		return stp;
	}
};

}
