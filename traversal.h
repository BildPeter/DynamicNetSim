#ifndef GUARD_traversal_h
#define GUARD_traversal_h

#include <vector>
#include <map>
#include <set>

//edges
typedef std::pair <int, int> EDGE;
typedef std::vector<EDGE> EDGES;
typedef std::map<int, EDGES> DAY_EDGES;

//nodes
typedef std::set<int> NODE_SET;

//BFS-like time-sensitive traversal of edges
NODE_SET traverse(const DAY_EDGES&, int, int, int);

#endif
