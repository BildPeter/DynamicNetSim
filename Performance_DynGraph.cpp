//
//  Performance_DynGraph.cpp
//  DynamicNetSim
//
//  Created by Peter on 09.01.13.
//  Copyright (c) 2013 Peter. All rights reserved.
//

#include "Performance_DynGraph.h"


#include <unistd.h>

#include <netevo.h>
#include <lemon/smart_graph.h>
#include <lemon/lgf_reader.h>
#include <lemon/time_measure.h>
#include <boost/unordered_map.hpp>
#include <peter/temporal_graph_handler.h>

using namespace std;
using namespace lemon;
using namespace netevo;


int main(){
    
//    string  schweinLGF      = "/Users/sonneundasche/Documents/FLI/DATA/03 Daten - Schwein/porkNEW_.lgf";
//    string  schweinDynArcs  = "/Users/sonneundasche/Documents/FLI/DATA/03 Daten - Schwein/porkNEW__time_tmpArcIDs.txt" ;
    string  schaf           = "/Users/sonneundasche/Documents/FLI/DATA/02 Daten - Schaf/Schaf_NEW_.lgf";
    string  schafDynAcrs    = "/Users/sonneundasche/Documents/FLI/DATA/02 Daten - Schaf/Schaf_NEW__time_tmpArcIDs.txt";
    
    vector< int >                                             activeTimes;
    boost::unordered_map< int, vector< System::Arc > >        mTime_Arcs_Vec;
    System                      mSysDyn, mSysMap;
    System::NodeMap<bool>             activeNodes( mSysDyn, false);
    System::ArcMap<bool>              activeArcs(  mSysDyn, false);
    Timer T(true);
    
    cout << "Start" << endl;
    //---------------------------------------------------------------------------------
    //                      SETUP THE SYSTEM
    //---------------------------------------------------------------------------------
    // --- read the graphs
        digraphReader( mSysDyn, schaf)
        .run();
        digraphReader( mSysMap, schaf)
        .run();

    
    // -- Assign the ODE dynamics
    SIRdynamic  mDyn( 0.6, 0.7);
    mSysDyn.addNodeDynamic( &mDyn);
    mSysDyn.setNodeDynamic_all( "SIRdynamic" );
    
    // --- Assign the map function
    ActiveArcsInteractionMap   mInterAct( activeArcs );
    mSysMap.addNodeDynamic( &mInterAct );
    mSysMap.setNodeDynamic_all( "InteractionMap" );
    
    // --- Observer
    SimObserverToStream coutObserver(cout);
    SimObserver         nullObserver;
    ChangeLog           nullLogger;
    
    // --- Simulators
    SimulateMap         simMap;
    SimulateOdeFixed    simODE( RK_4, 0.01 );
    State initial = State( mSysDyn.totalStates(), 0.1) ;
    for ( double &x : initial ) { x = 1; }

    // --- read the active arcs and time
    activeTimes = tempGR::readTemporalArcList<netevo::System>( mSysDyn, mTime_Arcs_Vec, schafDynAcrs);
    
    
    cout << "read Time: " << T.realTime() << endl;
    cout << "nodes: " << countNodes( mSysDyn ) << " Arcs: " << countArcs( mSysDyn ) << endl;
    
    //==================================================================================
    //==================================================================================
    
    tempGR::temporalGraphActivator<netevo::System>        mActivator( mSysDyn, activeNodes, activeArcs, mTime_Arcs_Vec );
    
    int calSteps = 0;
    
    T.restart();
    
    for ( auto currentTime : activeTimes) {
        
        mActivator.activate( currentTime );
        
        mActivator.deactivate( currentTime );
        calSteps++;
    }
    
    cout << "calculation Time: " << T.realTime() << endl;
    cout << "calculation Steps: " << calSteps << endl;
}
