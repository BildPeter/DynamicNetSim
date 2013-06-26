//
//  OwnFiles_ToDynEdgeList.cpp
//  DynamicNetSim
//
//  Created by Peter A. Kolski (BildPeter Visuals) on 15.01.13.
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


#include <lemon/list_graph.h>
#include <lemon/lgf_reader.h>
#include <lemon/time_measure.h>
#include <lemon/adaptors.h>

#include <boost/unordered_map.hpp>
#include <peter/temporal_graph_handler.h>

#include <stdio.h>
#include <fstream>

using namespace std;
using namespace lemon;

int main(){

    string  schweinLGF      = "/Users/sonneundasche/Documents/FLI/DATA/03 Daten - Schwein/porkNEW_.lgf";
    string  schweinDynArcs  = "/Users/sonneundasche/Documents/FLI/DATA/03 Daten - Schwein/porkNEW__time_tmpArcIDs.txt" ;

    ListDigraph     g;
    ListDigraph::NodeMap<bool>             activeNodes( g, false);
    ListDigraph::ArcMap<bool>              activeArcs(  g, false);
    FilterArcs<ListDigraph>                 mArcSys(g, activeArcs);
    
    vector< int >                                             activeTimes;
    boost::unordered_map< int, vector< ListDigraph::Arc > >   mTime_Arcs_Vec;

    activeTimes = tempGR::readTemporalArcList<ListDigraph>( g, mTime_Arcs_Vec, schweinDynArcs);
    
    cout << "Start" << endl;
//---------------------------------------------------------------------------------
//                      SETUP THE SYSTEM
//---------------------------------------------------------------------------------
// --- read the graphs
    digraphReader( g, schweinLGF)
    .run();
    
    tempGR::temporalGraphActivator<ListDigraph>     Activator( g, activeNodes, activeArcs, mTime_Arcs_Vec );
    
    ofstream    file("/Users/sonneundasche/Documents/FLI/DATA/03 Daten - Schwein/pork_continousTimes.dynEdge");
    
    for (int i = 0; i < activeTimes.size(); i++) {
        Activator.activate( activeTimes[ i ]);
        
        for (FilterArcs<ListDigraph>::ArcIt arc(mArcSys); arc!=INVALID; ++arc) {
            file << g.id( g.source(arc) ) << "\t" << g.id( g.target( arc)) << "\t1\t" << i << endl;
        }
        
        Activator.deactivate( activeTimes[ i ] );
    }

    file.close();
}