//
//  Processing.h
//  DynamicNetSim
//
//  Created by Peter on 27.06.13.
//  Copyright (c) 2013 Peter A. Kolski. All rights reserved.
//

#ifndef __DynamicNetSim__Processing__
#define __DynamicNetSim__Processing__

#include <iostream>

//
//  Processing.h
//  DynamicNetSim
//
//  Created by Peter on 27.06.13.
//  Copyright (c) 2013 Peter A. Kolski. All rights reserved.
//

#ifndef DynamicNetSim_Processing_h
#define DynamicNetSim_Processing_h

#include <iostream>
#include <lemon/smart_graph.h>
#include <lemon/lgf_reader.h>
#include <lemon/lgf_writer.h>
#include <lemon/adaptors.h>
#include <lemon/time_measure.h>
#include <lemon/arg_parser.h>
#include <lemon/maps.h>
#include <lemon/connectivity.h>

#include <peter/temporal_graph_handler.h>

using namespace lemon;
using namespace std;

class ElementWise_TimeSeries {
    SmartDigraph                                    mGraph;
    
    // Attributes
    SmartDigraph::NodeMap< vector< int > >      newArcs;
    SmartDigraph::NodeMap< vector< int > >      mDegree;
    SmartDigraph::NodeMap< vector< int > >      outFlow;
    SmartDigraph::NodeMap< vector< int > >      inFlow;
    SmartDigraph::ArcMap< vector< int > >       arcFlow;
    SmartDigraph::NodeMap< int >                lastGlobalDegree;
    
    // For the temporal graph
    SmartDigraph::ArcMap< int >             amountOnArc;
    SmartDigraph::ArcMap< int >             amountOnArc_dumped;
    SmartDigraph::ArcMap< bool >            activeArcs;
    SmartDigraph::NodeMap< bool >           activeNodes;
    SubDigraph<SmartDigraph>                tempGraph;
    
    // Second subGraph to measure the growing arcs
    SmartDigraph::ArcMap<bool>                  subArcs;
    SmartDigraph::NodeMap<bool>                 subNodes;
    SubDigraph<SmartDigraph>                    graphAdditive;
    
    map<int, vector<SmartDigraph::Arc> >                time_ToArcVec;
    map<int, vector<pair<SmartDigraph::Arc, int> > >    time_to_ArcWeightVec;
    int currentInFlow, currentOutFlow;
    
    tempGR::temporalGraphActivator<SmartDigraph>    *mGraphActivator;
    tempGR::temporalGraphActivator<SmartDigraph>    *ActivatorAdditive;

    bool            mWeighted;
    string          pathDegree, pathInFlow, pathOutFlow, pathNewArcs;
    
public:
    ElementWise_TimeSeries( )
    :newArcs( mGraph ),
    mDegree( mGraph ),
    outFlow( mGraph ),
    inFlow( mGraph ),
    arcFlow( mGraph ),
    lastGlobalDegree( mGraph, 0 ),
    
    amountOnArc( mGraph ),
    amountOnArc_dumped( mGraph ),
    activeArcs( mGraph ),
    activeNodes( mGraph ),
    tempGraph( mGraph, activeNodes, activeArcs ),
    
    subNodes( mGraph ),
    subArcs( mGraph ),
    graphAdditive( mGraph, subNodes, subArcs )
    {}
    
    void readGraph( string LGF, string tempEdges, bool weighted );
    void process();
    void writeData( string targetPath );
    
};


void ElementWise_TimeSeries::readGraph( string LGF, string tempEdges, bool weighted ){
    mWeighted = weighted;
    digraphReader( mGraph, LGF )
    .run();
    
    if (weighted) {
        tempGR::readTemporalArcListWeighted<SmartDigraph>( mGraph,  time_to_ArcWeightVec, tempEdges );
        mGraphActivator       = new tempGR::temporalGraphActivator<SmartDigraph>( mGraph, activeNodes, activeArcs, amountOnArc, time_to_ArcWeightVec);
        ActivatorAdditive     = new tempGR::temporalGraphActivator<SmartDigraph>( mGraph, subNodes, subArcs, amountOnArc_dumped, time_to_ArcWeightVec);
    }
    else{
        tempGR::readTemporalArcList(mGraph, time_ToArcVec, tempEdges);
        mGraphActivator       = new tempGR::temporalGraphActivator<SmartDigraph>( mGraph, activeNodes, activeArcs, time_ToArcVec);
        ActivatorAdditive     = new tempGR::temporalGraphActivator<SmartDigraph>( mGraph, subNodes, subArcs, time_ToArcVec);
    }
}

void ElementWise_TimeSeries::process(){
    if( mWeighted){
        for ( auto timeP : time_to_ArcWeightVec){
            int time = timeP.first;
            
            mGraphActivator->activate( time );
            ActivatorAdditive->activate( time ); //this is left turned on, to let a graph grow
            
            // --- NODES ---
            for (SubDigraph<SmartDigraph>::NodeIt n( tempGraph ); n!=INVALID; ++n) {
                currentInFlow   = 0;
                currentOutFlow  = 0;
                
                for (SubDigraph<SmartDigraph>::InArcIt inA(tempGraph, n); inA!=INVALID; ++inA) {
                    currentInFlow += amountOnArc[ inA ];
                }
                for (SubDigraph<SmartDigraph>::OutArcIt outA(tempGraph, n); outA!=INVALID; ++outA) {
                    currentOutFlow += amountOnArc[ outA ];
                }
                
                inFlow[ n ].push_back( currentInFlow );
                outFlow[ n ].push_back( currentOutFlow);
                mDegree[ n ].push_back( ( countInArcs( tempGraph, n) + countOutArcs( tempGraph, n ) ));
                
                // Nr. of new arcs is cal, when the additive graph has a higher degree.
                int currentDegree = countInArcs( graphAdditive, n) + countOutArcs( graphAdditive, n );
                newArcs[ n ].push_back( currentDegree - lastGlobalDegree[ n ] );
                lastGlobalDegree[ n ] = currentDegree;
            }
            
            
            // --- ARCS ---
            for (SubDigraph<SmartDigraph>::ArcIt a(tempGraph); a!=INVALID; ++a) {
                arcFlow[a].push_back( amountOnArc[ a ] );
            }
            
            mGraphActivator->deactivate( time);
        }
    }
    else{
        for ( int time  = 0; time < time_ToArcVec.size(); time++){
            
            mGraphActivator->activate( time );
            ActivatorAdditive->activate( time ); //this is left turned on, to let a graph grow
            
            // --- NODES ---
            for (SubDigraph<SmartDigraph>::NodeIt n( tempGraph ); n!=INVALID; ++n) {
                mDegree[ n ].push_back( ( countInArcs( tempGraph, n) + countOutArcs( tempGraph, n ) ));
                
                // Nr. of new arcs is cal, when the additive graph has a higher degree.
                int currentDegree = countInArcs( graphAdditive, n) + countOutArcs( graphAdditive, n );
                newArcs[ n ].push_back( currentDegree - lastGlobalDegree[ n ] );
                lastGlobalDegree[ n ] = currentDegree;
            }
            
            // --- ARCS ---
            for (SubDigraph<SmartDigraph>::ArcIt a(tempGraph); a!=INVALID; ++a) {
                arcFlow[a].push_back( amountOnArc[ a ] );
            }
            mGraphActivator->deactivate( time);
        }
    }
}

void ElementWise_TimeSeries::writeData(string targetPath ){
    string  pathDegree, pathInFlow, pathOutFlow, pathNewArcs;
    
    if ( mWeighted ) {
        pathInFlow  = targetPath + "_inFlow.txt";
        pathOutFlow = targetPath + "_outFlow.txt";

        ofstream    outFile_inFlow( pathInFlow.c_str() );
        ofstream    outFile_outFlow( pathOutFlow.c_str() );
        
        // WRITE THE HEADER
        outFile_inFlow  << "time\t";
        outFile_outFlow << "time\t";

        for (SmartDigraph::NodeIt n( mGraph ); n!=INVALID; ++n) {
            outFile_inFlow << mGraph.id( n ) << "\t";
            outFile_outFlow << mGraph.id( n ) << "\t";
        }
        outFile_inFlow << "\n";
        outFile_outFlow << "\n";

        // WRITE THE DATA
        for ( auto timeP : time_to_ArcWeightVec){
            int time = timeP.first;
            outFile_inFlow << time << "\t";
            outFile_outFlow << time << "\t";
            for (SmartDigraph::NodeIt n( mGraph ); n!=INVALID; ++n) {
                outFile_inFlow << (inFlow[ n ])[ time ] << "\t";
                outFile_outFlow << (outFlow[ n ])[ time ] << "\t";
            }
            outFile_inFlow << "\n";
            outFile_outFlow << "\n";
        }

        
        outFile_inFlow.close();
        outFile_outFlow.close();

    }
    
    pathDegree  = targetPath + "_degree.txt";
    pathNewArcs = targetPath + "_newArcs.txt";
    
    ofstream    outFile_degree( pathDegree.c_str() );
    ofstream    outFile_newArs( pathNewArcs.c_str() );
    
    // WRITE THE HEADER
    outFile_degree  << "time\t";
    outFile_newArs  << "time\t";
    
    for (SmartDigraph::NodeIt n( mGraph ); n!=INVALID; ++n) {
        outFile_degree << mGraph.id( n ) << "\t";
        outFile_newArs << mGraph.id( n ) << "\t";
    }
    outFile_degree << "\n";
    outFile_newArs << "\n";
    
    // WRITE THE DATA
    for ( auto timeP : time_to_ArcWeightVec){
        int time = timeP.first;
        outFile_degree << time << "\t";
        outFile_newArs << time << "\t";
        for (SmartDigraph::NodeIt n( mGraph ); n!=INVALID; ++n) {
            outFile_degree << (mDegree[ n ])[ time ] << "\t";
            outFile_newArs << (newArcs[ n ])[ time ] << "\t";
        }
        outFile_degree << "\n";
        outFile_newArs << "\n";
    }
    
    outFile_degree.close();
    outFile_newArs.close();
    

}
#endif


#endif /* defined(__DynamicNetSim__Processing__) */
