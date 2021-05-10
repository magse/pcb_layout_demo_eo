#pragma once

#include "math.hpp"

namespace pcbeo {

template<typename T> struct LED : public geometry2d::circle2<T> {
	typedef T real_t;
	typedef geometry2d::vec2<T> vec_t;
	typedef geometry2d::circle2<T> circle_t;
	real_t rng=0;
	real_t pwr=0;
	LED()=default;
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
	T flaw_overlap(LED& othercomponent) {
		return intersection_relative_area(othercomponent,*this);
	}
	T flaw_range(LED& othercomponent) {
		if(circle_t::intersection_area(othercomponent)) return posvalue(rng-othercomponent.rng)/rng;
		return T(0);
	}
	string svg_style() {
		ostringstream ss;
		ss << "fill:#fbfae1;stroke:#444444;stroke-width:0.4;stroke-linecap:round;stroke-linejoin:round";
		return ss.str();
	}
	bool is_LED() {return pwr!=0;}
};

enum LEDtype : uint32_t {DRIVER=0, RESISTOR, LED3, LED5, LED8, LEDX, LEDB};

template<typename T> LED<T> make_LED(LEDtype t) {
	return LED<T>();
}

template<typename T> bool is_LED(size_t n) {
	return (1<=n && n<=5) ? true : false;
}

}
