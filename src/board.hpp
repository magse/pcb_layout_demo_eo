#pragma once

#include "LED.hpp"

namespace pcbeo {

template<typename T> struct board {
	typedef T real_t;
	typedef geometry2d::vec2<T> point_t;
	typedef geometry2d::circle2<T> circle_t;
	typedef LED<T> LED_t;
	typedef vector<LED_t> parts_t;
	typedef pair<LED_t*,real_t> flaw_t;
	typedef vector<flaw_t> flaws_t;
	typedef vector<circle_t> targets_t;
	parts_t parts;
	RANDOMENGINE re;
	size_t filecounter=0;
	targets_t targets;
	ofstream resfile;
	board() {
		uint64_t sd=chrono::system_clock::now().time_since_epoch().count();
		re.seed(static_cast<default_random_engine::result_type>(sd));
		ostringstream fn;
		fn << "BRD" << setfill('0') << setw(8) << sd << ".txt";
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
		
	}
	size_t add_part(LEDtype t) {
		uniform_real_distribution<T> dist(0,WORLD_SIZE);
		circle_t pc(dist(re),dist(re),1);
		parts.push_back(make_LED<T>(t));
		balance_parts();
		return size();
	}
	size_t add_parts(LEDtype t,size_t n) {
		for(size_t i=0;i<n;i++) add_part(t);
		return size();
	}
	size_t configuration_default() {
		add_parts(LED3,20);
		add_parts(LED5,12);
		add_parts(LED8,6);
	}

	T flaw_targets(part_t* p) {
		size_t nt=0;
		if(LED_range<T>(p->type)<=0) return 0;
		auto pr=p->zone.p;
		for(auto& t:targets) {
			if(pr.distance(t.p)<LED_range<T>(p->type)) nt++;
			pr.x=-(p->zone.p.y);
			pr.y=+(p->zone.p.x);
			if(pr.distance(t.p)<LED_range<T>(p->type)) nt++;
			pr.x=-(p->zone.p.x);
			pr.y=-(p->zone.p.y);
			if(pr.distance(t.p)<LED_range<T>(p->type)) nt++;
			pr.x=+(p->zone.p.y);
			pr.y=-(p->zone.p.x);
			if(pr.distance(t.p)<LED_range<T>(p->type)) nt++;
//			svg_circle(f,-py,px,p.zone.r,"none","#888888",0.4);
//			svg_circle(f,-px,-py,p.zone.r,"none","#888888",0.4);
//			svg_circle(f,py,-px,p.zone.r,"none","#888888",0.4);
		}
		if(nt>3) return 0;
		if(nt>2) return 0.2;
		if(nt>1) return 0.5;
		return 1;
	}
	int improve_targets(part_t* p) {
		real_t dmax=sqr(T(WORLD_SIZE));
		auto tmax=targets.front();
		for(auto& t:targets) {
			if(p->zone.p.distance(t.p)<dmax) {
				dmax=p->zone.p.distance(t.p);
				tmax=t;
			}
		}
		point_t dir=tmax.p.direction(p->zone.p);
		dir*=T(SMALL_STEP);
		p->zone.p+=dir;
		p->zone.shake_and_check(re);
		return 7;
	}
	void get_flaws(flaws_t& flaws) {
		flaws.clear();
		flaws.resize(parts.size());
		for(size_t i=0;i<parts.size();i++) flaws[i].first=&(parts[i]);
		flaws.shrink_to_fit();
	}
	void update_flaws(flaws_t& flaws,const size_t base=0,const size_t inc=1) {
		for(size_t i=base;i<flaws.size();i+=inc) {
			flaws[i].second=flaw_outside(flaws[i].first->zone);
			flaws[i].second+=flaw_inside(flaws[i].first->zone);
			flaws[i].second+=flaw_border(flaws[i].first->zone);
//			flaws[i].second+=flaw_comp(flaws[i].first->zone);
			if(flaws[i].first->neighbours.size()) {
//				flaws[i].second+=flaw_smaller(flaws[i].first->zone,flaws[i].first->neighbours);
				flaws[i].second+=flaw_overlap(flaws[i].first->zone,flaws[i].first->neighbours);
			}
			flaws[i].second+=flaw_targets(flaws[i].first);
		}
	}
	part_t* sort_and_select_worst(flaws_t& flaws,const real_t prob=0.05,const real_t lambda=real_t(1.1)) {
		sort(begin(flaws),end(flaws),[](auto& a,auto& b){
			return a.second>b.second;
		});
		size_t n=random_exp(re,flaws.size(),prob,lambda);
		return flaws[n].first;
	}
	int improve(part_t& w) {
		T F1 = flaw_outside(w.zone);
		T F2 = flaw_inside(w.zone);
		T F3 = flaw_border(w.zone);
//		T F4 = 0;
		T F5 = 0;
//		T F6 = flaw_comp(w.zone);
		T F7 = 0;
		if(w.neighbours.size()) {
//			F4 = flaw_smaller(w.zone,w.neighbours);
			F5 = flaw_overlap(w.zone,w.neighbours);
		}
		T F=F1;
		F=max(F,F2);
		F=max(F,F3);
//		F=max(F,F4);
		F=max(F,F5);
//		F=max(F,F6);
		F=max(F,F7);
		if(F==F1) {
			w.zone.p*=real_t(0.9);
			w.zone.shake_and_check(re);
			return 1;
		}
		if(F==F2) {
			w.zone.p*=real_t(1.1);
			w.zone.shake_and_check(re);
			return 2;
		}
		if(F==F3) {
			T dx=(w.zone.p.x+w.zone.r)-T(BORDER_DISTANCE);
			if(dx<0) w.zone.p.x=abs(dx)+T(BORDER_DISTANCE);
			T dy=(w.zone.p.y+w.zone.r)-T(BORDER_DISTANCE);
			if(dy<0) w.zone.p.y=abs(dy)+T(BORDER_DISTANCE);
			w.zone.shake_and_check(re);
			return 3;
		}
		if(w.neighbours.size()) {
//			if(F==F4) return improve_smaller(w.zone,w.neighbours,re);
			if(F==F5) return improve_overlap(w.zone,w.neighbours,re);
		}
//		if(F==F6) {
//			improve_comp(w.zone);
//			w.zone.shake_and_check(re);
//			return 6;
//		}
		if(F==F7) return improve_targets(&w);
		return 0;
	}
	int one_step(flaws_t& flaws) {
		update_neighbours();
		update_flaws(flaws);
		auto w=sort_and_select_worst(flaws);
		return improve(*w);
	}
	real_t target_eveness(circle_t& cp) {
		point_t sum={0,0};
		for(auto& p:parts) {
			auto pwr=LED_power<T>(p.type);
			if(pwr>0) {
				sum+=energy_transfer(cp,p.zone,pwr);
			}
		}
		return sum.length();
	}
	real_t targets_evenness() {
		real_t tot=0;
		for(auto& t:targets) {
			auto c=t;
			tot+=target_eveness(c);
			c.p.x=-t.p.y;
			c.p.y=+t.p.x;
			tot+=target_eveness(c);
			c.p.x=-t.p.x;
			c.p.y=-t.p.y;
			tot+=target_eveness(c);
			c.p.x=+t.p.y;
			c.p.y=-t.p.x;
			tot+=target_eveness(c);
		}
		return tot;
	}
	real_t area_overlapping(part_t& ref) {
		long double sum=0;
		for(auto& p:parts) if(&ref!=&p) {
			sum+=area(ref.zone,p.zone);
		}
		return static_cast<T>(sum);
	}
	real_t areas_overlapping() {
		long double sum=0;
		for(auto& p:parts) sum+=area_overlapping(p);
		return static_cast<T>(sum);
	}
	size_t optimize(const size_t stp=1000,const size_t pinc=10000) {
		problem_board<T> pm;
		pm.brd=this;
		pagmo::problem p{pm};
		pagmo::vector_double storage;
		storage.resize(2*parts.size(),0);
		for(size_t n=0;n<parts.size();n++) {
			storage[2*n+0]=parts[n].zone.p.x;
			storage[2*n+1]=parts[n].zone.p.y;
		}
//		std::cout << "Fitness: " << p.fitness(storage)[0] << '\n';
//
//		// Fetch the lower/upper bounds for the first variable.
//		std::cout << "Lower bounds: [" << p.get_lb()[0] << "]\n";
//		std::cout << "Upper bounds: [" << p.get_ub()[0] << "]\n\n";
//
//		// Print p to screen.
//		std::cout << p << '\n';
		
		save_to_svg(true);
		pagmo::algorithm algo{pagmo::gaco(static_cast<unsigned>(stp),static_cast<unsigned>(parts.size()))};
		if(algo.has_set_verbosity()) {
			algo.set_verbosity(0);
		}
		save_to_svg(true);

		pagmo::population pop{p,4*parts.size()};

		pop = algo.evolve(pop);

		cout << "The population: \n" << pop;
		
		return 0;
	}
	T objective() {
		return targets_evenness();
	}
	size_t run_steps(const size_t stp=1000,const size_t pinc=10000) {
		cout << "running " << stp << " more steps" << endl;
		flaws_t flaws;
		get_flaws(flaws);
		save_to_svg();
		for(size_t n=0;n<stp;n++) {
			int f=one_step(flaws);
			long double sum_flaws=0;
			for(auto& f:flaws) sum_flaws+=f.second;
			if(resfile.good()) resfile << sum_flaws << CSV << f << CSV << parts.size() << endl;
			if(0==(n%pinc)) save_to_svg();
			if(0==sum_flaws) {
				cout << n+1 << " EO steps actually runed" << endl;
				optimize(stp,pinc);
				return n+1;
			}
		}
		cout << "all steps consumed" << endl;
		return stp;
	}
	void svg_circle(ostream& f,const real_t& x,const real_t& y,const real_t& ra,const char* fi="#bbbbbb",const char* st="#000000",const real_t w=0.25) {
		f << "<circle style=\"fill:" << fi << ";stroke:" << st << ";stroke-width:" << w << ";stroke-linecap:round;stroke-linejoin:round\" ";
		f << "cx=\"" << scale_x(x) << "\" cy=\"" << scale_y(y) << "\" r=\"" << scale(ra) << "\" />" << endl;
	}
	void svg_outline(ostream& f) {
		f << "<path id=\"World\" style=\"fill:#fafafa;stroke:#fafafa;stroke-width:0.25;stroke-linecap:round;stroke-linejoin:round\" d=\"";
		f << " M " << scale_x(+WORLD_SIZE) << " " << scale_y(-WORLD_SIZE);
		f << " L " << scale_x(+WORLD_SIZE) << " " << scale_y(+WORLD_SIZE);
		f << " L " << scale_x(-WORLD_SIZE) << " " << scale_y(+WORLD_SIZE);
		f << " L " << scale_x(-WORLD_SIZE) << " " << scale_y(-WORLD_SIZE);
		f << " Z\" />" << endl;
		f << "<path id=\"Xaxis\" style=\"fill:none;stroke:#556655;stroke-width:0.25;stroke-linecap:round;stroke-linejoin:round\" d=\"";
		f << " M " << scale_x(-OUTSIDE_RADIUS) << " " << scale_y(0);
		f << " L " << scale_x(+OUTSIDE_RADIUS) << " " << scale_y(0);
		f << " \" />" << endl;
		f << "<path id=\"Xaxis\" style=\"fill:none;stroke:#556655;stroke-width:0.25;stroke-linecap:round;stroke-linejoin:round\" d=\"";
		f << " M " << scale_x(0) << " " << scale_y(-OUTSIDE_RADIUS);
		f << " L " << scale_x(0) << " " << scale_y(+OUTSIDE_RADIUS);
		f << " \" />" << endl;
		svg_circle(f,T(0),T(0),T(1),"#aaaaaa","none",T(0));
		real_t a=atan2(real_t(BORDER_DISTANCE),real_t(OUTSIDE_RADIUS));
		real_t ax1=cos(a);
		real_t ay1=sin(a);
		a=atan2(real_t(BORDER_DISTANCE),real_t(INSIDE_RADIUS));
		real_t ax2=cos(a);
		real_t ay2=sin(a);
		f << "<path id=\"Border\" style=\"fill:#bbeebb;stroke:#bbeebb;stroke-width:0.25;stroke-linecap:round;stroke-linejoin:round\" d=\"";
		f << " M " << scale_x(ax1*OUTSIDE_RADIUS) << " " << scale_y(ay1*OUTSIDE_RADIUS);
		f << " A " << scale(OUTSIDE_RADIUS) << " " << scale(OUTSIDE_RADIUS) << " 0 0 0 " << scale_x(ay1*OUTSIDE_RADIUS) << " " << scale_y(ax1*OUTSIDE_RADIUS);
		f << " L " << scale_x(ay1*OUTSIDE_RADIUS) << " " << scale_y(ax2*INSIDE_RADIUS);
		f << " A " << scale(INSIDE_RADIUS) << " " << scale(INSIDE_RADIUS) << " 0 0 1 " << scale_x(ax2*INSIDE_RADIUS) << " " << scale_y(ay2*INSIDE_RADIUS);
		f << " Z\" />" << endl;
		for(auto& t:targets) svg_circle(f,t.p.x,t.p.y,t.r,"#cc0000","#000000",T(0.2));
	}
	void save_to_svg(const bool opt_stage=false,const bool show_ranges=false) {
		ostringstream fn;
		fn << "FRAME" << setfill('0') << setw(8) << filecounter << ".svg";
		ofstream f(fn.str());
		f << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>" << endl;
		f << "<svg xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:cc=\"http://creativecommons.org/ns#\" xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"  xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns=\"http://www.w3.org/2000/svg\" width=\"200mm\" height=\"200mm\" viewBox=\"0 0 1000 1000\" version=\"1.1\" id=\"eotest8\">" << endl;
		f << "<defs id=\"defs2\" />" << endl;
		f << "<metadata id=\"metadata5\">" << endl;
		f << "<rdf:RDF>" << endl;
		f << "<cc:Work rdf:about=\"\">" << endl;
		f << "<dc:format>image/svg+xml</dc:format>" << endl;
		f << "<dc:type rdf:resource=\"http://purl.org/dc/dcmitype/StillImage\" />" << endl;
		f << "<dc:title>FRAME</dc:title>" << endl;
		fn.clear();
		fn << "FRAME" << setfill('0') << setw(8) << filecounter;
		f << "<dc:identifier>" << fn.str() << "</dc:identifier>" << endl;
		f << "</cc:Work>" << endl;
		f << "</rdf:RDF>" << endl;
		f << "</metadata>" << endl;
		f << "<g id=\"layer1\">" << endl;
		svg_outline(f);
		if(show_ranges) {
			for(auto& p:parts) {
				T px=p.zone.p.x;
				T py=p.zone.p.y;
				size_t t=LED_range<T>(p.type);
				svg_circle(f,px,py,t,"none","#cccccc",0.4);
				svg_circle(f,-px,py,t,"none","#cccccc",0.4);
				svg_circle(f,px,-py,t,"none","#cccccc",0.4);
				svg_circle(f,-px,-py,t,"none","#cccccc",0.4);
			}
		}
		for(auto& p:parts) {
			T px=p.zone.p.x;
			T py=p.zone.p.y;
//			T rng=LED_range<T>(p.type);
			svg_circle(f,-py,px,p.zone.r,"none","#888888",0.4);
			svg_circle(f,-px,-py,p.zone.r,"none","#888888",0.4);
			svg_circle(f,py,-px,p.zone.r,"none","#888888",0.4);
		}
		for(auto& p:parts) p.save_to_svg(f);
		f << "<text x=\"" << scale_x(T(0.8*GRAPHICAL_SIZE)) << "\" y=\"" << scale_y(T(-0.9*GRAPHICAL_SIZE)) << "\">" << parts.size() << "</text>" << endl;
		if(opt_stage) {
			f << "<text x=\"" << scale_x(T(-0.8*GRAPHICAL_SIZE)) << "\" y=\"" << scale_y(T(-0.9*GRAPHICAL_SIZE)) << "\">" << "OPT" << "</text>" << endl;
		}
		f << "</g>" << endl;
		f << "</svg>" << endl;
		f << endl;
		f.close();
		filecounter++;
	}
};

/// Test segment area function by Monte Carlo integration
template<typename T,size_t N=90,size_t P=2000000> bool test_segmentarea(ostream* s=nullptr) {
	const T R=T(1);
	default_random_engine re;
	uint64_t sd=chrono::system_clock::now().time_since_epoch().count();
	re.seed(static_cast<default_random_engine::result_type>(sd));
	uniform_real_distribution<T> dist(-2*R,+2*R);
	for(size_t n=0;n<=N;n++) {
		size_t in=0;
		size_t ir=0;
		T d=-2*R+T(n)/T(N)*(T(4)*R);
		T As=segmentarea(R,d);
		for(size_t p=0;p<P;p++) {
			T x=dist(re);
			T y=dist(re);
			if(sqrt(sqr(x)+sqr(y))<R) {
				if(y>=d) in++;
				ir++;
			}
		}
		T Am=T(in)*M_PI*sqr(R)/T(ir);
		T relerr=(abs(As)>0 ? abs((Am-As)/As) : 0);
		if(s) {
			s->precision(20);
			*s << d << " " << As << " " << Am << " " << relerr << endl;
		}
		if(relerr>T(0.1)) return false;
	}
	return true;
}

int unit_tests(bool fileoutput=true) {
	ofstream* s=nullptr;
	if(fileoutput) {
		if(s && s->is_open()) {s->close(); delete s;}
		s=new ofstream;
		s->open("segmentarea_float.txt");
	}
	if(!test_segmentarea<float>(s)) return __LINE__;
	if(fileoutput) {
		if(s && s->is_open()) {s->close(); delete s;}
		s=new ofstream;
		s->open("segmentarea_double.txt");
	}
	if(!test_segmentarea<double>(s)) return __LINE__;
	if(fileoutput) {
		if(s && s->is_open()) {s->close(); delete s;}
		s=new ofstream;
		s->open("segmentarea_longdouble.txt");
	}
	if(!test_segmentarea<long double>(s)) return __LINE__;
	if(s && s->is_open()) {s->close(); delete s;}
	cout << "OK" << endl;
	return 0;
}

}
