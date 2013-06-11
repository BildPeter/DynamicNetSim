//
//  Process_Time_Cutoff.cpp
//  DynamicNetSim
//
//  Created by Peter A. Kolski on 22.05.13.
//  Copyright (c) 2013 Peter. All rights reserved.
//

/* ============================================================================
 
 AIM:
 The data should be cleaned from all edges and nodes which are not in use
 in the chosen timespan.
 
 - read all data types
 - define the timesteps which should be used
 - Subgraph of active arcs i& nodes
 - copy the graph
 - check validity of arcs in the temporal vector
 - copy the valid ones
 - write to file
 
 ==========================================================================*/

#include <iostream>

#include <lemon/smart_graph.h>
#include <lemon/lgf_reader.h>
#include <lemon/lgf_writer.h>
#include <lemon/adaptors.h>
#include <lemon/time_measure.h>

#include <peter/temporal_graph_handler.h>

using namespace lemon;

int main( int argc, char** argv ){

    string          pathSourceLGF, pathSourceTMP, pathSourceTMP_W, pathTarget;
    int             initTime, endTime;
    Timer           T;
    
    initTime        = 1000;
    endTime         = 3000;
    pathSourceLGF   = "/Users/sonneundasche/Documents/FLI/DATA/03Pork/porkNEW_.lgf";
    pathSourceTMP   = "/Users/sonneundasche/Documents/FLI/DATA/03Pork/porkNEW__time_tmpArcIDs.txt";
    pathSourceTMP_W = "/Users/sonneundasche/Documents/FLI/DATA/03Pork/porkNEW__time_tmpArcIDs_amountOnArc.txt";
    
    map< int, vector< SmartDigraph::Arc > >               Time_ToArcVec;
    map< int, vector< pair< SmartDigraph::Arc, int > > > Time_ToArcVec_Weight;
    
    SmartDigraph                        g;
    SmartDigraph::NodeMap< bool >       activeNodes( g );
    SmartDigraph::ArcMap< bool>         activeArcs( g );
    SmartDigraph::NodeMap< int >        nodeID( g );

    
    digraphReader( g, pathSourceLGF )
    .nodeMap("origID", nodeID)
    .run();
    cout << T.realTime() << endl; T.restart();
    cout << "LGF read\n";

    
    tempGR::readTemporalArcList( g, Time_ToArcVec, pathSourceTMP);
    cout << T.realTime() << endl; T.restart();
    tempGR::readTemporalArcListWeighted( g, Time_ToArcVec_Weight, pathSourceTMP_W );
    cout << T.realTime() << endl; T.restart();
    tempGR::temporalGraphActivator<SmartDigraph>    mActivator( g, activeNodes, activeArcs, Time_ToArcVec);
    cout << T.realTime() << endl; T.restart();
    cout << "TempArcs read\n";
    
    // Activate the Subgraph

    // Gib mir nicht den Wert, sondern den Iterator -
    // -> first gibt mir die Zeit
    // jetzt muss ich aber ->second aufrufen um den Vector zu erhalten
    for (int time = initTime; time < endTime ;time++){
        mActivator.activate( time );
    }
    cout << T.realTime() << endl; T.restart();
    
    
    // --------------------------------------------------------------------------------------------------
    // --- Copy the new System
    // --------------------------------------------------------------------------------------------------
    // A new graph 'f' has to be created to ensure that the LEMON IDs are correct
    
    SubDigraph<SmartDigraph>            subGraph( g, activeNodes, activeArcs);
    SmartDigraph                        f;
    SmartDigraph::ArcMap< SmartDigraph::Arc >       giveTargetArc( g );
    vector< vector < SmartDigraph::Arc > >               resultArcs;
    vector<   vector< pair< SmartDigraph::Arc, int > > > resultArcs_Weight;
    
    
    digraphCopy(subGraph, f)
    .arcRef(  giveTargetArc )
    .run();
    
    cout  << countNodes( g )    << "\tnew Nodes: " << countNodes( f ) << endl;
    cout  << countArcs( g )     << "\tnew Arcs:" << countArcs( f ) << endl;
    
    
    for (int time = initTime; time < endTime; time++) {
        vector< SmartDigraph::Arc >     mArcVec;
        for ( auto currentArc : Time_ToArcVec[ time ]) {
            // go though vector and copy valid into new one
            if ( f.valid( currentArc ) ) {
                mArcVec.push_back( giveTargetArc[ currentArc ] ); // save new Arc ID!
            }
        }
        resultArcs.push_back( mArcVec );
    }
    
    for (int time = initTime; time < endTime; time++) {
        vector< pair < SmartDigraph::Arc, int > >     mPairVec;
        for ( auto currentPair : Time_ToArcVec_Weight[ time ]) {
            // go though vector and copy valid into new one
            if ( f.valid( currentPair.first ) ) {
                mPairVec.push_back( make_pair( giveTargetArc[ currentPair.first ],  currentPair.second ) ); // new ArcID
            }
        }
        resultArcs_Weight.push_back( mPairVec );
    }

    tempGR::writeTempGraph<SmartDigraph>( f, resultArcs, "/Users/sonneundasche/Desktop//pork_FromTime1000_tempArcs.txt");
    tempGR::writeTempGraph_weighted<SmartDigraph>( f, resultArcs_Weight, "/Users/sonneundasche/Desktop/pork_FromTime1000_tempArcs_weighted.txt");
    
    digraphWriter(f, "/Users/sonneundasche/Desktop/pork_FromTime1000.lgf")
    .nodeMap("origID", nodeID)
    .run();
    
    cout << T.realTime() << endl; T.restart();
    cout << "Done\n";
    
    
}