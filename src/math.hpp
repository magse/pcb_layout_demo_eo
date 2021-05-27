#pragma once

#include "config.hpp"
#include <geometry2d/geometry2d.hpp>

using namespace std;
using namespace geometry2d;

namespace pcbeo {

template<typename T> T sqr(T x) { return x*x;}

template<typename T> T posvalue(const T x) {return x>T(0) ? x : T(0);}

template<typename T> T div(const T a,const T b) {
	assert(!isnan(b)&&!isinf(b));
	assert(!isnan(a)&&!isinf(a));
	assert(0!=b);
	return a/b;
}

template<typename T> size_t random_exp(RANDOMENGINE& re,const size_t s,const T lambda) {
	geometric_distribution<size_t> dist(lambda);
	size_t i=dist(re);
	if(i>=s) i=s-1;
	return i;
}

template<typename T> void limit_to(T& x,const T a,const T b) {
	assert(!isnan(x));
	assert(!isinf(x));
	if(x<a) x=a;
	if(x>b) x=b;
}

template<typename T> T maximumchorddistance(const T R,const T r) {
	/// https://mathworld.wolfram.com/Circle-CircleIntersection.html
	/// https://www.xarg.org/2016/07/calculate-the-intersection-area-of-two-circles/
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

template<typename T> T intersection_relative_area(circle2<T>& C,circle2<T>& c) {
	T A=c.intersection_area(C);
	return A/c.area();
}

}
