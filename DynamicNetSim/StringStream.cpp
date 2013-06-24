//
//  StringStream.cpp
//  DynamicNetSim
//
//  Created by Peter A. Kolski (BildPeter Visuals) on 15.04.13.
//  Copyright (c) 2013 Peter A. Kolski.
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  For the GNU General Public License see <http://www.gnu.org/licenses/>.
//
//
//

#include <iostream>
#include <sstream>  

#include "temporal_graph_handler.h"
#include <boost/unordered_map.hpp>
#include <lemon/list_graph.h>
#include <lemon/lgf_reader.h>

using namespace std;
using namespace lemon;

int main(){


    string a = "88\t567 999\t444 555\n22\t777 666\t111 000";
    string  file, line;
    int     out1, out2;
    
    istringstream   mStream( a );
//    ifstream mStream( "/Users/sonneundasche/Dropbox/FLI/04_HIT_Transform/_node size/Schwein_BL_07_time_tmpArcIDs_amountOnArc.txt" );
//    getline( mStream, file, '\n');
    
    while(getline( mStream, file, '\n')){
        istringstream lineStream( file );
        lineStream  >> out1;
        cout << "time: " << out1 << "\n";
        getline( lineStream, line, '\t');
        
        while( getline( lineStream, line, '\t') ){
            istringstream pairStream( line );
            pairStream >> out1;
            pairStream >> out2;
            cout << out1 << " : " << out2 << "\n";
            }
        }
    
//    
    vector< int >  activeTimes;
    ListDigraph   mGraph;
    digraphReader( mGraph, "/Users/sonneundasche/Dropbox/FLI/04_HIT_Transform/_node size/Schwein_BL_07.lgf")
    .run();

    
    map< int,  vector< pair <ListDigraph::Arc, int > > >  activeArcsAndWeight;
    
    activeTimes = tempGR::readTemporalArcListWeighted(mGraph, activeArcsAndWeight, "/Users/sonneundasche/Dropbox/FLI/04_HIT_Transform/_node size/Schwein_BL_07_time_tmpArcIDs_amountOnArc.txt");
    
    for (auto i : activeArcsAndWeight[2486]) {
        cout << mGraph.id( activeArcsAndWeight[2492][0].first ) << " : " << i.second << "\n";
    }

    cout << mGraph.id( activeArcsAndWeight[2492][0].first ) << " : " << activeArcsAndWeight[2492][0].second << endl;
    
}