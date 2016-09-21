#include <iostream>
#include <map>
#include <list>
#include <cmath>

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

void relax(const vertex_t &v, const weight_t &x, const delta_t& delta, map<vertex_t, weight_t> &tent, map<index_t, list<vertex_t>> &B);

int main () {
	// get graph

	list<edge_t> E;

	delta_t delta = 0.5;

	// gen heavy
	map<vertex_t, list<edge_t>> heavy;

	// gen light
	map<vertex_t, list<edge_t>> light;

	// gen tent
	map<vertex_t, weight_t> tent;

	// gen B
	map<index_t, list<vertex_t>> B;

	vertex_t s = 0; // TODO: set start
	relax(s, 0, delta, tent, B);
	index_t i = 0;

	while (B.size() != 0) {
		list<vertex_t> S;
		list<pair<vertex_t, weight_t>> Req;

		// while B[i] != {}
		while (B[i].size() != 0) {
			// Req = {(w; tent(v) + c(v,w)) | v in B[i] and (v,w) in light(v)}
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
			for (vertex_t v : B[i]) {
				S.emplace_back(v);
				B[i].remove(v);
			}

			// for each (v,x) in Req do relax(v,x)
			for (pair<vertex_t, weight_t> p : Req) {
				relax(p.first, p.second, delta, tent, B);
			}
		}

		// Req = {(w; tent(v) + c(v,w)) | v in S and (v,w) in heavy(v)}
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
		for (pair<vertex_t, weight_t> p : Req) {
			relax(p.first, p.second, delta, tent, B);
		}

		i++;
	}
}

void relax(const vertex_t &v, const weight_t &x, const delta_t& delta, map<vertex_t, weight_t> &tent, map<index_t, list<vertex_t>> &B) {
	if (x < tent[v]) {
		B[floor(tent[v]/delta)].remove(v);
		B[floor(x/delta)].emplace_back(v);
		tent[v] = x;
	}
}