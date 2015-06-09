#include "../comb.h"

#include <algorithm>
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <deque>
#include <vector>

struct Arc {
  unsigned weight :  1;
  int vertex      : 31;
};

typedef int32_t Index;
#define SCN_IDX SCNi32
#define PRI_IDX PRIi32

struct Edge {
  Index src;
  Arc dst;
};

int less(const Edge lhs, const Edge rhs) {
  int64_t rv = lhs.src - rhs.src;
  rv = rv ? rv : lhs.dst.vertex - rhs.dst.vertex;
  rv = rv ? rv : lhs.dst.weight - rhs.dst.weight;
  return rv < 0;
}

bool equal(const Edge lhs, const Edge rhs) {
  return lhs.src == rhs.src &&
         lhs.dst.vertex == rhs.dst.vertex;
}

typedef std::vector<Edge> RawEdges;
typedef Comb<Index, Arc> Graph;

void compactify_to(const RawEdges& raw, Index n, Graph* g) {
  Index src = 0;
  for (const Edge e : raw) {
    for (; src != e.src; ++src)
      g->end_row();
    g->append(e.dst);
  }

  for (; src != n; ++src)
    g->end_row();
}

void read_graph(FILE* in, Index n, Index m, Graph* g) {
  RawEdges edges(2 * m);
  for (size_t i = 0; i != m; ++i) {
    Index src, dst;
    unsigned weight;
    fscanf(in, "%" SCN_IDX "%" SCN_IDX "%u", &src, &dst, &weight);
    --src; --dst;
    edges[2 * i] = {src, {weight, dst}};
    edges[2 * i + 1] = {dst, {weight, src}};
  }

  std::sort(edges.begin(), edges.end(), less);
  edges.erase(std::unique(edges.begin(), edges.end(), equal),
              edges.end());

  g->reserve(n, edges.size());
  compactify_to(edges, n, g);
}

Index min_distance(const Graph& g, Index start, Index end) {
  std::deque<Index> front;
  front.push_back(start);

  std::vector<Index> dist(g.rows(), g.rows());
  dist[start] = 0;

  while (!front.empty()) {
    Index v = front.front(); front.pop_front();

    for (Index u = 0; u != g.length(v); ++u) {
      const Arc dst = g.at(v, u);
      if (dist[dst.vertex] > dist[v] + dst.weight) {
        dist[dst.vertex] = dist[v] + dst.weight;
        if (dst.weight)
          front.push_back(dst.vertex);
        else
          front.push_front(dst.vertex);
      }
    }
  }

  return dist[end];
}

int main(void) {
  Index n, m, a, b;
  scanf("%" SCN_IDX "%" SCN_IDX "%" SCN_IDX "%" SCN_IDX,
        &n, &m, &a, &b);
  --a; --b;

  Graph g;
  read_graph(stdin, n, m, &g);

  const Index distance = min_distance(g, a, b);
  if (distance == n)
    puts("-1");
  else
    printf("%" PRI_IDX "\n", distance);

  return 0;
}
