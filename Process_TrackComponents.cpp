//
//  Process_TrackComponents.cpp
//  DynamicNetSim
//
//  Created by Peter A. Kolski on 13.02.13.
//  Copyright (c) 2013 Peter A. Kolski. All rights reserved.
//


/*
 AIM:
 Create an aggregated graph through successive timesteps. 
 Track the number of components of the graph.
 Abbort when only one closed component is create.
 save the evolution of components and the final graph + degrees
 
 INPUT:     
            2 files - LGF and tempEdges
            Initial Time
 OUTPUT:     
            Timeseries of Components (init time in filename)
            NodeList with degree
            Final graph (LGF)
 
 TODO:

 */

#include <lemon/smart_graph.h>
#include <lemon/arg_parser.h>
#include <lemon/maps.h>
#include <lemon/lgf_reader.h>
#include <lemon/lgf_writer.h>
#include <lemon/connectivity.h>

#include <peter/temporal_graph_handler.h>
#include <boost/unordered_map.hpp>

#include <fstream>

using namespace lemon;
using namespace std;

int main(int argc, char** argv){
    
    // ------------------------------
    // --- INPUT - OUTPUT
    string  sourceLGF, sourceTempEdge, target;
    int     initTime = 0;
    
    ArgParser       ap( argc, argv);
    ap.refOption("n", "Target Filename", target, true);
    ap.refOption("i", "Initial time to start from", initTime);  // NOT mandatory
    ap.refOption("g", "Graph file of LEMON GRAPH FORMAT", sourceLGF, true);
    ap.refOption("t", "Temporal active edges", sourceTempEdge, true);
    ap.parse();
    
//    sourceLGF       = "/Users/sonneundasche/Documents/FLI/DATA/02Sheep/Schaf_NEW.lgf";
//    sourceTempEdge  = "/Users/sonneundasche/Documents/FLI/DATA/02Sheep/Schaf_NEW_time_tmpArcIDs.txt";

//    sourceLGF       = "/Users/sonneundasche/Documents/FLI/DATA/SheepBigClusters_4thres.lfg";
//    sourceTempEdge  = "/Users/sonneundasche/Documents/FLI/DATA/SheepBigClusters_4thres_tempArcs.txt.dynEdges";
//    
    // ------------------------------
    // --- Graph creation
    cout << "- READING\n";
    SmartDigraph                    mGraph;
    try {
        digraphReader(mGraph, sourceLGF)
        .run();
    } catch (lemon::Exception) {
        cerr << "Error reading";
        exit(0);
    }

    cout << "The complete graph has " << countConnectedComponents( undirector( mGraph ) )
         << " components" << endl;
    
    // ------------------------------
    // --- Temporal Edges
    vector<int> times;
    boost::unordered_map< int,  vector<SmartDigraph::Arc > >   mTimeToArcs;
    times = tempGR::readTemporalArcList(mGraph, mTimeToArcs, sourceTempEdge);
    
    // ------------------------------
    // ---  Create the graph though active arcs
    SmartDigraph::ArcMap<bool>      actArcs(mGraph, false);
    FilterArcs<SmartDigraph>        mySubGraph(mGraph, actArcs);
    FilterArcs<SmartDigraph>::ArcMap<int>       arcAct(mySubGraph, 0);
    vector< int >   componentTrack;
    vector< int >   actArcsTrack;
    vector< int >   newArcsTrack;
    int             compNr = -2;
    int             countOldArcs = 0;
    int             countNewArcs = 0;
    
    // ------------------------------
    // ---  Process the timesteps
    // ------------------------------
    cout << "- PROCESSING\n";
    
    // --- set iterator to initial time
    vector<int>::iterator  t = times.begin();
    if (initTime < times.size()) {
        t += initTime;
    }
    else{
        cout << "\nERROR\nInitial time larger than timesteps of data.\n";
        return -1;
    }

    for (; t != times.end(); t++) {
        for ( auto arc : mTimeToArcs[ *t ] ){
            actArcs[ arc ] = true;
            arcAct[ arc ]++;    // 
        }
        // --- Nr of new Arcs
        countNewArcs = countArcs( mySubGraph );
        newArcsTrack.push_back( countNewArcs - countOldArcs );
        countOldArcs = countNewArcs;

        // --- Nr of components
        compNr = countConnectedComponents( undirector( mySubGraph ) );
        componentTrack.push_back(compNr);
        
        // --- Nr of active Arcs
        actArcsTrack.push_back( mTimeToArcs[ *t ].size() );
    }
    
    
    // ------------------------------
    // ---  Write the results
    // ------------------------------
    cout << "- WRITING\n";
    string  targetResult = target + "_results.txt";
    ofstream mFile( targetResult.c_str() );
    
    mFile <<  "time\tactiveArcs\tnewArcs\tcomponents\n";
    for (int i = 0; i<newArcsTrack.size(); i++) {
        mFile << i << "\t" << actArcsTrack[i] << "\t" << newArcsTrack[i] << "\t" << componentTrack[i] << endl;
    }
    mFile.close();

    string targetArcs = target + "_arcsActivity.txt";
    mFile.open( targetArcs.c_str() );
    mFile << "ID\tactivity\n";
    for (FilterArcs<SmartDigraph>::ArcIt a(mySubGraph); a!=INVALID; ++a) {
        mFile << mySubGraph.id( a ) << "\t" <<  arcAct[ a ] << endl;
    }
    mFile.close();
    
    
    cout << "- DONE\n";
}
