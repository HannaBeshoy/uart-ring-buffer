#include <limits.h>
#include "demos.h"

void graph_init(graph_t *g, int n)
{
    g->n = n;
    for (int i = 0; i < G_MAX; i++) for (int j = 0; j < G_MAX; j++) g->w[i][j] = 0;
}

void graph_add_edge(graph_t *g, int a, int b, int weight) { g->w[a][b] = g->w[b][a] = weight; }

int graph_bfs(const graph_t *g, int start, int *order)
{
    int queue[G_MAX], head = 0, tail = 0, n = 0;
    bool seen[G_MAX] = { false };
    queue[tail++] = start; seen[start] = true;
    while (head < tail) {
        int v = queue[head++];
        order[n++] = v;
        for (int u = 0; u < g->n; u++)
            if (g->w[v][u] && !seen[u]) { seen[u] = true; queue[tail++] = u; }
    }
    return n;
}

static void dfs(const graph_t *g, int v, bool *seen, int *order, int *n)
{
    seen[v] = true;
    order[(*n)++] = v;
    for (int u = 0; u < g->n; u++) if (g->w[v][u] && !seen[u]) dfs(g, u, seen, order, n);
}

int graph_dfs(const graph_t *g, int start, int *order)
{
    bool seen[G_MAX] = { false };
    int n = 0;
    dfs(g, start, seen, order, &n);
    return n;
}

bool graph_has_euler_circuit(const graph_t *g)
{
    int order[G_MAX], first = -1, with_edges = 0;
    for (int v = 0; v < g->n; v++) {
        int degree = 0;
        for (int u = 0; u < g->n; u++) if (g->w[v][u]) degree++;
        if (degree % 2) return false;
        if (degree) { with_edges++; if (first < 0) first = v; }
    }
    if (first < 0) return true;
    int reached = graph_bfs(g, first, order);   /* isolated vertices do not matter */
    return reached == with_edges;
}

bool graph_is_bipartite(const graph_t *g)
{
    int colour[G_MAX], queue[G_MAX];
    for (int v = 0; v < g->n; v++) colour[v] = -1;
    for (int s = 0; s < g->n; s++) {
        if (colour[s] >= 0) continue;
        int head = 0, tail = 0;
        colour[s] = 0; queue[tail++] = s;
        while (head < tail) {
            int v = queue[head++];
            for (int u = 0; u < g->n; u++) {
                if (!g->w[v][u]) continue;
                if (colour[u] < 0) { colour[u] = 1 - colour[v]; queue[tail++] = u; }
                else if (colour[u] == colour[v]) return false;
            }
        }
    }
    return true;
}

void graph_dijkstra(const graph_t *g, int source, int *dist)
{
    bool done[G_MAX] = { false };
    for (int v = 0; v < g->n; v++) dist[v] = INT_MAX;
    dist[source] = 0;
    for (int k = 0; k < g->n; k++) {
        int v = -1;
        for (int u = 0; u < g->n; u++) if (!done[u] && dist[u] != INT_MAX && (v < 0 || dist[u] < dist[v])) v = u;
        if (v < 0) break;
        done[v] = true;
        for (int u = 0; u < g->n; u++)
            if (g->w[v][u] && !done[u] && dist[v] + g->w[v][u] < dist[u]) dist[u] = dist[v] + g->w[v][u];
    }
}
