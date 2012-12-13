//
//  temporal_graph_handler.h
//  DynamicNetSim
//
//  Created by Peter on 13.12.12.
//  Copyright (c) 2012 Peter. All rights reserved.
//

#ifndef temporal_graph_handler_h
#define temporal_graph_handler_h

#include <lemon/smart_graph.h>
#include <lemon/adaptors.h>

#include <unordered_map>
#include <fstream>

using namespace std;
using namespace lemon;

namespace tempGR {

    template < typename DGR >
    vector<int> readTemporalArcList( DGR &graph, unordered_map< int, vector<typename DGR::Arc > > &Time_ToArcVec, string fn){
        vector< int > activeTimes;
        ifstream input( fn );
        
        int     time, arcID;
        string tmpData;
        getline( input, tmpData );  // Kill first line (header)
        for (string line; getline(input, line, '\n'); ) {
            istringstream lineStream(line);
            lineStream >> time;     // -- read first number ---
            activeTimes.push_back( time );
            
            for (; getline(lineStream, tmpData, '\t'); ) {
                lineStream >> arcID;
                Time_ToArcVec[ time ].push_back( graph.arcFromId( arcID ) );
            }
            Time_ToArcVec[time].pop_back();    // -- do not really know why, seems to read last element twice (not endet be \t)
        }
        return activeTimes;
    }
    

    template < typename DGR >
    class temporalGraphActivator{
        DGR     &mGraph;
        typename DGR::template NodeMap<bool>     &mActiveNodes;
        typename DGR::template ArcMap<bool>      &mActiveArcs;
        unordered_map< int, vector<typename DGR::Arc > > &mTime_ToArcVec;
        
    public:
        temporalGraphActivator( DGR &graph, typename DGR::template NodeMap<bool> &activeNodes, typename DGR::template ArcMap<bool> &activeArcs, unordered_map< int, vector<typename DGR::Arc > > &Time_ToArcVec )
        :   mGraph(graph),
            mActiveNodes(activeNodes),
            mActiveArcs(activeArcs),
            mTime_ToArcVec(Time_ToArcVec)
        {}
        
        void activate( int time ){
            for ( auto arc : mTime_ToArcVec[ time ] ){
                mActiveArcs[ arc ]                   = true;
                mActiveNodes[ mGraph.source( arc ) ] = true;
                mActiveNodes[ mGraph.target( arc ) ] = true;
            }
        }
        
        void deactivate( int time ){
            for ( auto arc : mTime_ToArcVec[ time ] ){
                mActiveArcs[ arc ]                   = false;
                mActiveNodes[ mGraph.source( arc ) ] = false;
                mActiveNodes[ mGraph.target( arc ) ] = false;
            }
        }
    };

}
#endif
