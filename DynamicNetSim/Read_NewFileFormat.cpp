//
//  Read_NewFileFormat.cpp
//  DynamicNetSim
//
//  Created by Peter A. Kolski (BildPeter Visuals) on 12.12.12.
//  Copyright (c) 2012 Peter A. Kolski.
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

#include <unistd.h>

#include <lemon/smart_graph.h>
#include <lemon/lgf_reader.h>
#include <lemon/lgf_writer.h>
#include <lemon/time_measure.h>
#include <lemon/adaptors.h>

#include <unordered_map>
#include <fstream>

#include <peter/temporal_graph_handler.h>

using namespace std;
using namespace lemon;

int main(){
    
    string  schweinLGF      = "/Users/sonneundasche/Documents/FLI/DATA/03 Daten - Schwein/porkNEW_.lgf";
    string  schweinDynArcs  = "/Users/sonneundasche/Documents/FLI/DATA/03 Daten - Schwein/porkNEW__time_tmpArcIDs.txt" ;
    string  schaf           = "/Users/sonneundasche/Documents/FLI/DATA/02 Daten - Schaf/Schaf_NEW__time_tmpArcIDs.txt";

    vector< int >                                            activeTimes;
    unordered_map< int, vector< SmartDigraph::Arc > >        mTime_Arcs_Vec;
    SmartDigraph                        mGraph;

    Timer T(true);
    // --- read the LEMON graph
    digraphReader( mGraph, schweinLGF)
    .run();

    // --- read
    activeTimes = tempGR::readTemporalArcList<SmartDigraph>( mGraph, mTime_Arcs_Vec, schweinDynArcs);
    
    cout << "read Time: " << T.realTime() << endl;
    cout << "nodes: " << countNodes( mGraph ) << " Arcs: " << countArcs( mGraph ) << endl;
    
    
//==================================================================================
//==================================================================================
    SmartDigraph::NodeMap<bool>             activeNodes(mGraph, false);
    SmartDigraph::ArcMap<bool>              activeArcs(mGraph, false);
    SubDigraph<SmartDigraph>                subSystem(mGraph, activeNodes, activeArcs);
    SmartDigraph::ArcMap<int>               arcCount( mGraph );
    SmartDigraph::NodeMap<int>              nodeCount( mGraph );

    tempGR::temporalGraphActivator<SmartDigraph>        mActivator( mGraph, activeNodes, activeArcs, mTime_Arcs_Vec );
    
    int calSteps = 0;
    
    T.restart();
    
    for ( auto currentTime : activeTimes) {
        
        mActivator.activate( currentTime );
        
        for (SubDigraph<SmartDigraph>::NodeIt n(subSystem); n!=INVALID; ++n) {
            nodeCount[ n ]++;
            calSteps++;
        }
        
        mActivator.deactivate( currentTime );
        
    }
    
    cout << "calculation Time: " << T.realTime() << endl;
    cout << "calculation Steps: " << calSteps << endl;
    cout << endl << "pause" << endl;
    usleep(10000000);
    
    //==================================================================================
    //==================================================================================    

    InDegMap<SmartDigraph>      mInDegree(mGraph);
    OutDegMap<SmartDigraph>     mOutDegree(mGraph);
    
    digraphWriter( mGraph, "/Users/sonneundasche/Documents/FLI/DATA/03 Daten - Schwein/pork_Activations.txt")
    .nodeMap("Activations", nodeCount)
    .nodeMap("InDegree", mInDegree)
    .nodeMap("OutDegree", mOutDegree)
    .skipArcs()
    .run();
    
    /*
        - Filter all nodes which were less than 14 times active in 8 years (less than twice a year)
        - Create the resulting graph with nodes and arcs
     */
    mapFill(mGraph, activeArcs, true);
    for (SmartDigraph::NodeIt n(mGraph); n!=INVALID; ++n) {
        if (nodeCount[n] > 32 )
            activeNodes[n]  = true;
    }
    
    cout << "subGraph - nodes: " << countNodes( subSystem ) << " | arcs: " << countArcs( subSystem ) << endl;
 }
