//
//  Process_Time_Cutoff.cpp
//  DynamicNetSim
//
//  Created by Peter A. Kolski on 22.05.13.
//  Copyright (c) 2013 Peter. All rights reserved.
//

#include "Process_Time_Cutoff.h"
#include <lemon/smart_graph.h>
#include <lemon/lgf_reader.h>
#include <lemon/lgf_writer.h>

#include <peter/temporal_graph_handler.h>

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


using namespace lemon;

int main( int argc, char** argv ){

    string          pathSourceLGF, pathSourceTMP, pathSourceTMP_W, pathTarget;
    
    map< int, vector< pair< SmartDigraph::Arc, int > > > Time_ToArcVec_Weight;
    map< int, vector<SmartDigraph::Arc > >               Time_ToArcVec;
    vector< vector < SmartDigraph::Arc > >                resultArcs;
    
    
    SmartDigraph    g;
    
    digraphReader( g, pathSourceLGF )
    .run();
    
    tempGR::readTemporalArcList( g, Time_ToArcVec_Weight, pathSourceTMP)
    tempGR::readTemporalArcListWeighted( g, Time_ToArcVec_Weiggt, pathSourceTMP );
    
}