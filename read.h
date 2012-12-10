#ifndef GUARD_read_h
#define GUARD_read_h

#include <vector>
#include <map>
#include <set>

//edges
typedef std::pair <int, int> EDGE;
typedef std::vector<EDGE> EDGES;
typedef std::map<int, EDGES> DAY_EDGES;

//nodes
typedef std::set<int> NODE_SET;

//read nodes from file into NODE_SET
//nodeIDs need to be int, one ID per line
NODE_SET read_nodes(const char*);

//read space delimited edges 'source target day'
//into map<day, vector< pair <source, target> > >
//all values need to be integer
DAY_EDGES read_edges(const char*);



#endif 
