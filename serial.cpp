#include <iostream>
#include <fstream>
#include <map>
#include <list>
#include <string>

#include <cmath>
#include <cstdlib>
#include <cfloat>

using namespace std;

typedef int vertex_t;
typedef float weight_t;
typedef int index_t;
typedef float delta_t;

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

void relax(const vertex_t &v, const weight_t &x, const delta_t& delta, 
	map<vertex_t, weight_t> &tent, map<index_t, list<vertex_t>> &B);

string usage = "usage: ./serial.out [input_graph] [delta]";

int main (int argc, char** argv) {
	
	// get args
	if (argc != 3) {
		cout << usage << endl;
		return 0;
	}

	// get delta
	delta_t delta = stof(argv[2]);

	// get graph
	ifstream adj_list_file;
	adj_list_file.open(argv[1], ios::in);

	list<edge_t> E;
	string line;
	while (getline(adj_list_file, line)) {
		// get from value
		size_t i = line.find_first_of(" ");
		edge_t e;
		e.from = stoi(line.substr(0,i));

		// get to value
		line = line.substr(i+1);
		i = line.find_first_of(" ");
		e.to = stoi(line.substr(0,i));

		// get weight value
		size_t j = i+1;
		line = line.substr(i+1);
		e.weight = stof(line.substr(0,line.size()-1));

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

	// gen tent
	map<vertex_t, weight_t> tent;
	for (edge_t e : E)
		tent[e.from] = FLT_MAX;

	// gen B
	map<index_t, list<vertex_t>> B;

	vertex_t s = 0; // TODO: set start
	relax(s, 0, delta, tent, B);
	index_t i = 0;

	while (B.size() != 0) {
		list<vertex_t> S;
		cout << "S = 0" << endl;

		// while B[i] != {}
		cout << "while B[i] != {}" << endl;
		while (B[i].size() != 0) {
			// Req = {(w; tent(v) + c(v,w)) | v in B[i] and (v,w) in light(v)}
			cout << "Req = {(w; tent(v) + c(v,w)) | v in B[i] and (v,w) in light(v)}" << endl;
			list<pair<vertex_t, weight_t>> Req;
			for (vertex_t v : B[i]) {
				for (edge_t e1 : E) {
					for (edge_t e2 : light[v]) {
						if (e1 == e2) {
							weight_t weight = tent[v] + e1.weight;
							Req.emplace_back(pair<vertex_t, weight_t>(e2.to, weight));
						}
					}
				}
			}

			// S = S ++ B[i]
			// B[i] = {}
			cout << "S = S ++ B[i]; B[i] = {}" << endl;
			for (vertex_t v : B[i]) {
				S.emplace_back(v);
				B[i].remove(v);
			}

			// for each (v,x) in Req do relax(v,x)
			cout << "for each (v,x) in Req do relax(v,x)" << endl;
			for (pair<vertex_t, weight_t> p : Req) {
				relax(p.first, p.second, delta, tent, B);
			}
		}

		// Req = {(w; tent(v) + c(v,w)) | v in S and (v,w) in heavy(v)}
		cout << "Req = {(w; tent(v) + c(v,w)) | v in S and (v,w) in heavy(v)}" << endl;
		list<pair<vertex_t, weight_t>> Req;
		for (vertex_t v : S) {
			for (edge_t e1 : E) {
				for (edge_t e2 : heavy[v]) {
					if (e1 == e2) {
						weight_t weight = tent[v] + e1.weight;
						Req.emplace_back(pair<vertex_t, weight_t>(e2.to, weight));
					}
				}
			}
		}

		// for each (v,x) in Req do relax(v,x)
		cout << "for each (v,x) in Req do relax(v,x)" << endl;
		for (pair<vertex_t, weight_t> p : Req) {
			relax(p.first, p.second, delta, tent, B);
		}

		i++;
	}

	for (pair<vertex_t, weight_t> p : tent) {
		cout << "0:" << p.first << " = " << p.second << endl;
	}
}

void relax(const vertex_t &v, const weight_t &x, const delta_t& delta, 
	map<vertex_t, weight_t> &tent, map<index_t, list<vertex_t>> &B) {

	if (x < tent[v]) {
		B[floor(tent[v]/delta)].remove(v);
		B[floor(x/delta)].emplace_back(v);
		tent[v] = x;
	}
}