#pragma once

#include "math.hpp"

namespace pcbeo {

enum part_type : uint32_t {NONE=0,DRIVER=1, RESISTOR, LED3, LED5, LED8, LEDX, LEDB};

template<typename T> struct part : public geometry2d::circle2<T> {
	typedef T real_t;
	typedef geometry2d::vec2<T> vec_t;
	typedef geometry2d::circle2<T> circle_t;
	real_t rng=0;
	real_t pwr=0;
	part_type typ=NONE;
	part()=default;
	part(real_t px,real_t py,part_type t) {
		circle_t::x=px;
		circle_t::y=py;
		typ=t;
		switch (t) {
			case DRIVER:
				circle_t::r=0.5*DRIVER_SIZE;
				rng=WORLD_SIZE;
				pwr=0;
				break;
			case RESISTOR:
				circle_t::r=0.5*RESISTOR_SIZE;
				rng=WORLD_SIZE;
				pwr=0;
				break;
			case LED3:
				circle_t::r=0.5*LED3_SIZE;
				rng=LED3_RANGE;
				pwr=LED3_POWER;
				break;
			case LED5:
				circle_t::r=0.5*LED5_SIZE;
				rng=LED5_RANGE;
				pwr=LED5_POWER;
				break;
			case LED8:
				circle_t::r=0.5*LED8_SIZE;
				rng=LED8_RANGE;
				pwr=LED8_POWER;
				break;
			case LEDX:
				circle_t::r=0.5*LEDX_SIZE;
				rng=LEDX_RANGE;
				pwr=LEDX_POWER;
				break;
			case LEDB:
				circle_t::r=0.5*LEDB_SIZE;
				rng=LEDB_RANGE;
				pwr=LEDB_POWER;
				break;
			default:
				assert(false);
				break;
		}
	}
	part(circle_t pc,real_t rr,real_t pw) {
		circle_t::circle_t(pc);
		circle_t::rng=rr;
		circle_t::pwr=pw;
	}
	part(real_t px,real_t py,real_t pr,real_t rr,real_t pw) {
		circle_t::x=px;
		circle_t::y=py;
		circle_t::r=pr;
		circle_t::rng=rr;
		circle_t::pwr=pw;
	}
	real_t energy(circle_t& c) {
		auto d=circle_t::distance(c);
		d*=pwr*c.area()/circle_t::area();
	}
	T flaw_outside(circle_t& outeredge) {
		return T(1)-intersection_relative_area(outeredge,*this);
	}
	T flaw_inside(circle_t& inneredge) {
		return intersection_relative_area(inneredge,*this);
	}
	T flaw_bottom(real_t& Y) {
		return posvalue(Y - (circle_t::y-circle_t::r));
	}
	T flaw_side(real_t& X) {
		return posvalue(X - (circle_t::x-circle_t::r));
	}
	T flaw_overlap(part& othercomponent) {
		return intersection_relative_area(othercomponent,*this);
	}
	T flaw_range(part& othercomponent) {
		if(circle_t::intersection_area(othercomponent)) return posvalue(rng-othercomponent.rng)/rng;
		return T(0);
	}
	string svg_style() {
		ostringstream ss;
		ss << "fill:#fbfae1;stroke:#444444;stroke-width:0.4;stroke-linecap:round;stroke-linejoin:round";
		return ss.str();
	}
	bool is_LED() {return LED3==typ||LED5==typ||LED8==typ||LEDB==typ;}
};

}
