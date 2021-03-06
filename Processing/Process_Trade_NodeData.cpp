//
//  Process_Trade_NodeData.cpp
//  DynamicNetSim
//
//  Created by Peter A. Kolski (BildPeter Visuals) on 23.10.12.
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


// C++
#include <iostream>
#include <unordered_set>
#include <utility>
#include <algorithm>
#include <vector>

// Boost
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

// LEMON
#include <lemon/list_graph.h>
#include <lemon/lgf_reader.h>
#include <lemon/lgf_writer.h>
#include <lemon/maps.h>
#include <lemon/time_measure.h>


using namespace std;
using namespace lemon;

/* --------------------------------------------------------------------------------------------------------
 TASK:
 I read two kinds of files:
    1.) Trade data file - daily arc set with amount
    2.) Node data file  - informations for each node
 This programm will 
 + read the trade file, create a LEMON graph out of it
 + read the node data file and assign the data to the LEMON graph (maps)
 + analyse the graph regarding to the trade process
 
 
 -------------------------------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------------------
 TO DO:
 - Sind die Transaction/Flow maps mit int / Long Long ausreichend?
 - origIDtoLemon Map geht auch anders: IterableIntMap
 -------------------------------------------------------------------------------------------------------- */



typedef boost::unordered_map< unsigned int, vector< ListDigraph::Arc > >    timeToActiveArcs;

// ======================================================================================================================
// ======================================================================================================================

int main( void ){
    
    /*
     --- Daten Anordnung ---
     From	To	Amount	Date
     560739	254569	7	2682
     913338	356536	1	3497
     */
    
    // Datenstrukur von C++11 (werde aber Boost verwenden)
    //    unordered_set< unsigned int >       uniqueNodes;
    //    unordered_set< int >                uniqueArcs;
    //    pair< unsigned int, unsigned int >  paar;
    
    Timer t;
    
    string edgeListSource   = "/Users/sonneundasche/Dropbox/FLI/DATA/EinBundesland/02 Transport/tradeData_einKreis.csv";
    string lemonFileTmp     = "/Users/sonneundasche/Dropbox/FLI/DATA/EinBundesland/03 Processed/tmp.lgf";
    string lemonFileOut     = "/Users/sonneundasche/Dropbox/FLI/DATA/EinBundesland/03 Processed/tradeData_processed.lgf";
    string nodeDataFile     = "/Users/sonneundasche/Dropbox/FLI/DATA/EinBundesland/01 Betriebsgröße/Daten_Bearbeitet.csv";
    
    boost::unordered_set< unsigned int >                        uniqueNodes; // Mathematische Menge (set) der Knoten
    boost::unordered_set< pair< unsigned int, unsigned int > >  uniqueArcs;  // Menge der Kanten (set von Knoten-Paaren)
    //    vector< unsigned int >      vecAmount;
    //    vector< unsigned int >      vecDay;
  
    // =========================================================================================
    // Read the trade file + create LEMON Graph
    // =========================================================================================

    ifstream    myEdgeListFile( edgeListSource );
    string      foo;  // kill first line, because it has the header
    getline(myEdgeListFile, foo);
    
    unsigned int from, to, amount, day;
    
    // Einlesen der Werte in die Menge. Paar-Erstellung.
    while ( myEdgeListFile.good() ){
        
        myEdgeListFile >> from;
        myEdgeListFile >> to;
        myEdgeListFile >> amount;
        myEdgeListFile >> day;
        
        // --- find unique elements
        uniqueNodes.insert( from );
        uniqueNodes.insert( to );
        uniqueArcs.insert( make_pair( from, to ) );
        
        
    }
    
    myEdgeListFile.close();
    cout << "Nodes: " << uniqueNodes.size() << " Arcs: " << uniqueArcs.size() << endl;;
    
    // ----------------------------------------------------------------------------------------
    // Schreiben der LFG durch eigene Routine
    // ----------------------------------------------------------------------------------------
    
    ofstream myLemonFile( lemonFileTmp );
    
    // ----- Nodes -----
    myLemonFile << "@nodes" << endl;
    myLemonFile << "label" << "\t" << "name" << endl;
    for (auto iter = uniqueNodes.begin(); iter != uniqueNodes.end(); iter++) {
        myLemonFile << *iter << "\t"
        << *iter << endl;
    }
    
    myLemonFile << endl << endl << "@arcs" << endl;
    myLemonFile << "\t\tfrom\tto" << endl;
    for (auto iterArcs = uniqueArcs.begin(); iterArcs  != uniqueArcs.end(); iterArcs++ ){
        myLemonFile << (*iterArcs).first    << "\t"         // Erster  Paar-Eintrag für LEMON, damit es mit den Nodes übereinstimmt
        << (*iterArcs).second    << "\t"
        << (*iterArcs).first    << "\t"         // Zweiter Paar-Eintrag erhält die alten IDs, damit die Daten nicht beim Umwandeln verloren gehen
        << (*iterArcs).second   << endl;
    }

    
    
    // =========================================================================================
    // Create Graph and Format in a Lemon friendly way
    // Hier erhalten die Knoten und Kanten LEMON IDs
    ListDigraph myCulmiGraph;
    ListDigraph::NodeMap< unsigned int >    nameNodeMap( myCulmiGraph );
    ListDigraph::ArcMap< unsigned int >     fromNdArcMap( myCulmiGraph );
    ListDigraph::ArcMap< unsigned int>      toNdArcMap   ( myCulmiGraph );
    ListDigraph::ArcMap< bool >             activeArcsMap( myCulmiGraph );
    
    timeToActiveArcs    dayActivityArcIDs; //Menge an Zeitpunkten (int) und den dazu gehörigen aktiven Kanten (vec)
    
    boost::unordered_map< unsigned int, unsigned int >              origIDtoLemon;
    
    // Hier besitzen die Knoten & Kanten noch original IDs (name)
    digraphReader( myCulmiGraph, lemonFileTmp)
    .nodeMap( "name", nameNodeMap )
    .arcMap( "from", fromNdArcMap )
    .arcMap( "to", toNdArcMap)
    .run();

    // =========================================================================================
    // READ NODE DATA FILE + assign values to nodes
    // =========================================================================================

    ifstream    fileNodeData( nodeDataFile );
    getline( fileNodeData, foo );
    unsigned int id, xCo, yCo, am_all, am_c, am_p;
    boost::unordered_map<unsigned int, unsigned int >   mpxCo, mpyCo, mpAmA, mpAmC, mpAmP;  // Hash Maps
    
    while ( fileNodeData.good() ) {
        fileNodeData >> id;
        fileNodeData >> xCo;
        fileNodeData >> yCo;
        fileNodeData >> am_all;
        fileNodeData >> am_c;
        fileNodeData >> am_p;
                
        // Hash zuweisen ???
        mpxCo[ id ] = xCo;
        mpyCo[ id ] = yCo;
        mpAmA[ id ] = am_all;
        mpAmC[ id ] = am_c;
        mpAmP[ id ] = am_p;
    }
    
    fileNodeData.close();
    cout << "Amount of nodes in the node data: " << mpAmP.size() << endl;

    // =========================================================================================
    // Assign new node maps
    // Hash -> LEMON Map
    
    ListDigraph::NodeMap< unsigned int >    xCoord( myCulmiGraph );
    ListDigraph::NodeMap< unsigned int >    yCoord( myCulmiGraph );
    ListDigraph::NodeMap< unsigned int >    Amount_All( myCulmiGraph );
    ListDigraph::NodeMap< unsigned int >    Amount_Cattle( myCulmiGraph );
    ListDigraph::NodeMap< unsigned int >    Amount_Pork( myCulmiGraph );
    
    for ( ListDigraph::NodeIt n( myCulmiGraph ); n!=INVALID; ++n){
        xCoord[ n ] = mpxCo[ nameNodeMap[ n ] ];
        yCoord[ n ] = mpyCo[ nameNodeMap[ n ] ];
        Amount_All[ n ]     = mpAmA[ nameNodeMap[ n ] ];
        Amount_Cattle[ n ]  = mpAmC[ nameNodeMap[ n ] ];
        Amount_Pork[ n ]    = mpAmP[ nameNodeMap[ n ] ];

        // Eine Map, die original KnotenIDs zu LEMON IDs zuweist (LemonMap Umkehr, statisch)
        // Nötig, da ich die orginalIDs aus der Ursprungsdatei einlese
        origIDtoLemon[ nameNodeMap[ n ] ] = myCulmiGraph.id( n );
    }
    
    // =========================================================================================
    // ANALYSE TRADE DATA + assign values to nodes & arcs
    // =========================================================================================
    
    InDegMap< ListDigraph >                          nodeInDegree( myCulmiGraph );
    OutDegMap< ListDigraph >                         nodeOutDegree( myCulmiGraph );
    ListDigraph::NodeMap< int >                      nodeDegree( myCulmiGraph );
    ListDigraph::NodeMap< unsigned long long >       nodeInFlow ( myCulmiGraph );
    ListDigraph::NodeMap< unsigned long long >       nodeOutFlow ( myCulmiGraph );
    ListDigraph::NodeMap< long int >                 nodeFlowDifference( myCulmiGraph );
    ListDigraph::NodeMap< unsigned int >             nodeTransactions( myCulmiGraph );
    ListDigraph::ArcMap< unsigned long long >        arcFlow( myCulmiGraph );
    ListDigraph::ArcMap< unsigned int >              arcTransactions( myCulmiGraph );
    ListDigraph::Arc                                 currentArc;
    ListDigraph::Node                                fromNode, toNode;
    
    myEdgeListFile.open( edgeListSource );
    getline(myEdgeListFile, foo);
    
    // Einlesen der Werte in die Menge. Paar-Erstellung.
    while ( myEdgeListFile.good() ){
        
        myEdgeListFile >> from;
        myEdgeListFile >> to;
        myEdgeListFile >> amount;
        myEdgeListFile >> day;
        
        currentArc = findArc( myCulmiGraph,  myCulmiGraph.nodeFromId( origIDtoLemon[ from ] ),
                                             myCulmiGraph.nodeFromId( origIDtoLemon[ to   ] ) );
        fromNode   = myCulmiGraph.nodeFromId( origIDtoLemon[ from ] );
        toNode     = myCulmiGraph.nodeFromId( origIDtoLemon[ to ] );
        
        nodeInFlow[ toNode ]            += amount;
        nodeOutFlow[ fromNode ]         += amount;
        nodeTransactions[ toNode ]      ++;
        nodeTransactions[ fromNode ]    ++;
        arcFlow[ currentArc ]           += amount;
        arcTransactions[ currentArc ]   ++;
    }
    
    // Calculate flow difference and degree
    for (ListDigraph::NodeIt n( myCulmiGraph ); n!=INVALID; ++n ){
        nodeDegree[ n ]         = nodeInDegree[ n ] + nodeOutDegree[ n ];
        nodeFlowDifference[ n ] = nodeInFlow[ n ] - nodeOutFlow[ n ];
    }
    
    cout << "LEMON - Nodes: " << countNodes( myCulmiGraph ) << " Arcs: " << countArcs( myCulmiGraph ) << endl;
    
    // Ausgabe der neuen LEMON IDs für Knoten und Kanten
    digraphWriter( myCulmiGraph, lemonFileOut)
    .nodeMap( "name", nameNodeMap )
    .nodeMap("xCoord", xCoord )
    .nodeMap("yCoord", yCoord)
    .nodeMap("size_all", Amount_All)
    .nodeMap("size_cattle", Amount_Cattle)
    .nodeMap("size_pork", Amount_Pork)
    .nodeMap("degree", nodeDegree )
    .nodeMap("inDegree", nodeInDegree)
    .nodeMap("outDegree", nodeOutDegree)
    .nodeMap("inFlow", nodeInFlow)
    .nodeMap("outFlow", nodeOutFlow)
    .nodeMap("flowDifference", nodeFlowDifference )
    .nodeMap("transactions", nodeTransactions)
    .arcMap( "from", fromNdArcMap )
    .arcMap( "to" , toNdArcMap )
    .arcMap("flow", arcFlow)
    .arcMap("transactions", arcTransactions)
    .run();
    
}

