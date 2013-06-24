//
//  Process_EdgeList_toTempGraph_noWeight.cpp
//  DynamicNetSim
//
//  Created by Peter A. Kolski on 03.06.13.
//  Copyright (c) 2013 Peter A. Kolski. All rights reserved.
//


#include <lemon/smart_graph.h>
#include <lemon/lgf_writer.h>
#include <lemon/time_measure.h>
#include <lemon/arg_parser.h>

#include <peter/temporal_graph_handler.h>

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <fstream>





using namespace std;
using namespace lemon;

typedef pair< long long, long long > arcIDpair;

int main( int argc, char** argv ){
    
    string edgeListSource   = "/Volumes/Augenweide/Facebook/data/facebook_links_zero.txt";
    string outputNAME       = "/Volumes/Augenweide/Facebook/data/facebook_Links";

    if ( argc < 2 ) {
        cout << "\n1.) IN edgeList with time \n2.) target name\n";
        return 1;
    }else{
        edgeListSource      = argv[ 1 ];
        outputNAME          = argv[ 2 ];
    }

    
    Timer   T(true);
    // INPUT
    SmartDigraph                            mGraph;
    SmartDigraph::NodeMap< long long >      mOrigID( mGraph );

    
    set< long long >                                                         mUniqueNodes;   //temporär
    set< arcIDpair >                                                         mUniqueArcs;    //temporär
    set< long long >                                                         mUniqueDays;
    map< arcIDpair, SmartDigraph::Arc >                                         mOrigPair_ToArc;
    unordered_map< long long, SmartDigraph::Node >                           mOrigID_ToNode;
    unordered_map< long long, vector< arcIDpair > >                          mTime_ToPair_Vec;   //temporär

    unordered_map< long long, vector< SmartDigraph::Arc > >                  mTime_toArc_Vec;

    /*
     // --- Daten Anordnung ---
     // From	To	Date
     // 560739	254569	2682
     // 913338	356536	3497
     */
    
    
    mGraph.clear();     // the Input graph will be resettet
    
    
    ifstream    myEdgeListFile( edgeListSource );
    long long from, to, day;
    pair<long long, long long>      tmpPair;
    string      foo;  // kill first line, because it has the header
    getline(myEdgeListFile, foo);
    
    // Einlesen der Werte in die Menge. Paar-Erstellung.
    while ( myEdgeListFile.good() ){
        
        myEdgeListFile >> from;
        myEdgeListFile >> to;
        myEdgeListFile >> day;
        
        mUniqueNodes.insert( from );
        mUniqueNodes.insert( to );
        mUniqueDays.insert( day );
        
        tmpPair = make_pair( from, to );
        mUniqueArcs.insert( tmpPair );
        mTime_ToPair_Vec[ day ].push_back( tmpPair );
    }
    cout << "Unique - \t Nodes " << mUniqueNodes.size() << " Arcs: " << mUniqueArcs.size() << endl;;
    
    
    // ========= CREATE AGGREGATED GRAPH =======
    
    // ---------------------------------------------------
    // --- Create nodes in graph
    // ---- combine ID with the new node
    // ---------------------------------------------------
    //    std::sort( mUniqueNodes.begin(), mUniqueNodes.end() );
    SmartDigraph::Node n;
    
    for ( auto nodeID : mUniqueNodes ) {
        n = mGraph.addNode();
        mOrigID_ToNode[ nodeID ] =  n ;
    }
    // ---------------------------------------------------
    // --- Create ARCS in graph
    // --- combine pairIDs with the new arc
    // ---------------------------------------------------
    SmartDigraph::Arc   arc;
    for ( auto arcPair : mUniqueArcs ){
        arc = mGraph.addArc( mOrigID_ToNode[ arcPair.first ], mOrigID_ToNode[ arcPair.second ] );
        mOrigPair_ToArc.insert( make_pair( arcPair, arc )  );
    }
    
    // ---------------------------------------------------
    // --- Save original node ID
    // ---------------------------------------------------
    for ( auto nodeID : mUniqueNodes ){
        mOrigID[ mOrigID_ToNode[ nodeID ] ] = nodeID;
    }
    
    // ========= CREATE TEMPORAL ARCS =======
    
    // ---------------------------------------------------
    // --- Transform pair of IDs into arcs
    // ---------------------------------------------------
    for ( auto time : mUniqueDays ){
        for ( auto mPair : mTime_ToPair_Vec[ time ] ){
            mTime_toArc_Vec[ time ].push_back( mOrigPair_ToArc[ mPair ] );
        }
    }
    
    cout << "build time: " << T.realTime() << endl;
    cout << "LEMON - \t Nodes " << countNodes( mGraph ) << " - Arcs: " << countArcs( mGraph ) << endl;
    cout << "HASH - \t\t Nodes " << mOrigID_ToNode.size() << " - Arcs: " << mOrigPair_ToArc.size() << endl;
    
    
    // ======================================
    // ========= WRITE TO FILES       =======
    // ======================================
    
    digraphWriter( mGraph, outputNAME + ".lgf")
    .nodeMap("origID", mOrigID )
    .run();
    
    ofstream    outFile( outputNAME + "_time_tmpArcIDs.txt" );
    outFile << "time" << "\t" << "arcIDs" << endl;
    
    for ( auto time : mUniqueDays){
        outFile << time << "\t";
        for ( auto arc : mTime_toArc_Vec[time] ){
            outFile << mGraph.id( arc ) << "\t";
        }
        outFile << endl;
    }
    outFile.close();
    
}
