// read.cpp

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

//handling file input


#include <fstream>
#include <algorithm>
#include "read.h"

using namespace std;


//predicate functions for 'split'
bool space (char c){
  return isspace(c); }

bool not_space (char c) {
  return !isspace(c); }


//splits a string a spaces, returns words in vector
vector<string> split(const string& line)
{
  string word;
  vector<string> words;
  typedef string::const_iterator iter;
  
  iter i = line.begin();
  while (i != line.end())
  {
    i = find_if(i, line.end(), not_space);
    iter j = find_if(i, line.end(), space);
    if (i != line.end())
    {
      word.assign(i,j);
      words.push_back(word);
    }
    i = j;
  }
  return words;
}


//converts a string of 3 space delimited
//integer into an array of int
void convert(string sline, int numbers[])
{
  vector<string> words = split(sline);
  numbers[0] = atoi(words[0].c_str());
  numbers[1] = atoi(words[1].c_str());
  numbers[2] = atoi(words[2].c_str());
}
 
 
//read nodes from file into NODE_SET
//nodeIDs need to be int, one ID per line
NODE_SET read_nodes(const char* fname)
{
  NODE_SET nodes;
  string line;
      
  ifstream file (fname);
  while (file.good())
  {
     getline (file, line); //line as string
     if (line.size() > 1) //avoid weird last line
      nodes.insert(atoi(line.c_str()));
  }
  return nodes;
}
 
    
//read space delimited edges 'source target day'
//into map<day, vector< pair <source, target> > >
//all values need to be integer
DAY_EDGES read_edges(const char* fname)
{
  DAY_EDGES edges;
  
  ifstream file (fname);
  while (file.good())
  {
    string sline;
    getline (file, sline);
    if (sline.size() > 1) //avoids corrupted last line
    {
      int line[3];
      convert(sline, line);
      EDGE e = make_pair(line[0], line[1]);
      edges[line[2]].push_back(e);
    }
  }
  return edges;
}
