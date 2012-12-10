// sets.cpp

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

//definition of common set operations

#include <algorithm>
#include "sets.h"
 
using namespace std;

set<int> difference(const set<int>& s1, const set<int>& s2)
{
  set<int> diff;
  set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(), inserter(diff, diff.end()));
  return diff;
}

set<int> myunion(const set<int>& s1, const set<int>& s2)
{
  set<int> u;
  set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), inserter(u, u.begin()));
  return u;
}

