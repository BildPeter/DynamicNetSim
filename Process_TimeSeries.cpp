//
//  Process_TimeSeries.cpp
//  DynamicNetSim
//
//  Created by Peter A. Kolski on 22.05.13.
//  Copyright (c) 2013 Peter. All rights reserved.
//


/*
 AIM:
    Calculate the TimeSeries of several values.
 
 INPUT:
 2 files - LGF and tempEdges
 
 OUTPUT:
 Timeseries of 
 - Components
 - Nodes/Arcs All
 - Nodes/Arcs New
 - Nodes/Arcs Active
 - TradeVolume

 + Size biggest Component
 
 TODO:
 
 */

#include <lemon/smart_graph.h>
#include <lemon/arg_parser.h>
#include <lemon/maps.h>
#include <lemon/lgf_reader.h>
#include <lemon/lgf_writer.h>
#include <lemon/connectivity.h>
#include <lemon/time_measure.h>

#include <peter/temporal_graph_handler.h>

#include <fstream>

using namespace lemon;
using namespace std;

int main(int argc, char** argv){
    Timer   T;    
    // ------------------------------
    // --- INPUT - OUTPUT
    // ------------------------------
    SmartDigraph                    mGraph;
    string  sourceLGF, sourceTempEdge, target;
    int     initTime = 0;

    
//    ArgParser       ap( argc, argv);
//    ap.refOption("n", "Target Filename", target, true);
//    ap.refOption("i", "Initial time to start from", initTime);  // NOT mandatory
//    ap.refOption("g", "Graph file of LEMON GRAPH FORMAT", sourceLGF, true);
//    ap.refOption("t", "Temporal active edges", sourceTempEdge, true);
//    ap.parse();

    sourceLGF       = "/Volumes/Augenweide/Facebook/data/facebook_WallPosts.lgf";
    sourceTempEdge  = "/Volumes/Augenweide/Facebook/data/facebook_WallPosts_time_tmpArcIDs.txt";
    target          = "/Volumes/Augenweide/Facebook/data/fb_TimeSeries.txt";

    if ( argc < 3 ) {
        cout << "\n1.) source LGF \n2.) source dynEdges \n3.) target name\n";
        return 1;
    }else{
        sourceLGF       = argv[ 1 ];
        sourceTempEdge  = argv[ 2 ];
        target          = argv[ 3 ];
    }
        
    
    // ------------------------------
    // --- Graph creation
    // ------------------------------
    cout << "- READING\n";

    try {
        digraphReader(mGraph, sourceLGF)
        .run();
    } catch (lemon::Exception) {
        cerr << "Error reading";
        exit(0);
    }
    
    cout << "The complete graph has " << countConnectedComponents( undirector( mGraph ) )
    << " weak components\n";
    cout << countStronglyConnectedComponents( mGraph ) << "strong components\n";
    cout << "This graph is loopfree: " << loopFree( mGraph ) << endl;
    
    // ------------------------------
    // --- Temporal Edges
    // ------------------------------
    map< int,  vector<SmartDigraph::Arc > >   mTimeToArcs;
    tempGR::readTemporalArcList(mGraph, mTimeToArcs, sourceTempEdge);
    
    /* ------------------------------
        VARIABLES FOR:
     - Components
     - Nodes/Arcs All
     - Nodes/Arcs New
     - Nodes/Arcs Active
     - TradeVolume
    // ------------------------------
     */
    vector< int >   componentsAll, componentsCurrent, strongCompAll, strongCompCurrent,
                    arcsAll, arcsNew, arcsActive,
                    nodeAll, nodeNew, nodeActive,
                    tradeVolume;
    
    // For opimisation to count the difference
    int             arcsCountOld = 0;
    int             arcsCountNew = 0;
    int             nodeCountOld = 0;
    int             nodeCountNew = 0;


    
    // ------------------------------
    // ---  Create the graph though active arcs
    // ------------------------------
    SmartDigraph::ArcMap<bool>                  subArcs(mGraph, false);
    SmartDigraph::NodeMap<bool>                 subNodes( mGraph, false);
    SubDigraph<SmartDigraph>                    graphAdditive(mGraph, subNodes, subArcs);
    
    SmartDigraph::NodeMap<bool>                 currentNodes( mGraph, false);
    SmartDigraph::ArcMap<bool>                  currentArcs(mGraph, false);
    SubDigraph<SmartDigraph>                    graphCurrent( mGraph, currentNodes, currentArcs);
    
    tempGR::temporalGraphActivator<SmartDigraph>    ActivatorAdditive( mGraph, subNodes, subArcs, mTimeToArcs);
    tempGR::temporalGraphActivator<SmartDigraph>    ActivatorCurrent( mGraph, currentNodes, currentArcs, mTimeToArcs);
    
    // ------------------------------
    // ---  Process the timesteps
    // ------------------------------
    cout << "- PROCESSING\n";
    T.restart();
     
    for (int i = 0; i < mTimeToArcs.size(); i++) {
        // The elemets stay turned on
        ActivatorAdditive.activate(i);
        
        // --- Arcs
        arcsCountNew = countArcs( graphAdditive );
        arcsAll.push_back( arcsCountNew );
        arcsNew.push_back( arcsCountNew - arcsCountOld );
        arcsActive.push_back( mTimeToArcs[ i ].size() );    // cheap trick
        
        arcsCountOld = arcsCountNew; // to cal the diff

        // --- Nodes
        nodeCountNew = countNodes( graphAdditive );
        nodeAll.push_back( nodeCountNew );
        nodeNew.push_back( nodeCountNew - nodeCountOld );
        nodeCountOld = nodeCountNew; // to cal the diff
        
        // active
        ActivatorCurrent.activate(i);
        nodeActive.push_back( countNodes( graphCurrent ) );
        

        // --- Nr of components
        componentsAll.push_back( countConnectedComponents( undirector( graphAdditive ) ) );
        componentsCurrent.push_back( countConnectedComponents( undirector( graphCurrent ) ) );
        
        strongCompAll.push_back(  countStronglyConnectedComponents( graphAdditive ) );
        strongCompCurrent.push_back( countStronglyConnectedComponents( graphCurrent ) );
        
        ActivatorCurrent.deactivate(i);
    }

    cout << "Time: " << T.realTime() << endl;
    T.restart();
    
    // ------------------------------
    // ---  Write the results
    // ------------------------------
    cout << "- WRITING\n";
    ofstream mFile( target.c_str() );
    
    mFile
        << "time\t"
        << "nodesAll\t"
        << "nodesNew\t"
        << "nodesActive\t"
        << "arcsAll\t"
        << "arcsNew\t"
        << "arcsActive\t"
        << "componentsAll\t"
        << "componentsCurrent\t"
        << "strongCompAll\t"
        << "strongCompCurrent"
        << "\n";
    
    
    for (int i = 0; i < mTimeToArcs.size(); i++) {
        mFile
            <<   i
            << "\t" << nodeAll[i]       
            << "\t" << nodeNew[i]
            << "\t" << nodeActive[i]
            << "\t" << arcsAll[i]
            << "\t" << arcsNew[i]
            << "\t" << arcsActive[i]
            << "\t" << componentsAll[i]
            << "\t" << componentsCurrent[i]
            << "\t" << strongCompAll[i]
            << "\t" << strongCompCurrent[i]
            << "\n";    //EOL
    }
    mFile.close();
    
    cout << "Time: " << T.realTime() << endl;
    cout << "- DONE\n";
}
