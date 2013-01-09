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
    
    string  schweinLGF      = "/Users/sonneundasche/Documents/FLI/DATA/03 Daten - Schwein/porkNEW_.lgf";
    string  schweinDynArcs  = "/Users/sonneundasche/Documents/FLI/DATA/03 Daten - Schwein/porkNEW__time_tmpArcIDs.txt" ;
    string  schaf           = "/Users/sonneundasche/Documents/FLI/DATA/02 Daten - Schaf/Schaf_NEW__time_tmpArcIDs.txt";
    
    vector< int >                                                     activeTimes;
    boost::unordered_map< int, vector< System::Arc > >        mTime_Arcs_Vec;
    netevo::System                      mSysDyn, mSysMap;
    
    Timer T(true);
    // --- read the LEMON graph
    digraphReader( mSysDyn, schweinLGF)
    .run();
    digraphReader( mSysMap, schweinLGF)
    .run();
    
    // -- Assign the ODE dynamics
    SIRdynamic  mDyn( 0.6, 0.7);
    mSysDyn.addNodeDynamic( &mDyn);
    mSysDyn.setNodeDynamic_all( "SIRdynamic" );
    
    // --- Assign the map function
    
    
    // --- read
    activeTimes = tempGR::readTemporalArcList<netevo::System>( mSysDyn, mTime_Arcs_Vec, schweinDynArcs);
    
    cout << "read Time: " << T.realTime() << endl;
    cout << "nodes: " << countNodes( mSysDyn ) << " Arcs: " << countArcs( mSysDyn ) << endl;
    
    
    //==================================================================================
    //==================================================================================
    System::NodeMap<bool>             activeNodes( mSysDyn, false);
    System::ArcMap<bool>              activeArcs(  mSysDyn, false);
    
    tempGR::temporalGraphActivator<netevo::System>        mActivator( mSysDyn, activeNodes, activeArcs, mTime_Arcs_Vec );
    
    int calSteps = 0;
    
    T.restart();
    
    for ( auto currentTime : activeTimes) {
        
        mActivator.activate( currentTime );
        
        mActivator.deactivate( currentTime );
    }
    
    cout << "calculation Time: " << T.realTime() << endl;
    cout << "calculation Steps: " << calSteps << endl;
    cout << endl << "pause" << endl;
}
