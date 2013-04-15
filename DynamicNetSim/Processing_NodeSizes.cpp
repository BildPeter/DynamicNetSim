//
//  Processing_NodeSizes.cpp
//  DynamicNetSim
//
//  Created by Peter A. Kolski on 15.04.13.
//  Copyright (c) 2013 Peter. All rights reserved.
//

#include "Processing_NodeSizes.h"

#include <lemon/smart_graph.h>
#include <lemon/lgf_reader.h>
#include <lemon/adaptors.h>
#include <lemon/time_measure.h>

#include "temporal_graph_handler.h"
#include <map>
#include <fstream>

using namespace lemon;

int main(){

    Timer       T(true);
    // ================================================================================
    ifstream    file("/Users/sonneundasche/Dropbox/FLI/04_HIT_Transform/_node size/CountDays.txt");
    string txt;
    vector<int> days;
    int         tmpDay;
    while ( file.good() ) {
        file >> tmpDay;
        days.push_back( tmpDay);
    }
    // ================================================================================
    
    SmartDigraph                                    mGraph;
    SmartDigraph::NodeMap< unsigned long long >     BNR(    mGraph );
    SmartDigraph::NodeMap< unsigned int >           flowSum( mGraph );
    
    SmartDigraph::ArcMap< bool >                    activeArcs( mGraph );
    SmartDigraph::NodeMap< bool >                   activeNodes( mGraph );
    SmartDigraph::ArcMap< unsigned int >            amountOnArc( mGraph );
    
    SubDigraph<SmartDigraph>                        tempGraph( mGraph, activeNodes, activeArcs);
    
    map<int, vector<pair<SmartDigraph::Arc, int> > > time_to_ArcWeightVec;
    
    digraphReader( mGraph, "/Users/sonneundasche/Dropbox/FLI/04_HIT_Transform/_node size/Schwein_BL_07.lgf")
    .nodeMap("origID", BNR)
    .run();
    
    tempGR::readTemporalArcListWeighted<SmartDigraph>( mGraph,  time_to_ArcWeightVec,
                                                      "/Users/sonneundasche/Dropbox/FLI/04_HIT_Transform/_node size/Schwein_BL_07_time_tmpArcIDs_amountOnArc.txt");
    tempGR::temporalGraphActivator<SmartDigraph>    mGraphActivator( mGraph, activeNodes, activeArcs, amountOnArc, time_to_ArcWeightVec);
    
    // ================================================================================
    // To save the intermediate results
    vector< vector<int > >  resultVec;
    int dayFit = 0;
    
    // ================================================================================
    for ( auto timeP : time_to_ArcWeightVec){
        int time = timeP.first;

        mGraphActivator.activate( time);
        for (SubDigraph<SmartDigraph>::NodeIt n( tempGraph ); n!=INVALID; ++n) {
            for (SubDigraph<SmartDigraph>::InArcIt inA(tempGraph, n); inA!=INVALID; ++inA) {
                flowSum[ n ] += amountOnArc[ inA ];
            }
            for (SubDigraph<SmartDigraph>::OutArcIt outA(tempGraph, n); outA!=INVALID; ++outA) {
                flowSum[ n ] -= amountOnArc[ outA];
            }
            if (flowSum[ n ] < 0) {
                flowSum[ n ] = 0;
            }
        }
        mGraphActivator.deactivate( time);

        // Save the map into a vector at the time read from the file
        if ( time == days[ dayFit ]) {
            vector<int> interm;
            for (SmartDigraph::NodeIt n(mGraph); n!=INVALID; ++n) {
                interm.push_back( flowSum[ n ] );
            }
            
            resultVec.push_back( interm );
            dayFit++;
        }
    }
    // ================================================================================
    cout << "Calculation Time: " << T.realTime() << "\n";
    
    ofstream    outFile("/Users/sonneundasche/Dropbox/FLI/04_HIT_Transform/_node size/NodeSizes.txt");
    outFile << "BNR";
    for ( auto d : days ){
        outFile << "\t " << d;
    }
    outFile << "\n";
    

    int i = 0;
    for (SmartDigraph::NodeIt n(mGraph); n!=INVALID; ++n) {
        outFile << BNR[ n ];
        for (auto r : resultVec)
            outFile << "\t" << r[ i ];
        outFile << "\n";
        i++;
    }

    outFile.close();
    
    cout << "Full Time: " << T.realTime() << endl;
    
}