//
//  Trafo_EdgelistTo_TempLGF_nodeSize.cpp
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

#include "Trafo_EdgelistTo_TempLGF_nodeSize.h"

#include <lemon/arg_parser.h>
#include <lemon/smart_graph.h>
#include <lemon/lgf_writer.h>
#include <lemon/time_measure.h>

#include <peter/temporal_graph_handler.h>

#include <set>
#include <map>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <fstream>


using namespace lemon;
using namespace std;
using namespace boost;

typedef pair< unsigned long long, unsigned long long > arcIDpair;

int main( int argc, char ** argv){

    ArgParser   ag( argc, argv );
    
    
    Timer   T(true);
    // INPUT
    string edgeListSource   = "/Users/sonneundasche/Dropbox/FLI/04_HIT_Transform/_node size/uvwd_OCBD_07.txt";
    string outputNAME       = "/Users/sonneundasche/Dropbox/FLI/04_HIT_Transform/_node size/Schwein_BL_07";
    
    SmartDigraph                                                                mGraph;
    SmartDigraph::NodeMap< unsigned long long >                                 mOrigID( mGraph );
    set< unsigned long long >                                                   mUniqueNodes;   //temporär
    set< arcIDpair >                                                            mUniqueArcs;    //temporär
    set< unsigned int >                                                         mUniqueDays;
    map< arcIDpair, SmartDigraph::Arc >                                         mOrigPair_ToArc;
    unordered_map< unsigned long long, SmartDigraph::Node >                     mOrigID_ToNode;
    unordered_map< unsigned int, vector< arcIDpair > >                          mTime_ToPair_Vec;   //temporär
    unordered_map< unsigned int, map <arcIDpair, int > >                        mTime_ToPair_Amount_Map;   //temporär
    unordered_map< unsigned int, vector< SmartDigraph::Arc > >                  mTime_toArc_Vec;
    unordered_map< unsigned int, vector< pair<SmartDigraph::Arc, int > > >      mTime_toArc_Amount_Vec;
    /*
     // --- Daten Anordnung ---
     // From	To	Amount	Date
     // 560739	254569	7	2682
     // 913338	356536	1	3497
     */
    
    mGraph.clear();     // the Input graph will be resettet
    
    
    ifstream    myEdgeListFile( edgeListSource.c_str() );
    unsigned int amount, day;
    unsigned long long from, to;
    pair<unsigned long long, unsigned long long>      tmpPair;

    
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
        mTime_ToPair_Amount_Map[ day ].insert( make_pair( tmpPair, amount ) );
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
            mTime_toArc_Amount_Vec[ time ].push_back( make_pair(
                                                                mOrigPair_ToArc[ mPair ],
                                                                (mTime_ToPair_Amount_Map[ time ])[ mPair ] ) );
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
    
    string tmpName = outputNAME + "_time_tmpArcIDs.txt";
    
    ofstream    outFile( tmpName.c_str() );
    outFile << "time\tarcIDs\n";
    
    for ( auto time : mUniqueDays){
        outFile << time;
        for ( auto arc : mTime_toArc_Vec[time] ){
            outFile << "\t" << mGraph.id( arc );
        }
        outFile << "\n";
    }
    outFile.close();
    
    tmpName = outputNAME + "_time_tmpArcIDs_amountOnArc.txt";
    
    outFile.open( tmpName.c_str() );
    outFile << "time\tarcIDs_amount\n";
    
    for ( auto time : mUniqueDays){
        outFile << time;
        for ( auto pair : mTime_toArc_Amount_Vec[time] ){
            outFile << "\t" << mGraph.id( pair.first ) << " " << pair.second;
        }
        outFile << "\n";
    }
    outFile.close();
    
    
    tmpName = outputNAME + "_time_countArcs.txt";
    
    outFile.open( tmpName.c_str() );
    outFile << "time\tamountOfArcs\n";
    for ( auto time : mUniqueDays){
        outFile << time << "\t" << mTime_toArc_Vec[time].size() << "\n";
    }
    outFile.close();
    
    
    
    //    delete mUniqueArcs;
    //    delete mUniqueNodes;


    
}