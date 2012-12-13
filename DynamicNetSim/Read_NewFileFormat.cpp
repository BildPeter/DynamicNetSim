//
//  Read_NewFileFormat.cpp
//  DynamicNetSim
//
//  Created by Peter on 12.12.12.
//  Copyright (c) 2012 Peter. All rights reserved.
//

#include <unistd.h>

#include <lemon/smart_graph.h>
#include <lemon/lgf_reader.h>
#include <lemon/lgf_writer.h>
#include <lemon/time_measure.h>
#include <lemon/adaptors.h>

#include <unordered_map>
#include <fstream>

using namespace std;
using namespace lemon;

template < typename DGR >
vector<int> readTemporalArcList( DGR &graph, unordered_map< int, vector<typename DGR::Arc > > &Time_ToArcVec, string fn);

int main(){
    
    string  schweinLGF      = "/Users/sonneundasche/Documents/FLI/DATA/03 Daten - Schwein/porkNEW_.lgf";
    string  schweinDynArcs  = "/Users/sonneundasche/Documents/FLI/DATA/03 Daten - Schwein/porkNEW__time_tmpArcIDs.txt" ;
    string  schaf           = "/Users/sonneundasche/Documents/FLI/DATA/02 Daten - Schaf/Schaf_NEW__time_tmpArcIDs.txt";

    vector< int >                                            activeTimes;
    unordered_map< int, vector< SmartDigraph::Arc > >        mTime_Arcs_Vec;
    SmartDigraph                        mGraph;

    Timer T(true);
    digraphReader( mGraph, schweinLGF)
    .run();

    activeTimes = readTemporalArcList<SmartDigraph>( mGraph, mTime_Arcs_Vec, schweinDynArcs);
    
    cout << "read Time: " << T.realTime() << endl;
    cout << "nodes: " << countNodes( mGraph ) << " Arcs: " << countArcs( mGraph ) << endl;
    
    
//==================================================================================
//==================================================================================
    SmartDigraph::NodeMap<bool>             activeNodes(mGraph, false);
    SmartDigraph::ArcMap<bool>              activeArcs(mGraph, false);
    SubDigraph<SmartDigraph>                subSystem(mGraph, activeNodes, activeArcs);
    SmartDigraph::ArcMap<int>               arcCount( mGraph );
    SmartDigraph::NodeMap<int>              nodeCount( mGraph );

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
    
    cout << "calculation Time: " << T.realTime() << endl;
    cout << "calculation Steps: " << calSteps << endl;
    cout << nodeCount[ mGraph.nodeFromId( 2 ) ] << endl;
    cout << endl << "pause" << endl;
//    usleep(10000000);
    
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


template < typename DGR >
vector<int> readTemporalArcList( DGR &graph, unordered_map< int, vector<typename DGR::Arc > > &Time_ToArcVec, string fn){
    vector< int > activeTimes;
    ifstream input( fn );
    
    int     time, arcID;
    string tmpData;
    getline( input, tmpData );  // Kill first line (header)
    for (string line; getline(input, line, '\n'); ) {
        istringstream lineStream(line);
        lineStream >> time;     // -- read first number ---
        activeTimes.push_back( time );
        
        for (; getline(lineStream, tmpData, '\t'); ) {
            lineStream >> arcID;
            Time_ToArcVec[ time ].push_back( graph.arcFromId( arcID ) );
        }
        Time_ToArcVec[time].pop_back();    // -- do not really know why, seems to read last element twice (not endet be \t)
    }
    return activeTimes;
}