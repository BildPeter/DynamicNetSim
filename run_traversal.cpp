// run_traversal.cpp

/**
*    Copyright (C) 2012 Mario Konschake
*
*    This program is free software: you can redistribute it and/or  modify
*    it under the terms of the GNU Affero General Public License, version 3,
*    as published by the Free Software Foundation.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU Affero General Public License for more details.
*
*    You should have received a copy of the GNU Affero General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* This file illustrates example usage of time-sensitive traversal of a graph
 * 
 * Input:
 *  - edgelist in the form 'source target day\n' with all values integer
 *  - node list with 'nodeID\n' with integer ID of the indexnodes to be considered
 *  - infectious period aka the maximum permitted waiting time on node before traversal 
 *  - indexday to start the the traversal
 * 
 * Outut:
 *  -file in the form 'indexnodeID out-component-size' for all indexnodes with non-vanishing out-component
 * 
 * This algorithm does not permit same-day traversal after visit, i.e. for a node visited today
 * outbound traversal is allowed only the day after
 * 
 * This algorithm assumes node recoverey to happen before the day starts, i.e. before the traversal
 */

#include <fstream>
#include "read.h"
#include "traversal.h"
#include <iostream>

using namespace std;

int main()
{
  const int INFECTIOUS_PERIOD = 5;
  const int INDEXDAY = 102;
  const char* OUTPUT_FILENAME = "cTraversal_result.txt";
  
  const char* EDGE_FILENAME = "/Users/sonneundasche/Programmierung/Extern/DynamicNetSim/edges_example.txt"; //put yours here
  const char* NODE_FILENAME = "/Users/sonneundasche/Programmierung/Extern/DynamicNetSim/nodes_example.txt";
  
  DAY_EDGES edges = read_edges(EDGE_FILENAME);
  NODE_SET index_nodes = read_nodes(NODE_FILENAME);
  
  ofstream file;
  file.open (OUTPUT_FILENAME);
  
  for (NODE_SET::const_iterator n = index_nodes.begin(); n != index_nodes.end(); ++n)
  {
    NODE_SET recovered = traverse(edges, *n, INDEXDAY, INFECTIOUS_PERIOD);
    
    if (!recovered.empty())
      file << *n << " " << recovered.size() << "\n";
  }
  
  file.close();
  return 0;
}
