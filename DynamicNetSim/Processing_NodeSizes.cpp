//
//  Processing_NodeSizes.cpp
//  DynamicNetSim
//
//  Created by Peter A. Kolski on 15.04.13.
//  Copyright (c) 2013 Peter A. Kolski. All rights reserved.
//

/*°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
 °  AIM: 
 °      - Read the ActiveArc Data with weights
 °      - Sum up IN- & OUT-Flow for each node
 °      - Save the FlowSum for specifiy timesteps (read from a file)
 °
 °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°*/


#include "temporal_graph_handler.h"
#include <map>
#include <fstream>


#include <lemon/smart_graph.h>
#include <lemon/lgf_reader.h>
#include <lemon/adaptors.h>
#include <lemon/time_measure.h>
#include <lemon/arg_parser.h>

#ifdef TERMINAL
#define INPUT "Terminal"
#else
#define INPUT "none"
#endif



using namespace lemon;

int main(int argc, char** argv){

    ArgParser   ag( argc, argv);
    string      pathLFG, pathActivity, pathCountDays, pathTarget;

            std::cout << INPUT << "\n";
    
    if (INPUT == "Terminal") {
        ag
        .parse();
    }else {
        pathLFG         = "/Users/sonneundasche/Desktop/Pork_08-09_.lgf";
        pathActivity    = "/Users/sonneundasche/Desktop/Pork_08-09__time_tmpArcIDs_amountOnArc.txt";
        pathCountDays   = "/Users/sonneundasche/Dropbox/FLI/DATA/04_HIT_Transform/_node size/CountDays.txt";
        pathTarget      = "/Users/sonneundasche/Desktop/NodeSizes_08-09.txt";

//        pathLFG         = "/Users/sonneundasche/Dropbox/FLI/04_HIT_Transform/_node size/Schwein_BL_07.lgf";
//        pathActivity    = "/Users/sonneundasche/Dropbox/FLI/04_HIT_Transform/_node size/Schwein_BL_07_time_tmpArcIDs_amountOnArc.txt";
//        pathCountDays   = "/Users/sonneundasche/Dropbox/FLI/04_HIT_Transform/_node size/CountDays.txt";
//        pathTarget      = "/Users/sonneundasche/Dropbox/FLI/04_HIT_Transform/_node size/NodeSizes.txt";
    }
    
    exit(0);
    
    Timer       T(true);
    // ================================================================================
    // ===  Read a file, which dates to save the FlowValues
    // ================================================================================
    ifstream    file( pathCountDays.c_str() );
    string txt;
    vector<int> days;
    int         tmpDay;
    while ( file.good() ) {
        file >> tmpDay;
        days.push_back( tmpDay);
    }
    // ================================================================================
    
    SmartDigraph                                    mGraph;
    SmartDigraph::NodeMap< long long >     BNR(    mGraph );
    SmartDigraph::NodeMap< long long >     flowSum( mGraph );
    
    SmartDigraph::ArcMap< bool >                    activeArcs( mGraph );
    SmartDigraph::NodeMap< bool >                   activeNodes( mGraph );
    SmartDigraph::ArcMap< unsigned int >            amountOnArc( mGraph );
    
    SubDigraph<SmartDigraph>                        tempGraph( mGraph, activeNodes, activeArcs);
    
    map<int, vector<pair<SmartDigraph::Arc, int> > > time_to_ArcWeightVec;
    
    digraphReader( mGraph, pathLFG )
    .nodeMap("origID", BNR)
    .run();
    
    tempGR::readTemporalArcListWeighted<SmartDigraph>( mGraph,  time_to_ArcWeightVec, pathActivity );
    tempGR::temporalGraphActivator<SmartDigraph>    mGraphActivator( mGraph, activeNodes, activeArcs, amountOnArc, time_to_ArcWeightVec);
    
    // ================================================================================
    // To save the intermediate results
    vector< vector< long long > >  resultVec;
    int dayFit = 0;
    
    // ================================================================================
    // === Calculate the IN/OUT Flow values
    // ================================================================================
    for ( auto timeP : time_to_ArcWeightVec){
        int time = timeP.first;

        mGraphActivator.activate( time );
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
            vector<long long> interm;
            for (SmartDigraph::NodeIt n(mGraph); n!=INVALID; ++n) {
                interm.push_back( flowSum[ n ] );
            }
            
            resultVec.push_back( interm );
            dayFit++;
        }
    }
    // ================================================================================
    cout << "Calculation Time: " << T.realTime() << "\n";
    
    ofstream    outFile( pathTarget.c_str() );
    outFile << "BNR";
    for ( auto d : days ){
        outFile << "\t " << d;
    }
    outFile << "\n";
    

    int i = 0;
    for (SmartDigraph::NodeIt n(mGraph); n!=INVALID; ++n) {
        outFile << BNR[ n ];
        for (vector< long long > r : resultVec)
            outFile << "\t" << r[ i ];
        outFile << "\n";
        i++;
    }

    outFile.close();
    
    cout << "Full Time: " << T.realTime() << endl;
    
}