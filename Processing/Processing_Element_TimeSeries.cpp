//
//  Processing_Element_TimeSeries.cpp
//  DynamicNetSim
//
//  Created by Peter on 27.06.13.
//  Copyright (c) 2013 Peter A. Kolski. All rights reserved.
//



/*°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
 °  AIM:
 °          - Calculate values for each node / arc at each timestep (timeseries per node)
 °          - 
 °
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
    pathTarget      = "/Volumes/Augenweide/HIT_Element_Timeseries";
    
    Timer       T(true);
    // ================================================================================
    
    SmartDigraph                                    mGraph;

    // Attributes
    SmartDigraph::NodeMap< vector< int > >      newArcs( mGraph );
    SmartDigraph::NodeMap< vector< int > >      mDegree( mGraph );
    SmartDigraph::NodeMap< vector< int > >      outFlow( mGraph );
    SmartDigraph::NodeMap< vector< int > >      inFlow( mGraph );
    SmartDigraph::ArcMap< vector< int > >       arcFlow( mGraph );
    SmartDigraph::NodeMap< int >                lastGlobalDegree( mGraph, 0 );
    
    // For the temporal graph
    SmartDigraph::ArcMap< int >             amountOnArc( mGraph, 0 );
    SmartDigraph::ArcMap< int >             amountOnArc_dumped( mGraph, 0 );
    SmartDigraph::ArcMap< bool >            activeArcs( mGraph, false );
    SmartDigraph::NodeMap< bool >           activeNodes( mGraph, false );
    SubDigraph<SmartDigraph>                tempGraph( mGraph, activeNodes, activeArcs);
    
    // Second subGraph to measure the growing arcs
    SmartDigraph::ArcMap<bool>                  subArcs(mGraph, false);
    SmartDigraph::NodeMap<bool>                 subNodes( mGraph, false);
    SubDigraph<SmartDigraph>                    graphAdditive(mGraph, subNodes, subArcs);

    map<int, vector<pair<SmartDigraph::Arc, int> > > time_to_ArcWeightVec;
    int currentInFlow, currentOutFlow;
    
    
    digraphReader( mGraph, pathLFG )
    .run();
    
    tempGR::readTemporalArcListWeighted<SmartDigraph>( mGraph,  time_to_ArcWeightVec, pathActivity );
    tempGR::temporalGraphActivator<SmartDigraph>    mGraphActivator( mGraph, activeNodes, activeArcs, amountOnArc, time_to_ArcWeightVec);
    tempGR::temporalGraphActivator<SmartDigraph>    ActivatorAdditive( mGraph, subNodes, subArcs, amountOnArc_dumped, time_to_ArcWeightVec);
    
    cout << "Data reading time: \t" << T.realTime() << "\n";
    T.restart();
    
    // ================================================================================
    // === Calculate the IN/OUT Flow values
    // ================================================================================
    for ( auto timeP : time_to_ArcWeightVec){
        int time = timeP.first;
        
        mGraphActivator.activate( time );
        ActivatorAdditive.activate( time ); //this is left turned on, to let a graph grow
        
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
        
        mGraphActivator.deactivate( time);
    }
    
    // ================================================================================
    cout << "Calculation Time: \t" << T.realTime() << "\n";
    T.restart();
    string  pathDegree, pathInFlow, pathOutFlow, pathNewArcs;
    pathDegree  = pathTarget + "_degree.txt";
    pathInFlow  = pathTarget + "_inFlow.txt";
    pathOutFlow = pathTarget + "_outFlow.txt";
    pathNewArcs = pathTarget + "_newArcs.txt";
    
    ofstream    outFile_degree( pathDegree.c_str() );
    ofstream    outFile_inFlow( pathInFlow.c_str() );
    ofstream    outFile_outFlow( pathOutFlow.c_str() );
    ofstream    outFile_newArs( pathNewArcs.c_str() );

    // WRITE THE HEADER
    outFile_degree  << "time\t";
    outFile_inFlow  << "time\t";
    outFile_outFlow << "time\t";
    outFile_newArs  << "time\t";

    for (SmartDigraph::NodeIt n( mGraph ); n!=INVALID; ++n) {
        outFile_degree << mGraph.id( n ) << "\t";
        outFile_inFlow << mGraph.id( n ) << "\t";
        outFile_outFlow << mGraph.id( n ) << "\t";
        outFile_newArs << mGraph.id( n ) << "\t";
    }
    outFile_degree << "\n";
    outFile_inFlow << "\n";
    outFile_outFlow << "\n";
    outFile_newArs << "\n";

    // WRITE THE DATA
    for ( auto timeP : time_to_ArcWeightVec){
        int time = timeP.first;
        outFile_inFlow << time << "\t";
        for (SmartDigraph::NodeIt n( mGraph ); n!=INVALID; ++n) {
            outFile_degree << (mDegree[ n ])[ time ] << "\t";
            outFile_inFlow << (inFlow[ n ])[ time ] << "\t";
            outFile_outFlow << (outFlow[ n ])[ time ] << "\t";
            outFile_newArs << (newArcs[ n ])[ time ] << "\t";
        }
        outFile_degree << "\n";
        outFile_inFlow << "\n";
        outFile_outFlow << "\n";
        outFile_newArs << "\n";
    }   
    
    outFile_degree.close();
    outFile_inFlow.close();
    outFile_outFlow.close();
    outFile_newArs.close();
    
    
    cout << "Writing time: \t\t" << T.realTime() << endl;
    
}