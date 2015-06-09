#include <cinttypes>
#include <cstdio>
#include <deque>
#include <list>
#include <map>
#include <vector>


struct graph {
    size_t vertex_count;
    size_t edges_count;
    std::vector<std::map<size_t, int>> edges;

    graph(size_t n, size_t m)
        : vertex_count(n), edges_count(m), edges(n) { }
};

void read_graph(FILE* in, graph* g) {
    for (size_t i = 0; i < g->edges_count; ++i) {
        size_t s, e;
        int w;
        fscanf(in, "%zu%zu%d", &s, &e, &w);
        --s;
        --e;

        if (!g->edges[s].count(e) || w < g->edges[s][e])
            g->edges[s][e] = g->edges[e][s] = w;
    }
}


int min_distance(const graph& g, size_t start, size_t end) {
    std::deque<size_t> frontier;
    frontier.push_back(start);

    int inf_dist = g.edges_count + 1;
    std::vector<int> dist(g.vertex_count, inf_dist);
    dist[start] = 0;

    while (!frontier.empty()) {
        auto v = frontier.front();
        frontier.pop_front();

        for (auto& u : g.edges[v])
            if (dist[u.first] > dist[v] + u.second) {
                dist[u.first] = dist[v] + u.second;

                if (u.second)
                    frontier.push_back(u.first);
                else
                    frontier.push_front(u.first);
            }
    }

    if (dist[end] < inf_dist)
        return dist[end];

    return -1;
}


int main() {
    uint32_t n, m, a, b;
    scanf("%" SCNu32 "%" SCNu32 "%" SCNu32 "%" SCNu32,
          &n, &m, &a, &b);

    graph g(n, m);
    read_graph(stdin, &g);

    printf("%d", min_distance(g, a - 1, b - 1));

    return 0;
}
