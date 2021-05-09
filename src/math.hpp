#pragma once

#include "config.hpp"
#include "geometry2d.hpp"

using namespace std;
using namespace geometry2d;

namespace pcbeo {

template<typename T> T sqr(T x) { return x*x;}

template<typename T> T gtoz(const T x) {
	if(x<0) return 0;
	return x;
}

template<typename T> T div(const T a,const T b) {
	assert(!isnan(b)&&!isinf(b));
	assert(!isnan(a)&&!isinf(a));
	assert(0!=b);
	return a/b;
}

template<typename T> size_t random_exp(RANDOMENGINE& re,const size_t s,const T prob,const T lambda) {
	exponential_distribution<T> dist(lambda);
	T r=dist(re);
	r=min(r,T(5.0*lambda));
	r=r/T(5.0*lambda);
	size_t i=static_cast<size_t>(floor(s*prob*r));
	return i;
}

/// https://mathworld.wolfram.com/Circle-CircleIntersection.html
/// https://www.xarg.org/2016/07/calculate-the-intersection-area-of-two-circles/
template<typename T> T maximumchorddistance(const T R,const T r) {
	assert(R>=r);
	assert(r>0);
	T d_max=sqrt(abs(sqr(r)-sqr(R)));
	return d_max;
}

template<typename T> T segmentangle(const T R,const T d) {
	assert(abs(d)<=R);
	T a=acos(div(d,R));
	if(a<0) a+=T(M_PI);
	return a;
}

template<typename T> T maximumsegmentangle(const T R,const T r) {
	T d_max=maximumchorddistance(R,r);
	return segmentangle(r,d_max);
}

template<typename T> T segmentarea(const T R,const T d) {
	if(d>R) return 0;
	if(d<-R) return area(R);
	return sqr(R)*segmentangle(R,d)-d*sqrt(sqr(R)-sqr(d));
}

template<typename T> T unionarea(const T r,const T R,const T d) {
	if(r>R) return unionarea(R,r,d);
	if((r+R)<d) return 0;
	if((r+d)<R) return area(r);
	T d1=div(sqr(d)-sqr(r)+sqr(R),T(2)*d);
	T d2=div(sqr(d)+sqr(r)-sqr(R),T(2)*d);
	return segmentarea(R,d1)+segmentarea(r,d2);
}

template<typename T> T outsidearea(const T r,const T R,const T d) {
	if(r>R) return outsidearea(R,r,d);
	if(d<(R-r)) return 0;
	T d_opt=maximumchorddistance(R,r);
	if(d>d_opt) {
		return (d-d_opt)*T(2)*r+0.5*area(r);
	}
	return unionarea(r,R,d);
}

template<typename T> T insidearea(const T r,const T R,const T d) {
	if(r>R) return insidearea(R,r,d);
	if(d>(R+r)) return 0;
	T d_opt=maximumchorddistance(R,r);
	if(d<d_opt) {
		return (d_opt-d)*T(2)*r+0.5*area(r);
	}
	return unionarea(r,R,d);
}

template<typename T> T outside_relative_area(circle2<T>& C,circle2<T>& c) {
	if(C.r<c.r) return outside_relative_area(c,C);
	R A=c.inter
}

}
