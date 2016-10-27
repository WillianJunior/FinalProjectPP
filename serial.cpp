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

// dynamic sized array in a struct
typedef struct {
	int last=-1;
	edge_t edges[];
} edge_s;

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
}

string usage = "usage: ./serial.out <input_graph> <delta> <number of vertices>";

int main (int argc, char** argv) {
	
	// get args
	if (argc != 4) {
		cout << usage << endl;
		return 0;
	}

	// get delta
	delta_t delta = stof(argv[2]) == 0 ? 0.0001 : stof(argv[2]);

	// get number of vertices
	int n_vertices = stoi(argv[3]);

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
		e.weight = stof(line.substr(0,line.size()));

		E.emplace_back(e);
	}
	adj_list_file.close();

	// for (edge_t e : E) {
	// 	cout << e.from << ":" << e.to << " = " << e.weight << endl;
	// }

	// gen heavy and light
	map<vertex_t, list<edge_t>> heavy_m;
	map<vertex_t, list<edge_t>> light;
	
	int edges_count[n_vertices];
	for (int i=0; i<n_vertices; i++)
		edges_count[i] = 0;

	for (edge_t e : E) {
		edges_count[e.from]++;
		if (e.weight > delta)
			heavy_m[e.from].emplace_back(e);
		else
			light[e.from].emplace_back(e);
	}

	// cout << "heavy:" << endl;
	// printHL(heavy);
	// cout << endl << "light:" << endl;
	// printHL(light);

	vertex_t s = 0; // TODO: set start

	// gen tent
	map<vertex_t, weight_t> tent;
	for (edge_t e : E) {
		tent[e.to] = FLT_MAX;
	}
	tent[s] = FLT_MAX;

	// gen B
	map<index_t, list<vertex_t>> B;

	relax(s, 0, delta, tent, B);
	index_t i = 0;

	// get max number of edges of all nodes
	int max_edges = 0;
	for (int i=0; i<n_vertices; i++)
		if (edges_count[i] > max_edges)
			max_edges = edges_count[i];

	// convert heavy and light to arrays
	edge_s* heavy = (edge_s*)malloc(n_vertices * (sizeof(edge_s) + max_edges));
	for (int i=0; i<n_vertices; i++) {
		heavy[i].last = -1;
	}

	for (pair<vertex_t, list<edge_t>> p : heavy_m) {
		for (edge_t e : p.second) {
			heavy[p.first].last++;
			heavy[p.first].edges[heavy[p.first].last] = e;
		}
	}


	while (B.size() != 0) {
		cout << "S = 0" << endl;
		list<vertex_t> S;

		cout << "while B[i] != {}" << endl;
		while (B[i].size() != 0) {
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

			cout << "S = S ++ B[i]; B[i] = {}" << endl;
			for (list<vertex_t>::const_iterator v=B[i].cbegin(); v!=B[i].cend();) {
				S.emplace_back(*v);
				v = B[i].erase(v);
			}

			cout << "for each (v,x) in Req do relax(v,x)" << endl;
			for (pair<vertex_t, weight_t> p : Req) {
				relax(p.first, p.second, delta, tent, B);
			}
		}

		cout << "Req = {(w; tent(v) + c(v,w)) | v in S and (v,w) in heavy(v)}" << endl;
		list<pair<vertex_t, weight_t>> Req;
		for (vertex_t v : S) {
			for (edge_t e1 : E) {
				for (int i=0; i<heavy[v].last; i++) {
					if (e1 == heavy[v].edges[i]) {
						weight_t weight = tent[v] + e1.weight;
						Req.emplace_back(pair<vertex_t, weight_t>(heavy[v].edges[i].to, weight));
					}
				}
			}
		}

		cout << "for each (v,x) in Req do relax(v,x)" << endl;
		for (pair<vertex_t, weight_t> p : Req) {
			relax(p.first, p.second, delta, tent, B);
		}

		B.erase(i);
		i++;
		printTent(tent);
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