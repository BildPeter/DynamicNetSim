//
//  Read_NewFileFormat.cpp
//  DynamicNetSim
//
//  Created by Peter on 12.12.12.
//  Copyright (c) 2012 Peter. All rights reserved.
//

#include <unistd.h>

#include <lemon/smart_graph.h>
#include <lemon/static_graph.h>
#include <lemon/lgf_reader.h>
#include <lemon/lgf_writer.h>
#include <lemon/time_measure.h>
#include <lemon/adaptors.h>

#include <unordered_map>
#include <fstream>

using namespace std;
using namespace lemon;

int main(){

    vector< int >                                            activeTimes;
    unordered_map< int, vector< SmartDigraph::Arc > >        mTime_Arcs_Vec;
    StaticDigraph                       mStatGraph;
    SmartDigraph                        mGraph;
    SmartDigraph::ArcMap<int>           arcCount( mGraph );
    SmartDigraph::NodeMap<int>          nodeCount( mGraph );

    Timer T(true);
    digraphReader( mGraph, "/Users/sonneundasche/Documents/FLI/DATA/03 Daten - Schwein/porkNEW_.lgf" )
    .run();
    ifstream input( "/Users/sonneundasche/Documents/FLI/DATA/03 Daten - Schwein/porkNEW__time_tmpArcIDs.txt" );
    
    
//    digraphReader( mGraph, "/Users/sonneundasche/Documents/FLI/DATA/02 Daten - Schaf/Schaf_NEW_.lgf" )
//    .run();
//    ifstream input( "/Users/sonneundasche/Documents/FLI/DATA/02 Daten - Schaf/Schaf_NEW__time_tmpArcIDs.txt" );

    int     time, arcID;
    string tmpData;
    getline( input, tmpData );  // Kill first line (header)
    for (string line; getline(input, line, '\n'); ) {
        istringstream lineStream(line);
        lineStream >> time;     // -- read first number ---
        activeTimes.push_back( time );
        
        for (; getline(lineStream, tmpData, '\t'); ) {
            lineStream >> arcID;
            mTime_Arcs_Vec[ time ].push_back( mGraph.arcFromId( arcID ) );
        }
        mTime_Arcs_Vec[time].pop_back();    // -- do not really know why, seems to read last element twice (not endet be \t)
    }

    cout << "read Time: " << T.realTime() << endl;
    cout << "nodes: " << countNodes( mGraph ) << " Arcs: " << countArcs( mGraph ) << endl;
    
    
//==================================================================================
//==================================================================================
    SmartDigraph::NodeMap<bool>             activeNodes(mGraph, false);
    SmartDigraph::ArcMap<bool>              activeArcs(mGraph, false);
    SubDigraph<SmartDigraph>                subSystem(mGraph, activeNodes, activeArcs);
    int calSteps = 0;
    
    T.restart();
    
    for ( auto currentTime : activeTimes) {
        
        for ( auto arc : mTime_Arcs_Vec[ currentTime ] ){
            activeArcs[ arc ]                   = true;
            activeNodes[ mGraph.source( arc ) ] = true;
            activeNodes[ mGraph.target( arc ) ] = true;
        }
        
        for (SubDigraph<SmartDigraph>::NodeIt n(subSystem); n!=INVALID; ++n) {
            nodeCount[ n ]++;
            calSteps++;
        }

        for ( auto arc : mTime_Arcs_Vec[ currentTime ] ){
            activeArcs[ arc ]                   = false;
            activeNodes[ mGraph.source( arc ) ] = false;
            activeNodes[ mGraph.target( arc ) ] = false;
        }
    }
    
//    for (SmartDigraph::NodeIt n(mGraph); n!=INVALID; ++n) {
//        for (SmartDigraph::NodeIt n(mGraph); n!=INVALID; ++n)
//            nodeCount[ n ]++;
//    }
    
    cout << "calculation Time: " << T.realTime() << endl;
    cout << "calculation Steps: " << calSteps << endl;
    cout << nodeCount[ mGraph.nodeFromId( 2 ) ] << endl;
    cout << endl << "pause" << endl;
//    usleep(10000000);
    
    digraphWriter( mGraph, "/Users/sonneundasche/Documents/FLI/DATA/03 Daten - Schwein/pork_Activations.txt")
    .nodeMap("Activations", nodeCount)
    .skipArcs()
    .run();
    
    /*
        - Filter all nodes which were less than 14 times active in 8 years (less than twice a year)
        - Create the resulting graph with nodes and arcs
     */
    mapFill(mGraph, activeArcs, true);
    for (SmartDigraph::NodeIt n(mGraph); n!=INVALID; ++n) {
        if (nodeCount[n] > 14 )
            activeNodes[n]  = true;
    }
    
    cout << "subGraph - nodes: " << countNodes( subSystem ) << " | arcs: " << countArcs( subSystem ) << endl;
    
    digraphCopy(subSystem, mStatGraph);
    
    digraphWriter( mStatGraph, "/Users/sonneundasche/Documents/FLI/DATA/03 Daten - Schwein/pork_14Active_Graph.txt")
//    .nodeMap("Activations", nodeCount)
    .run();
    
    
    
//    for( int time : activeTimes ){
//        cout << time << " ";
//        for( auto arcs : mTime_Arcs_Vec[ time ] ){
//            cout << mGraph.id(arcs) << " ";
//        }
//        cout << endl;
//    }
//    for( auto arcs : mTime_Arcs_Vec[ 2555 ] ){
//        cout << mGraph.id(arcs) << " ";
//    }

}