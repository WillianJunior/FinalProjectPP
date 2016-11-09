#include <iostream>
#include <fstream>
#include <map>
#include <list>
#include <string>

#include <cmath>
#include <cstdlib>
#include <cfloat>
#include <sys/time.h>
#include <omp.h>

#include <boost/lockfree/queue.hpp>

using namespace std;

typedef int vertex_t;
typedef float weight_t;
typedef int index_t;
typedef float delta_t;
struct timeval start1, start2, start3, start4, start5, start6;
struct timeval end1, end2, end3, end4, end5, end6;
int tempo1, tempo2, tempo3, tempo4, tempo5, tempo6;

class edge_t {
public:
	vertex_t from;
	vertex_t to;
	weight_t weight;

	bool operator==(const edge_t& e) {
		if (this->from != e.from)
			return false;
		if (this->to != e.to)
			return false;
		if (this->weight != e.weight)
			return false;
		return true;
	}
};

typedef struct {
	vertex_t v;
	weight_t w;
} p_t;

void relax(const vertex_t &v, const weight_t &x, const delta_t& delta, 
	map<vertex_t, weight_t> &tent, map<index_t, list<vertex_t>> &B);

void printB(map<index_t, list<vertex_t>> B) {
	for (pair<index_t, list<vertex_t>> p : B) {
		cout << "index = " << p.first << ":" << endl;
		for (vertex_t v : p.second) {
			cout << "\t" << v << endl;
		}
	}
}

void printHL(map<vertex_t, list<edge_t>> hl) {
	for (pair<index_t, list<edge_t>> p : hl) {
		cout << "index = " << p.first << ":" << endl;
		for (edge_t e : p.second) {
			cout << "\t" << e.from << ":" << e.to << " = " << e.weight << endl;
		}
	}
}

void printTent(map<vertex_t, weight_t> tent) {
	for (pair<vertex_t, weight_t> p : tent) {
		cout << "0:" << p.first << " = " << p.second << endl;
	}
	cout << endl << "Tempos" << endl;
	cout << "Relax inicial: " << tempo1 << endl;
	cout << "For Req.emplace_back B: " << tempo2 << endl;
	cout << "For S.emplace_back: " << tempo3 << endl;
	cout << "For relax: " << tempo4 << endl;
	cout << "For Req.emplace_back S: " << tempo5 << endl;
	cout << "For relax final: " << tempo6 << endl;
	cout << "Total:" << (tempo1+tempo2+tempo3+tempo4+tempo5+tempo6) << endl;
}

string usage = "usage: ./serial.out [input_graph] [delta]";

int main (int argc, char** argv) {

	// get args
	if (argc != 3) {
		cout << usage << endl;
		return 0;
	}

	// get delta
	delta_t delta = stof(argv[2]) == 0 ? 0.0001 : stof(argv[2]);

	// get graph
	ifstream adj_list_file;
	adj_list_file.open(argv[1], ios::in);

	list<edge_t> E;
	string line;
	while (getline(adj_list_file, line)) {
		// continues if the lis isn't form an edge
		if (line.front() != 'a')
			continue;

		// get from value
		size_t i = line.find_first_of(" ");
		line = line.substr(i+1);
		i = line.find_first_of(" ");
		edge_t e;
		e.from = stoi(line.substr(0,i));

		// get to value
		line = line.substr(i+1);
		i = line.find_first_of(" ");
		e.to = stoi(line.substr(0,i));

		// get weight value
		size_t j = i+1;
		line = line.substr(i+1);
		e.weight = stof(line.substr(0,line.size()));

		E.emplace_back(e);
	}
	adj_list_file.close();

	// for (edge_t e : E) {
	// 	cout << e.from << ":" << e.to << " = " << e.weight << endl;
	// }

	// gen heavy and light
	map<vertex_t, list<edge_t>> heavy;
	map<vertex_t, list<edge_t>> light;
	for (edge_t e : E) {
		if (e.weight > delta)
			heavy[e.from].emplace_back(e);
		else
			light[e.from].emplace_back(e);
	}

	// get max heavy and light elements per list
	int max_light_size = 0;
	for (pair<vertex_t, list<edge_t>> p : light)
		if (max_light_size < p.second.size())
			max_light_size = p.second.size();
	int max_heavy_size = 0;
	for (pair<vertex_t, list<edge_t>> p : heavy)
		if (max_heavy_size < p.second.size())
			max_heavy_size = p.second.size();

	// cout << "heavy:" << endl;
	// printHL(heavy);
	// cout << endl << "light:" << endl;
	// printHL(light);
	tempo1 = tempo2 = tempo3 = tempo4 = tempo5 = tempo6 = 0;
	vertex_t s = 1; // TODO: set start

	// gen tent
	map<vertex_t, weight_t> tent;
	for (edge_t e : E) {
		tent[e.to] = FLT_MAX;
	}
	tent[s] = FLT_MAX;

	// gen B
	map<index_t, list<vertex_t>> B;

	gettimeofday(&start1, NULL);
	relax(s, 0, delta, tent, B);
	index_t i = 0;
	gettimeofday(&end1, NULL);
	tempo1 = ((end1.tv_sec*1000000+end1.tv_usec) - (start1.tv_sec*1000000+start1.tv_usec));

	while (B.size() != 0) {
		// cout << "S = 0" << endl;
		list<vertex_t> S;

		// cout << "while B[i] != {}" << endl;
		while (B[i].size() != 0) {
			// cout << "Req = {(w; tent(v) + c(v,w)) | v in B[i] and (v,w) in light(v)}" << endl;
			boost::lockfree::queue<p_t> Req(B[i].size()*(1+max_light_size));

			gettimeofday(&start2, NULL);

			#ifdef LIGHT_PAR
			#pragma omp parallel 
			{
			#pragma omp single 
			{
			#endif

			for (vertex_t v : B[i]) {
				#if defined(GRANULARITY_1) && defined(LIGHT_PAR)
				#pragma omp task
				{
				#endif
				for (edge_t e2 : light[v]) {
					#if defined(GRANULARITY_0) && defined(LIGHT_PAR)
					#pragma omp task
					{
					#endif

					weight_t weight = tent[v] + e2.weight;
					p_t tmp;
					tmp.v = e2.to;
					tmp.w = weight;
					Req.push(tmp);

					#if defined(GRANULARITY_0) && defined(LIGHT_PAR)
					}
					#endif
				}
				#if defined(GRANULARITY_1) && defined(LIGHT_PAR)
				}
				#endif
			}

			#ifdef LIGHT_PAR
			}
			}
			#endif

			gettimeofday(&end2, NULL);
			tempo2 = tempo2 + ((end2.tv_sec*1000000+end2.tv_usec) - (start2.tv_sec*1000000+start2.tv_usec));
			// cout << "S = S ++ B[i]; B[i] = {}" << endl;

			gettimeofday(&start3, NULL);
			for (list<vertex_t>::const_iterator v=B[i].cbegin(); v!=B[i].cend();) {
				S.emplace_back(*v);
				v = B[i].erase(v);
			}
			gettimeofday(&end3, NULL);
			tempo3 = tempo3 + ((end3.tv_sec*1000000+end3.tv_usec) - (start3.tv_sec*1000000+start3.tv_usec));
			// cout << "for each (v,x) in Req do relax(v,x)" << endl;
			
			gettimeofday(&start4, NULL);
			while (!Req.empty()) {
				p_t p;
				Req.pop(p);
				relax(p.v, p.w, delta, tent, B);
			}
			gettimeofday(&end4, NULL);
			tempo4 = tempo4 + ((end4.tv_sec*1000000+end4.tv_usec) - (start4.tv_sec*1000000+start4.tv_usec));
		}

		// cout << "Req = {(w; tent(v) + c(v,w)) | v in S and (v,w) in heavy(v)}" << endl;
		boost::lockfree::queue<p_t> Req(B[i].size()*(1+max_heavy_size));
		
		gettimeofday(&start5, NULL);

		#ifdef HEAVY_PAR
		#pragma omp parallel 
		{
		#pragma omp single 
		{
		#endif

		for (vertex_t v : S) {
			
			#if defined(GRANULARITY_1) && defined(HEAVY_PAR)
			#pragma omp task
			{
			#endif

			for (edge_t e2 : heavy[v]) {
				
				#if defined(GRANULARITY_0) && defined(HEAVY_PAR)
				#pragma omp task
				{
				#endif

				weight_t weight = tent[v] + e2.weight;
				p_t tmp;
				tmp.v = e2.to;
				tmp.w = weight;
				Req.push(tmp);

				#if defined(GRANULARITY_0) && defined(HEAVY_PAR)
				}
				#endif
			}

			#if defined(GRANULARITY_1) && defined(HEAVY_PAR)
			}
			#endif
		}

		#ifdef HEAVY_PAR
		}
		}
		#endif


		gettimeofday(&end5, NULL);
		tempo5 = tempo5 + ((end5.tv_sec*1000000+end5.tv_usec) - (start5.tv_sec*1000000+start5.tv_usec));
		
		gettimeofday(&start6, NULL);
		// cout << "for each (v,x) in Req do relax(v,x)" << endl;
		while (!Req.empty()) {
			p_t p;
			Req.pop(p);
			relax(p.v, p.w, delta, tent, B);
		}
		gettimeofday(&end6, NULL);
		tempo6 = tempo6 + ((end6.tv_sec*1000000+end6.tv_usec) - (start6.tv_sec*1000000+start6.tv_usec));
		
		B.erase(i);
		i++;
		// printTent(tent);
	}

	printTent(tent);
}

void relax(const vertex_t &v, const weight_t &x, const delta_t& delta, 
	map<vertex_t, weight_t> &tent, map<index_t, list<vertex_t>> &B) {

	if (x < tent[v]) {
		if (B.find(floor(tent[v]/delta)) != B.end())
			B[floor(tent[v]/delta)].remove(v);
		B[floor(x/delta)].emplace_back(v);
		tent[v] = x;
	}
}