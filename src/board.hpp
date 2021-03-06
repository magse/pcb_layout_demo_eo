#pragma once

#include "part.hpp"

#include <cstdlib>

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
	string filename;
	board(std::string& prefix) {
		uint64_t sd=chrono::system_clock::now().time_since_epoch().count();
		re.seed(static_cast<default_random_engine::result_type>(sd));
		uniform_int_distribution<uint32_t> dist(1,1000000000);
		ostringstream fn;
		fn << prefix << ".csv";
		cout << fn.str() << endl;
		filename=fn.str();
		resfile.open(fn.str());
		circle_t c(0,0,TARGET_RADIUS);
		c=vec2<real_t>(T(0.5)*CHANNEL_DISTANCE,0);
		targets.push_back(c);
		c=vec2<real_t>(T(1.5)*CHANNEL_DISTANCE,0);
		targets.push_back(c);
		c=vec2<real_t>(T(2.5)*CHANNEL_DISTANCE,0);
		targets.push_back(c);
		c=vec2<real_t>(0,T(0.5)*CHANNEL_DISTANCE);
		targets.push_back(c);
		c=vec2<real_t>(0,T(1.5)*CHANNEL_DISTANCE);
		targets.push_back(c);
		c=vec2<real_t>(0,T(2.5)*CHANNEL_DISTANCE);
		targets.push_back(c);
	}
	~board() {
		if(resfile.is_open()) resfile.close();
		ostringstream cmd;
		cmd << "gzip -9 " << filename;
		std::system(cmd.str().c_str());
	}
	size_t size() {return parts.size();}
	void balance_parts() {
		size_t nLEDS=0;
		for(auto& p:parts) if(p.is_LED()) nLEDS++;
		size_t nDrivers=nLEDS/8+1;
		for(auto& p:parts) if(DRIVER==p.typ) nDrivers--;
		add_parts(DRIVER,nDrivers);
		// TODO: Resistors disabled for now
//		size_t nResistors=nLEDS;
//		for(auto& p:parts) if(RESISTOR==p.typ) nResistors--;
//		add_parts(RESISTOR,nResistors);
	}
	size_t add_part(part_type t) {
		uniform_real_distribution<T> dist(0,WORLD_SIZE);
		parts.push_back(part_t(dist(re),dist(re),t));
		balance_parts();
		return size();
	}
	size_t add_parts(part_type t,size_t n) {
		for(size_t i=0;i<n;i++) add_part(t);
		return size();
	}
	void sort_radius() {
		sort(begin(parts),end(parts),[](auto& a,auto& b){return a.border.r > b.border.r;});
	}
	size_t copy_from(board& brd) {
		parts.clear();
		copy(begin(brd.parts),end(brd.parts),back_inserter(parts));
		sort_radius();
		return size();
	}
	size_t configuration_default() {
		add_parts(LED3,72);
		add_parts(LED5,24);
		add_parts(LED8,18);
		sort_radius();
		return size();
	}
	part_t* nearest_part(part_t& p) {
		real_t f=sqr(WORLD_SIZE);
		auto cp=&p;
		for(auto& q:parts) {
			if(0<p.border.intersection_area(q.border)) {
				auto d=q.border.center()-p.border.center();
				if(0<d.length() && d.length()<f) {
					cp=&q;
				}
			}
		}
		return cp;
	}
	real_t flaw_overlay(part_t& p) {
		auto cp=nearest_part(p);
		auto d=cp->border.center()-p.border.center();
		return abs(d.length()/(cp->border.r+p.border.r));
	}
	real_t flaw_range(part_t& p) {
		real_t f=3.0;
		for(auto& t:targets) {
			auto d=t.center() - p.border.center();
			if(d.length()<p.rng) f=f-1.0;
		}
		return posvalue(f);
	}
	real_t flaw(part_t& p) {
		real_t f=0;
		bool seqflaws=true;
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
		f+=flaw_range(p);
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
		uniform_real_distribution<real_t> dist(-1.0,1.0);
		if(0<p.flaw_outside(outeredge)) {
			p.border.x=p.border.x*real_t(0.99)/*+dist(re)*/;
			p.border.y=p.border.y*real_t(0.99)/*+dist(re)*/;
            return true;
		}
		if(0<p.flaw_inside(inneredge)) {
			p.border.x=p.border.x*real_t(1.01)/*+dist(re)*/;
			p.border.y=p.border.y*real_t(1.01)/*+dist(re)*/;
            return true;
		}
		if(0<p.flaw_bottom(SIDEDISTANCE)) {
			p.border.x=p.border.x;
			p.border.y=p.border.y+real_t(0.1)/*+dist(re)*/;
            return true;
		}
		if(0<p.flaw_side(SIDEDISTANCE)) {
			p.border.x=p.border.x+real_t(0.1)/*+dist(re)*/;
			p.border.y=p.border.y;
            return true;
		}
		if(0<flaw_overlay(p)) {
			// This flaw differs from the ones above in that the other parts have to move when this part scores a flaw.
			auto cp=nearest_part(p);
			auto d=cp->border.center()-p.border.center();
			cp->border+=(d*real_t(0.05)*p.border.r);
			p.border-=(d*real_t(0.05)*p.border.r);
			limit_to<real_t>(cp->border.x,0,+WORLD_SIZE);
			limit_to<real_t>(cp->border.y,0,+WORLD_SIZE);
			limit_to<real_t>(p.border.x,0,+WORLD_SIZE);
			limit_to<real_t>(p.border.y,0,+WORLD_SIZE);
			return true;
		}
		if(0<flaw_range(p)) {
			vector<real_t> pwrs(targets.size(),0);
			for(auto& q:parts) {
				if(q.border.r==p.border.r) {
					for(size_t t=0;t<targets.size();t++) {
						auto d=p.border.center() - targets[t].center();
						pwrs[t]+=p.rng/sqr(d.length());
					}
				}
			}
			size_t g=0;
			for(size_t n=0;n<pwrs.size();n++) {
				if(pwrs[n]<pwrs[g]) g=n;
			}
			auto d=targets[g].center() - p.border.center();
			d.normalize();
			p.border+=(d*real_t(0.25)*p.border.r);
			limit_to<real_t>(p.border.x,0,+WORLD_SIZE);
			limit_to<real_t>(p.border.y,0,+WORLD_SIZE);
			return true;
		}
		return false;
	}
	void save_state(real_t F,size_t i,size_t w) {
		for(auto& p:parts) {
			assert(!isnan(p.border.x)&&!isinf(p.border.x));
			assert(!isnan(p.border.y)&&!isinf(p.border.y));
		}
		resfile << w << CSV << F << CSV << i;
		resfile << CSV << parts[w].border.x << CSV << parts[w].border.y << CSV << parts[w].border.r;
		resfile << endl;
        filecounter++;
	}
	void save_all_states() {
		for(size_t w=0;w<parts.size();w++) save_state(-1,0,w);
	}
	size_t select_worst(flaws_t& flaws,const real_t lambda,size_t& n) {
		n=0;
		uniform_real_distribution<real_t> dist(0,1);
		if(dist(re)<0.05) n++;
		if(dist(re)<0.005) n++;
		if(dist(re)<0.0001) n++;
		return flaws[n].first;
	}
	bool one_step(flaws_t& fl) {
		auto F=flaws(fl);
        sort(begin(fl),end(fl),[](auto& a,auto& b){
            return a.second>b.second;
        });
		size_t i=0;
		size_t w=select_worst(fl,real_t(0.05),i);
		bool res=improve(parts[w]);
		save_state(F,i,w);
		return res;
	}
	size_t run_steps(const size_t stp=1000) {
		cout << "running " << stp << " more steps" << endl;
		save_all_states();
		flaws_t fl;
		size_t s=stp;
		while(s && one_step(fl)) s--;
		cout << "all steps consumed or target reached" << endl;
		return stp-s;
	}
};

}
