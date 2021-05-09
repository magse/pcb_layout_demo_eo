#pragma once

#include "math.hpp"

namespace pcbeo {

template<typename T> struct LED : public geometry2d::circle2<T> {
	typedef T real_t;
	typedef geometry2d::vec2<T> vec_t;
	typedef geometry2d::circle2<T> circle_t;
	real_t rng=0;
	real_t pwr=0;
	real_t energy(circle_t& c) {
		auto d=circle_t::distance(c);
		d*=pwr*c.area()/circle_t::area();
	}
};


template<typename T> T flaw_outside(circle2<T>& c) {
	const T R=T(OUTSIDE_RADIUS);
	auto d=c.p.length();
	return outsidearea(c.r,R,d)/area(c.r);
}

template<typename T> T flaw_inside(circle2<T>& c) {
	const T R=T(INSIDE_RADIUS);
	auto d=c.p.length();
	return insidearea(c.r,R,d)/area(c.r);
}

template<typename T> T flaw_border(circle2<T>& c) {
	const T D=T(BORDER_DISTANCE);
	T dx=0;
	T dy=0;
	T e=0;
	if((c.p.y+c.r)<D) {
		dy=c.p.y+c.r-D;
		e+=gtoz(-dy)*c.r;
	}
	if((c.p.x+c.r)<D) {
		dx=c.p.x+c.r-D;
		e+=gtoz(-dx)*c.r;
	}
	return T(2)*e/c.area();
}

template<typename T> T flaw_comp(circle<T>& c) {
	if(c.ref) {
		auto d=c.rimdistance(*(c.ref));
		if(d<c.r) d=0;
		return d*T(2)/c.area();
		//		return 0;
	}
	return 0;
}

template<typename T> int improve_comp(circle<T>& c) {
	if(c.ref) {
		//		auto d=c.rimdistance(*(c.ref));
		//		auto dx=c.p.x - c.ref->p.x;
		//		auto dy=c.p.y - c.ref->p.y;
		//		auto L=sqrt(sqr(dx)+sqr(dy));
		//		if(dx) dx/=L;
		//		if(dy) dy/=L;
		//		if(d<0) {
		//			c.p.x+=dx*T(WORLD_SIZE)*T(SMALL_STEP);
		//			c.p.y+=dy*T(WORLD_SIZE)*T(SMALL_STEP);
		//		}
		//		if(d>2*c.r) {
		//			c.p.x-=dx*T(WORLD_SIZE)*T(SMALL_STEP);
		//			c.p.y-=dy*T(WORLD_SIZE)*T(SMALL_STEP);
		//		}
	}
	return 6;
}

template<typename T> T flaw_smaller(circle<T>& c1,vector<circle<T>*>& cn) {
	if(c1.r<2) return 0;
	if(cn.front()->r<2) return 0;
	auto d1=c1.p.length();
	auto d2=cn.front()->p.length();
	auto A1=c1.area();
	auto A2=cn.front()->area();
	auto At=A1+A2;
	auto Mt=A1*d1+A2*d2;
	return At*gtoz(Mt/At-d1)/d1;
}

template<typename T> int improve_smaller(circle<T>& c1,vector<circle<T>*>& cn,RANDOMENGINE& re) {
	c1.swap(*(cn.front()));
	c1.shake_and_check(re);
	return 4;
}

template<typename T> T flaw_overlap(circle<T>& c,vector<circle<T>*>& cn) {
	auto Ac=c.area();
	auto Ae=0;
	for(auto& n:cn) {
		auto d=distance(c.p,n->p);
		Ae+=unionarea(c.r,n->r,d);
	}
	return Ae/Ac;
}

template<typename T> int improve_overlap(circle<T>& c1,vector<circle<T>*>& cn,RANDOMENGINE& re) {
	point<T> pc={0,0};
	T At=0;
	for(auto& c:cn) {
		T d=c1.p.distance(c->p);
		T Ac=c->area();
		T Au=unionarea(c1.r,c->r,d);
		point<T> cc=c->p;
		cc-=c1.p;
		if(Au) {
			cc*=Ac/Au;
			pc+=cc;
			At+=Ac/Au;
		}
	}
	if(At) {
		pc*=(div(T(1),At));
		c1.p-=pc;
		c1.shake_and_check(re);
	}
	return 5;
}



template<typename T> T IC_size(size_t n) {
	if(0==n) return COMP_SIZE;
	if(1==n) return LED3_SIZE;
	if(2==n) return LED5_SIZE;
	if(3==n) return LED8_SIZE;
	if(4==n) return LEDX_SIZE;
	if(5==n) return LEDB_SIZE;
	if(6==n) return DRIVER_SIZE;
	return 0;
}

template<typename T> string IC_style(size_t n) {
	if(0==n) return string("fill:#3b4d33;stroke:#000000;stroke-width:0.0;stroke-linecap:round;stroke-linejoin:round");
	if(1==n) return string("fill:#5271ff;stroke:#444444;stroke-width:0.2;stroke-linecap:round;stroke-linejoin:round");
	if(2==n) return string("fill:#1b9bd9;stroke:#444444;stroke-width:0.2;stroke-linecap:round;stroke-linejoin:round");
	if(3==n) return string("fill:#fbfae1;stroke:#444444;stroke-width:0.4;stroke-linecap:round;stroke-linejoin:round");
	if(4==n) return string("fill:#12ffd0;stroke:#444444;stroke-width:0.4;stroke-linecap:round;stroke-linejoin:round");
	if(5==n) return string("fill:#9999ff;stroke:#444444;stroke-width:0.4;stroke-linecap:round;stroke-linejoin:round");
	if(6==n) return string("fill:#e7f419;stroke:#444444;stroke-width:0.4;stroke-linecap:round;stroke-linejoin:round");
	return 0;
}

template<typename T> T LED_range(size_t n) {
	if(1==n) return LED3_RANGE;
	if(2==n) return LED5_RANGE;
	if(3==n) return LED8_RANGE;
	if(4==n) return LEDX_RANGE;
	if(5==n) return LEDB_RANGE;
	return 0;
}

template<typename T> T LED_power(size_t n) {
	if(1==n) return LED3_POWER;
	if(2==n) return LED5_POWER;
	if(3==n) return LED8_POWER;
	if(4==n) return LEDX_POWER;
	if(5==n) return LEDB_POWER;
	return 0;
}

template<typename T> bool is_LED(size_t n) {
	return (1<=n && n<=5) ? true : false;
}

template<typename T> struct part {
	typedef T real_t;
	typedef circle<T> circle_t;
	typedef vector<circle_t*> neighbours_t;
	circle_t zone;
	size_t type=0;
	real_t range=0;
	neighbours_t neighbours;
	part* comp=nullptr;
	part(RANDOMENGINE& re,size_t t) {
		type=t;
		zone.r=real_t(0.5)*IC_size<T>(t);
		zone.p.random_radius_even(re,T(WORLD_SIZE),0);
		if(is_LED<T>(type)) range=LED_range<T>(type); else range=0;
	}
	bool is_enabled() {
		return zone.p.length()<WORLD_SIZE;
	}
	void disable(RANDOMENGINE& re) {
		zone.p.random(re,WORLD_SIZE,M_SQRT2*WORLD_SIZE);
	}
	real_t distance(part& q) {return zone.p.distance(q.zone.p);}
	void save_to_svg(ostream& f) {
		zone.save_to_svg(f,type);
	}
};

}
