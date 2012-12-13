// traversal.cpp

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

/* BFS-like traversal of edges of a graph defined be 'edges'
 * temporal sequence of edges respected traversal and only permitted if own visit not more than k timesteps in the past
 * 
 * Input:
 *  - edgelist as definied in the header
 *  - indexnode, indexday and infectious period
 * 
 * Outut:
 *  -set of all ever-infected nodes
 * 
 * This algorithm does not permit same-day traversal after visit, i.e. for a node visited today
 * outbound traversal is allowed only the day after
 * 
 * This algorithm assumes node recoverey to happen before the day starts, i.e. before the traversal
 */

#include <algorithm>
#include "sets.h"
#include "traversal.h"

using namespace std;

typedef map<int, NODE_SET> RDAY_MAP;

//iterators
typedef EDGES::const_iterator EDGES_ITER;
typedef RDAY_MAP::iterator RDAY_ITER;
typedef NODE_SET::const_iterator NODE_ITER;



//get max key of map
int max_key(const DAY_EDGES& edges)
{ return max_element(edges.begin(), edges.end())->first; }


//return newly infected nodes for 'day'
NODE_SET infect(const DAY_EDGES& edges, const NODE_SET& infectious, const NODE_SET& recovered, int day)
{
  NODE_SET new_infectious;
  
  DAY_EDGES::const_iterator it = edges.find(day);
  if (it != edges.end())
  {
    for (EDGES_ITER b = (*it).second.begin(); b != (*it).second.end(); ++b)
    {
      int u = (*b).first; //source
      int v = (*b).second; //target
      
      //infections happening here
      //add if source is infected and target
      //not already in recovered state
      NODE_ITER it = infectious.find(u);
      if (it != infectious.end())
      {
        if (!recovered.count(v))
          new_infectious.insert(v);
      }
    }
  }
  return new_infectious;
}


//BFS-like traversal of edges
//temporal sequence of edges respected
//traversal only permitted if own visit not more than k timesteps in the past
//same-day traversal not permitted
NODE_SET traverse(const DAY_EDGES& edges, int inode, int iday, int k)
{
  NODE_SET infectious, recovered;
  RDAY_MAP recovery; //to store map<day, set<'nodes to recover this day'> >
    
  //init
  infectious.insert(inode);
  recovery[iday +k].insert(inode);
  
  int lastday = max_key(edges) + k ;
  for (int day = iday + 1; day <= lastday; day++)
  {     
    
    //infection
    if (!infectious.empty())
    {
      NODE_SET new_infectious = infect(edges, infectious, recovered, day);
      infectious = myunion(infectious, new_infectious);
      recovery[day+k].insert(new_infectious.begin(), new_infectious.end());
    }
    
    //recovery
    RDAY_ITER it = recovery.find(day);
    if (it != recovery.end())
    {
      infectious = difference(infectious, (*it).second);
      recovered = myunion(recovered, (*it).second);
    }
    
  }
  
  //do not count index node
  recovered.erase(inode);
  return recovered;
}
