//
//  Read_tempGraph.cpp
//  DynamicNetSim
//
//  Created by Peter on 12.12.12.
//  Copyright (c) 2012 Peter. All rights reserved.
//


#include <lemon/smart_graph.h>
#include <lemon/lgf_writer.h>
#include <lemon/time_measure.h>

#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <fstream>



using namespace std;
using namespace lemon;

int main( void ){

    Timer   T(true);
    // INPUT
    SmartDigraph                    mGraph;
    SmartDigraph::NodeMap<int>      mOrigID( mGraph );
    SmartDigraph::ArcMap<int>       mFrom( mGraph );
    SmartDigraph::ArcMap< int >     mTo(mGraph );
    string edgeListSource   = "/Users/sonneundasche/Documents/FLI/DATA/03 Daten - Schwein/Pork_tempEdgeList.txt";
    string outputNAME       = "/Users/sonneundasche/Documents/FLI/DATA/03 Daten - Schwein/Pork_Data";
    
    set< unsigned int >                                                         mUniqueNodes;   //temporär
    set< pair <unsigned int, unsigned int > >                                   mUniqueArcs;    //temporär
    set< unsigned int >                                                         mUniqueDays;
    map< pair< unsigned int, unsigned int>, SmartDigraph::Arc >                 mOrigPair_ToArc;
    unordered_map< unsigned int, SmartDigraph::Node >                           mOrigID_ToNode;
    unordered_map< unsigned int, vector< pair< unsigned int, unsigned int > > > mTime_ToPair_Vec;   //temporär
    unordered_map< unsigned int, vector< SmartDigraph::Arc > >                  mTime_toArc_Vec;
    
/*
    // --- Daten Anordnung ---
    // From	To	Amount	Date
    // 560739	254569	7	2682
    // 913338	356536	1	3497
 */
    

    mGraph.clear();     // the Input graph will be resettet
    
    
    ifstream    myEdgeListFile( edgeListSource );
    unsigned int from, to, amount, day;
    pair<int, int>      tmpPair;
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
    }
    cout << "Unique - \t Nodes " << mUniqueNodes.size() << " Arcs: " << mUniqueArcs.size() << endl;;

    
    // ========= CREATE AGGREGATED GRAPH =======
    
    // ---------------------------------------------------
    // --- Create nodes in graph
    // ---- combine ID with the new node
    // ---------------------------------------------------
    for ( auto nodeID : mUniqueNodes ) {
        SmartDigraph::Node n = mGraph.addNode();
        mOrigID_ToNode[ nodeID ] =  n ;
    }
    // ---------------------------------------------------
    // --- Create ARCS in graph
    // --- combine pairIDs with the new arc
    // ---------------------------------------------------
    for ( auto arcPair : mUniqueArcs ){
        SmartDigraph::Arc   arc = mGraph.addArc( mOrigID_ToNode[ arcPair.first ], mOrigID_ToNode[ arcPair.second ] );
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
    
    for (auto mPair : mUniqueArcs ){
        mFrom[ mOrigPair_ToArc[ mPair ] ] = mPair.first;
        mTo[ mOrigPair_ToArc[ mPair ] ]     = mPair.second;
    }
    
    cout << "build time: " << T.realTime() << endl;
    cout << "LEMON - \t Nodes " << countNodes( mGraph ) << " - Arcs: " << countArcs( mGraph ) << endl;
    cout << "HASH - \t\t Nodes " << mOrigID_ToNode.size() << " - Arcs: " << mOrigPair_ToArc.size() << endl;
    
    
    // ======================================
    // ========= WRITE TO FILES       =======
    // ======================================
    
    digraphWriter( mGraph, outputNAME + ".lgf")
    .nodeMap("origID", mOrigID )
    .arcMap("origFrom", mFrom)
    .arcMap("origTo", mTo)
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
    
    outFile.open( outputNAME + "_time_amountOfArcs.txt" );
    outFile << "time" << "\t" << "amountOfArcs" << endl;
    for ( auto time : mUniqueDays){
        outFile << time << "\t";
        outFile << mTime_toArc_Vec[time].size() << endl;
    }
    outFile.close();
    
    

//    delete mUniqueArcs;
//    delete mUniqueNodes;
}

