//
//  Processing_NodeSize_Flows.cpp
//  DynamicNetSim
//
//  Created by Peter A. Kolski on 27.05.13.
//  Copyright (c) 2013 Peter. All rights reserved.
//


/*°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
 °  AIM:
 °      - Read the ActiveArc Data with weights
 °      - Sum up IN- & OUT-Flow for each node
 °
 °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°*/


#include "temporal_graph_handler.h"
#include <map>
#include <fstream>

#include <lemon/smart_graph.h>
#include <lemon/lgf_reader.h>
#include <lemon/lgf_writer.h>
#include <lemon/time_measure.h>



using namespace lemon;

int main(int argc, char** argv){
    string      pathLFG, pathActivity, pathCountDays, pathTarget, pathTargetLGF;
    int         timeSteps = 0;

    pathLFG         = "/Users/sonneundasche/Documents/FLI/DATA/03Pork/porkNEW_.lgf";
    pathActivity    = "/Users/sonneundasche/Documents/FLI/DATA/03Pork/porkNEW__time_tmpArcIDs_amountOnArc.txt";
    pathTargetLGF   = "/Users/sonneundasche/Dropbox/FLI/DATA/04_HIT_Transform/flow/pork_FromTime1000_FlowsC.lgf";
    
//    pathLFG         = "/Users/sonneundasche/Dropbox/FLI/DATA/04_HIT_Transform/pork_FromTime1000.lgf";
//    pathActivity    = "/Users/sonneundasche/Dropbox/FLI/DATA/04_HIT_Transform/pork_FromTime1000_tempArcs_weighted.txt";
//    pathTargetLGF   = "/Users/sonneundasche/Dropbox/FLI/DATA/04_HIT_Transform/flow/pork_FromTime1000_FlowsB.lgf";

    
    Timer       T(true);
    
    SmartDigraph                            mGraph;
    SmartDigraph::NodeMap< long long >      BNR(    mGraph );
    SmartDigraph::NodeMap< long long >      flowSum( mGraph );
    SmartDigraph::NodeMap< long long >      inFlow( mGraph );
    SmartDigraph::NodeMap< long long >      outFlow( mGraph );
    SmartDigraph::ArcMap< long long >       arcFlow( mGraph );
    
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
    // === Calculate the IN/OUT Flow values
    // ================================================================================
    cout << "Start calculation!\n";
    for ( auto timeP : time_to_ArcWeightVec){
        int time = timeP.first;
        
        if (time > 1000 ) {
        
        mGraphActivator.activate( time );
        for (SubDigraph<SmartDigraph>::NodeIt n( tempGraph ); n!=INVALID; ++n) {
            for (SubDigraph<SmartDigraph>::InArcIt inA(tempGraph, n); inA!=INVALID; ++inA) {
                flowSum[ n ] += amountOnArc[ inA ];
                inFlow[ n ] += amountOnArc[ inA ];
            }
            for (SubDigraph<SmartDigraph>::OutArcIt outA(tempGraph, n); outA!=INVALID; ++outA) {
                flowSum[ n ] -= amountOnArc[ outA ];
                outFlow[ n ] += amountOnArc[ outA ];
            }
        }
        for (SubDigraph<SmartDigraph>::ArcIt arc( tempGraph ); arc!=INVALID; ++arc) {
            arcFlow[ arc ] += amountOnArc[ arc ];
        }
        mGraphActivator.deactivate( time);
        timeSteps++;
        }
    }
    // ================================================================================
    cout << "Calculation Time: " << T.realTime() << "\n";
    
    
    ofstream out2File( pathTargetLGF.c_str() );
    
    out2File    << "Addition of Maps on the in- and out-Flow amount. Full NodeSet. \nTimeSet: Larger than step 1000\n"
                << "resulting timesteps: " << timeSteps << "\n\n";
    
    digraphWriter( mGraph, out2File )
    .nodeMap("origID", BNR)
    .nodeMap("inFlow", inFlow)
    .nodeMap("outFlow", outFlow)
    .nodeMap("sumFlow", flowSum)
    .arcMap("flow", arcFlow)
    .run();
    
    out2File.close();
    
    
    cout << "Full Time: " << T.realTime() << endl;
    
}