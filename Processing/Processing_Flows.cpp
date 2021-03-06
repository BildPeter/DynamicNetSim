//
//  Processing_Flows.cpp
//  DynamicNetSim
//
//  Created by Peter A. Kolski (BildPeter Visuals) on 29.05.13.
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


/*°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
 °  AIM:
 °      - Read the ActiveArc Data with weights
 °      - Sum up IN- & OUT-Flow for each node
 °      - Save the FlowSum for specifiy timesteps (read from a file)
 °
 °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°*/


#include <peter/temporal_graph_handler.h>
#include <map>
#include <fstream>


#include <lemon/smart_graph.h>
#include <lemon/lgf_reader.h>
#include <lemon/lgf_writer.h>
#include <lemon/adaptors.h>
#include <lemon/time_measure.h>
#include <lemon/arg_parser.h>
#include <lemon/maps.h>
#include <lemon/connectivity.h>


using namespace lemon;

int main(int argc, char** argv){
    

    string          pathLFG, pathActivity, pathCountDays, pathTarget;
    pathLFG         = "/Volumes/Augenweide/HIT_Jul2006_CLEAN.lgf";
    pathActivity    = "/Volumes/Augenweide/HIT_Jul2006_CLEAN_time_tmpArcIDs_amountOnArc.txt";
    pathTarget      = "/Volumes/Augenweide/HIT_Jul2006_Flow_Daily.lgf";
    
    Timer       T(true);
    // ================================================================================
    
    SmartDigraph                                    mGraph;
    SmartDigraph::NodeMap< long long >      BNR(     mGraph );
    SmartDigraph::NodeMap< long long >      flowSum( mGraph, 0 );
    SmartDigraph::NodeMap< long long >      inFlow( mGraph, 0 );
    SmartDigraph::NodeMap< long long >      outFlow( mGraph, 0 );
    SmartDigraph::NodeMap< long long >      activeDays( mGraph, 0);
    SmartDigraph::NodeMap< int >            nodeLastDay( mGraph, -1);
    SmartDigraph::NodeMap< int >            maxInFlowPerDay( mGraph, 0 );
    SmartDigraph::NodeMap< int >            maxOutFlowPerDay( mGraph, 0 );
    SmartDigraph::NodeMap< int >            mComponents( mGraph, -1 );
    SmartDigraph::ArcMap< long long >       arcFlow( mGraph, 0 );
    SmartDigraph::ArcMap<long long >        arcDays( mGraph, 0 );
    SmartDigraph::ArcMap< int >             arcLastDay( mGraph, -1);
    SmartDigraph::ArcMap< int >             amountOnArc( mGraph, 0 );
    
    SmartDigraph::ArcMap< bool >            activeArcs( mGraph, false );
    SmartDigraph::NodeMap< bool >           activeNodes( mGraph, false );
    SubDigraph<SmartDigraph>                tempGraph( mGraph, activeNodes, activeArcs);
    
    InDegMap<SmartDigraph>                  inDegree( mGraph );
    OutDegMap<SmartDigraph>                 outDegree( mGraph );
    
    int currentInFlow, currentOutFlow;
    
    map<int, vector<pair<SmartDigraph::Arc, int> > > time_to_ArcWeightVec;
    
    digraphReader( mGraph, pathLFG )
    .nodeMap("origID", BNR)
    .run();
    
    tempGR::readTemporalArcListWeighted<SmartDigraph>( mGraph,  time_to_ArcWeightVec, pathActivity );
    tempGR::temporalGraphActivator<SmartDigraph>    mGraphActivator( mGraph, activeNodes, activeArcs, amountOnArc, time_to_ArcWeightVec);
    
    cout << "Data reading time: \t" << T.realTime() << "\n";
    T.restart();
    
    // ================================================================================
    // === Calculate the IN/OUT Flow values
    // ================================================================================
    for ( auto timeP : time_to_ArcWeightVec){
        int time = timeP.first;
        
        mGraphActivator.activate( time );
        for (SubDigraph<SmartDigraph>::NodeIt n( tempGraph ); n!=INVALID; ++n) {
            currentInFlow   = 0;
            currentOutFlow  = 0;
            activeDays[ n ]++;
            nodeLastDay[ n ] = time;
            
            for (SubDigraph<SmartDigraph>::InArcIt inA(tempGraph, n); inA!=INVALID; ++inA) {
                currentInFlow += amountOnArc[ inA ];
            }
            for (SubDigraph<SmartDigraph>::OutArcIt outA(tempGraph, n); outA!=INVALID; ++outA) {
                currentOutFlow += amountOnArc[ outA ];
            }
            
            flowSum[ n ]    += ( currentInFlow - currentOutFlow );
            inFlow[ n ]     += currentInFlow;
            outFlow[ n ]    += currentOutFlow;

            // --- set the maximum daily flow values
            if (maxInFlowPerDay[ n ] < currentInFlow) {
                maxInFlowPerDay[ n ] = currentInFlow;
            }
            
            if (maxOutFlowPerDay[ n ] < currentOutFlow ) {
                maxOutFlowPerDay[ n ] = currentOutFlow;
            }
        }
        
        
        for (SubDigraph<SmartDigraph>::ArcIt a(tempGraph); a!=INVALID; ++a) {
            arcFlow[a] += amountOnArc[ a ];
            arcDays[a]++;
            arcLastDay[ a ] = time;
        }
        
        mGraphActivator.deactivate( time);
        
    }
    // ================================================================================
    // === Components
    // ================================================================================
    
    connectedComponents( undirector( mGraph ), mComponents);
    
    // ================================================================================
    cout << "Calculation Time: \t" << T.realTime() << "\n";
    T.restart();
    
    ofstream    outFile( pathTarget.c_str() );
    
    digraphWriter( mGraph, outFile)
    .nodeMap("BNR", BNR)
    .nodeMap("activeDays", activeDays)
    .nodeMap("lastActiveDay", nodeLastDay)
    .nodeMap("outFlow", outFlow)
    .nodeMap("inFlow", inFlow)
    .nodeMap("maxInFlowDaily", maxInFlowPerDay)
    .nodeMap("maxOutFlowDaily", maxOutFlowPerDay)
    .nodeMap("flowSum", flowSum)
    .nodeMap("inDegree", inDegree)
    .nodeMap("outDegree", outDegree)
    .nodeMap("componentID", mComponents)
    .arcMap("flowSum", arcFlow)
    .arcMap("days", arcDays)
    .arcMap("lastActiveDay", arcLastDay)
    .run();
    
    outFile.close();
    
    cout << "Writing time: \t\t" << T.realTime() << endl;
    
}