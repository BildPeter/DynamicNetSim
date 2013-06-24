//
//  OwnFiles_ToDynEdgeList.cpp
//  DynamicNetSim
//
//  Created by Peter A. Kolski on 15.01.13.
//  Copyright (c) 2013 Peter A. Kolski. All rights reserved.
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