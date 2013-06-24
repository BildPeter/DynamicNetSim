//
//  TemporalActiveArcs.cpp
//  NE2_SIR
//
//  Created by BildPeter Visuals on 16.11.11.
//  Copyright (c) 2011 Peter A. Kolski.
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

// NetEvo
#include <netevo.h>

// Boost
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

// LEMON
#include <lemon/lgf_reader.h>
#include <lemon/lgf_writer.h>
#include <lemon/maps.h>
#include <lemon/time_measure.h>


using namespace std;
using namespace lemon;
using namespace netevo;

/* --------------------------------------------------------------------------------------------------------
 TASK:
 General: Dynamics should run over the network with arcs beeing temporal on or off.
 
 The aim is to read the real data we have from HIT. It's in the form of an edgelist with time resolution.
 Then the NetEvo dynamics should run for a fixed timespan. The following interaction should be applied only 
 on arcs which are temporally turned on or off.
 
 + Read an edgelist and transform it into a LEMON Graph
 + Read the edgelist again and assign active arcs (IDs) to each timestep
 + Create a LemonMap with BOOL, to assigne active arcs
 + Use the BoolLemonMap in the NEmapping to calculate the interaction
 + Swap ODE-Simulation and Mapping
 
 -------------------------------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------------------
 TO DO:
 - Überprüfe die Zuweisung, der Korrekten AN/AUS Kanten 
 
 -------------------------------------------------------------------------------------------------------- */



typedef boost::unordered_map< unsigned int, vector< ListDigraph::Arc > >    timeToActiveArcs;
void initDegreeDistributed( System &sys, State &initial, int states, double degreeFactor );
void setBoolMapOfDay( IterableBoolMap< ListDigraph, ListDigraph::Arc >      &localItBoolMap, timeToActiveArcs &dayActiveArcs , unsigned int currentDay);



// ======================================================================================================================
// ======================================================================================================================

class ActiveArcsInteractionMap    : public NodeDynamic {
public:
    IterableBoolMap< ListDigraph, ListDigraph::Arc >   *arcBoolMap;  // An arcMap should be read from the Contructor!
    
    string  getName()   { return "InteractionMap"; }
    int     getStates() { return 3; }
    void    setDefaultParams( Node n, System &sys ){
        sys.nodeData( n ).dynamicParams.push_back( 0.1 );  // Parameter um die Transport-Rate festzulegen
        //        sys.nodeData( n ).dynamicParams.push_back( 1 );  // Population size
    }
    
    void    fn( Node n, System &sys, const State &x, State &dx, double t ){
        
        double nodeID = sys.stateID( n );
        
        double sumS = 0.0;
        double sumI = 0.0;
        double sumR = 0.0;
        
        if ( countInArcs( sys, n ) > 0 ){
            double sourcePopulation, nodePopulation;
            double unchangedRest, demandRatio;
            
            // ----- Get all the States of the sources
            for (System::InArcIt Arc( sys, n ); Arc != INVALID; ++Arc ){
                if ( (*arcBoolMap)[ Arc ] ) {                               // Hier wird entschieden, ob die Kante an ist
                    sumS += x[ sys.stateID( sys.source( Arc ) )     ];
                    sumI += x[ sys.stateID( sys.source( Arc ) ) + 1 ];
                    sumR += x[ sys.stateID( sys.source( Arc ) ) + 2 ];
                }
            }
            
            // ----- Population amount
            // Conditions are 
            // 1.) There are InArcs
            // 2.) The sum of values is not ZERO (not all arcs InActive)
            sourcePopulation = sumS + sumI + sumR;
            
            if ( sourcePopulation > 0 ) {
                nodePopulation =  x[ nodeID ] + x[ nodeID + 1 ] + x[ nodeID + 2 ];            
                
                // ----- Amount of 'demand' for the current node
                unchangedRest  = 1 - sys.nodeData( n ).dynamicParams[ 0 ];
                demandRatio =     sys.nodeData( n ).dynamicParams[ 0 ];
                
                // ----- Only the 'demand' amount will be scaled and changed by the interaction
                dx[ nodeID     ] = x[ nodeID     ] * unchangedRest + ( sumS * nodePopulation / sourcePopulation * demandRatio );
                dx[ nodeID + 1 ] = x[ nodeID + 1 ] * unchangedRest + ( sumI * nodePopulation / sourcePopulation * demandRatio );
                dx[ nodeID + 2 ] = x[ nodeID + 2 ] * unchangedRest + ( sumR * nodePopulation / sourcePopulation * demandRatio );
            }
            else{
                //                cout << " FEHLER!! SourcePopulation = 0 für Node " << sys.id( n ) << endl << endl;  // Fehler abfangen
                dx[ nodeID     ] = x[ nodeID     ] ;
                dx[ nodeID + 1 ] = x[ nodeID + 1 ] ;
                dx[ nodeID + 2 ] = x[ nodeID + 2 ] ;
            }
            
        }
        else{
            dx[ nodeID     ] = x[ nodeID     ] ;
            dx[ nodeID + 1 ] = x[ nodeID + 1 ] ;
            dx[ nodeID + 2 ] = x[ nodeID + 2 ] ;
        }
    }
    
    ActiveArcsInteractionMap( IterableBoolMap< ListDigraph, ListDigraph::Arc > &myIterMap ){
        arcBoolMap = &myIterMap;
    }
    
    
};


class SIRdynamic : public NodeDynamic {
public:
    double beta;
    double lambda;
    
    string  getName()   { return "SIRdynamic"; }
    int     getStates() { return 3; }
    void    setDefaultParams( Node n, System &sys ){
        sys.nodeData( n ).dynamicParams.push_back( beta );
        sys.nodeData( n ).dynamicParams.push_back( lambda );
    }
    
    void fn( Node n, System &sys, const State &x, State &dx, const double t ) {
        int nodeID = sys.stateID( n );
        vector< double > &nodeParams = sys.nodeData( n ).dynamicParams;
        double N = x[ nodeID ] + x[ nodeID + 1 ] + x[ nodeID + 2 ];
        
        //    dS = - S * I * beta
        //    dI = 
        //    dR = lambda * I
        
        dx[ nodeID     ] = ( (-1) * x[ nodeID ] * x[ nodeID + 1 ] * nodeParams[ 0 ]  ) / N;
        dx[ nodeID + 1 ] = ( x[ nodeID ] * x[ nodeID + 1 ] * nodeParams[ 0 ]  ) / N - nodeParams[ 1 ] * x[ nodeID + 1 ];
        dx[ nodeID + 2 ] = nodeParams[ 1 ] * x[ nodeID + 1 ];
    }
    
    // ----- Constructor -----
    SIRdynamic( double tmpBeta, double tmpLambda ){
        beta    = tmpBeta;
        lambda  = tmpLambda;
    }
};

// ======================================================================================================================
// ======================================================================================================================

int main( void ){
    
    /*
     --- Daten Anordnung ---
     From	To	Amount	Date
     560739	254569	7	2682
     913338	356536	1	3497
     */
        
    string edgeListSource   = "TradingData.txt";
    string lemonFileTmp     = "DataTMP.lgf";
    string lemonFileOut     = "DataProcessed.lgf";
    
    boost::unordered_set< unsigned int >                        uniqueNodes; // Mathematische Menge (set) der Knoten
    boost::unordered_set< pair< unsigned int, unsigned int > >  uniqueArcs;  // Menge der Kanten (set von Knoten-Paaren)
    
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
        
        uniqueNodes.insert( from );
        uniqueNodes.insert( to );
        uniqueArcs.insert( make_pair( from, to ) );
    }
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
    
    
    cout << "LEMON - Nodes: " << countNodes( myCulmiGraph ) << " Arcs: " << countArcs( myCulmiGraph ) << endl;
    
    // Ausgabe der neuen LEMON IDs für Knoten und Kanten
    digraphWriter( myCulmiGraph, lemonFileOut)
    .nodeMap( "name", nameNodeMap )
    .arcMap( "from", fromNdArcMap )
    .arcMap( "to" , toNdArcMap )
    .run();
    
    // =========================================================================================
    // Create HashMap of time and active arc pairs
    
    
    // Eine Map, die original KnotenIDs zu LEMON IDs zuweist (LemonMap Umkehr, statisch)
    // Nötig, da ich die orginalIDs aus der Ursprungsdatei einlese
    for (ListDigraph::NodeIt n( myCulmiGraph ) ; n!=INVALID; ++n) {
        origIDtoLemon[ nameNodeMap[ n ] ] = myCulmiGraph.id( n );
    }
        
    myEdgeListFile.close();
    myEdgeListFile.open( edgeListSource );
    getline(myEdgeListFile, foo);    
    
    // Einlesen der Werte in die Menge. Paar-Erstellung.
    while ( myEdgeListFile.good() ){
        
        myEdgeListFile >> from;
        myEdgeListFile >> to;
        myEdgeListFile >> amount;
        myEdgeListFile >> day;
        
        (dayActivityArcIDs[ day ]).push_back( findArc( myCulmiGraph,  myCulmiGraph.nodeFromId( origIDtoLemon[ from ] ), 
                                                      myCulmiGraph.nodeFromId( origIDtoLemon[ to   ] ) ) );
    }
    
    // =========================================================================================
    // ==================    Dies art von Map kann über alle true Werte iterieren   ============
    // 
    IterableBoolMap< ListDigraph, ListDigraph::Arc >      myItBoolMap( myCulmiGraph, false );
    
    // =========================================================================================
    // =====================    Creating the NetEvo System   ==================
    // 
    
    System mapSys, odeSys;
    ActiveArcsInteractionMap interaction( myItBoolMap );
    SIRdynamic odeDyn( 10, 0.1 );
    
    mapSys.addNodeDynamic( &interaction );
    mapSys.copyDigraph( myCulmiGraph, "InteractionMap", "NoArcDynamic");
    
    odeSys.addNodeDynamic( & odeDyn );
    odeSys.copyDigraph( myCulmiGraph, "SIRdynamic", "NoArcDynamic");
    
    SimObserverToStream coutObserver(cout);
    SimObserver         nullObserver;
    ChangeLog           nullLogger;
    
    SimulateMap         simMap;
    SimulateOdeFixed    simODE( RK_4, 0.01 );
    State initial = State( mapSys.totalStates(), 1.0) ;
    initDegreeDistributed( mapSys, initial, 3, 10.0);
    
    Timer t;
    
    cout << "Days to calculate: " << dayActivityArcIDs.size() << endl;
    cout << "Starte Simulation!" << endl;
    t.restart();
    for( int i = 1; i < dayActivityArcIDs.size() ; i++){
        setBoolMapOfDay( myItBoolMap, dayActivityArcIDs, i );
        //        cout << "Zeit: " << i << " Aktive Kanten: " << myItBoolMap.trueNum() << endl  << " ODE:" << endl;
        simODE.simulate( odeSys, 1, initial, nullObserver, nullLogger);
        simMap.simulate( mapSys, 1, initial, nullObserver, nullLogger);
    } 
    cout << "Laufzeit: " << t.realTime() << endl;
}


// =========================================================================================
// =========================================================================================
// =========================================================================================




void setBoolMapOfDay( IterableBoolMap< ListDigraph, ListDigraph::Arc> &localItBoolMap, timeToActiveArcs &dayActiveArcs , unsigned int currentDay){
    localItBoolMap.setAll( false );
    //Test, ob currentDay in der Map drin ist
    
    for( auto iter =  (dayActiveArcs[ currentDay ]).begin(); iter != (dayActiveArcs[ currentDay ]).end(); iter++  ){
        localItBoolMap.set( *iter , true );
    }
}


void initDegreeDistributed( System &sys, State &initial, int states, double degreeFactor ){
    int nodeNr = 0;     // Index des Knoten, um keinen Iterator zu benutzen
    
    for ( System::NodeIt n (sys); n != INVALID; ++n ){
        unsigned int degree = 0;
        for ( System::InArcIt a( sys, n ); a != INVALID; ++a) {
            degree++;
        }
        //        initial[ (nodeNr * states ) ] = degree * degreeFactor + 10;   //Anzahl des Grades 
        initial[ (nodeNr * states ) ] = degree * degreeFactor;   //Anzahl des Grades 
        nodeNr++;
    }
    //    for_each( initial.begin(), initial.end(),printCout );
}





