//
//  Read_tempGraph.cpp
//  DynamicNetSim
//
//  Created by Peter A. Kolski on 12.12.12.
//  Copyright (c) 2012 Peter A. Kolski. All rights reserved.
//




#include <iostream>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <fstream>

#include <peter/temporal_graph_handler.h>


#include <lemon/smart_graph.h>
#include <lemon/lgf_writer.h>
#include <lemon/time_measure.h>



using namespace std;
using namespace lemon;

typedef pair< long long, long long > arcIDpair;

int main( void ){

    Timer   T(true);
    // INPUT
    SmartDigraph                            mGraph;
    SmartDigraph::NodeMap< long long >      mOrigID( mGraph );
    string edgeListSource   = "/Volumes/Augenweide/HIT_Jul2006_CLEAN.txt";
    string outputNAME       = "/Volumes/Augenweide/HIT_Jul2006_CLEAN";
    
    set< long long >                                                         mUniqueNodes;   //temporär
    set< arcIDpair >                                                         mUniqueArcs;    //temporär
    set< long long >                                                         mUniqueDays;
    map< arcIDpair, SmartDigraph::Arc >                                         mOrigPair_ToArc;
    unordered_map< long long, SmartDigraph::Node >                           mOrigID_ToNode;
    unordered_map< long long, vector< arcIDpair > >                          mTime_ToPair_Vec;   //temporär
    unordered_map< long long, map <arcIDpair, int > >                        mTime_ToPair_Amount_Vec;   //temporär
    unordered_map< long long, vector< SmartDigraph::Arc > >                  mTime_toArc_Vec;
    unordered_map< long long, vector< pair<SmartDigraph::Arc, int > > >      mTime_toArc_Amount_Vec;
/*
    // --- Daten Anordnung ---
    // From	To	Amount	Date
    // 560739	254569	7	2682
    // 913338	356536	1	3497
 */
    

    mGraph.clear();     // the Input graph will be resettet
    
    
    ifstream    myEdgeListFile( edgeListSource );
    long long from, to, amount, day;
    pair<long long, long long>      tmpPair;
    string      foo;  // kill first line, because it has the header
    getline(myEdgeListFile, foo);
    
    // Einlesen der Werte in die Menge. Paar-Erstellung.
    while ( myEdgeListFile.good() ){
        
        myEdgeListFile >> from;
        myEdgeListFile >> to;
        myEdgeListFile >> amount;
        myEdgeListFile >> day;
        
        mUniqueNodes.insert( from );
        mUniqueNodes.insert( to );
        mUniqueDays.insert( day );
        
        tmpPair = make_pair( from, to );
        mUniqueArcs.insert( tmpPair );
        mTime_ToPair_Vec[ day ].push_back( tmpPair );
        mTime_ToPair_Amount_Vec[ day ].insert( make_pair( tmpPair, amount ) );
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
            mTime_toArc_Amount_Vec[ time ].push_back( make_pair(
                                                                mOrigPair_ToArc[ mPair ],
                                                                (mTime_ToPair_Amount_Vec[ time ])[ mPair ] ) );
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

    outFile.open( outputNAME + "_time_tmpArcIDs_amountOnArc.txt" );
    outFile << "time" << "\t" << "arcIDs_amount" << endl;
    
    for ( auto time : mUniqueDays){
        outFile << time << "\t";
        for ( auto pair : mTime_toArc_Amount_Vec[time] ){
            outFile << mGraph.id( pair.first ) << " " << pair.second << "\t";
        }
        outFile << endl;
    }
    outFile.close();

    
    outFile.open( outputNAME + "_time_countArcs.txt" );
    outFile << "time" << "\t" << "amountOfArcs" << endl;
    for ( auto time : mUniqueDays){
        outFile << time << "\t";
        outFile << mTime_toArc_Vec[time].size() << endl;
    }
    outFile.close();
    
    

//    delete mUniqueArcs;
//    delete mUniqueNodes;
}

