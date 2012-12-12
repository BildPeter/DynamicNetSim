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
#include <lemon/time_measure.h>

#include <unordered_map>
#include <fstream>

using namespace std;
using namespace lemon;

int main(){

    vector< int >                                            activeTimes;
    unordered_map< int, vector< SmartDigraph::Arc > >        mTime_Arcs_Vec;
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

    cout << "read Time" << T.realTime() << endl;
    cout << "nodes: " << countNodes( mGraph ) << " Arcs: " << countArcs( mGraph ) << endl;
    
    cout << endl << "pause" << endl;
    
    
//    usleep(10000000);
    

    
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