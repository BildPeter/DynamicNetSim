//
//  Process_TrackComponents.cpp
//  DynamicNetSim
//
//  Created by Peter A. Kolski on 13.02.13.
//  Copyright (c) 2013 Peter. All rights reserved.
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
    
//    ArgParser       ap( argc, argv);
//    ap.refOption("i", "Initial time to start from", initTime);  // NOT mandatory
//    ap.refOption("g", "Graph file of LEMON GRAPH FORMAT", sourceLGF, true);
//    ap.refOption("t", "Temporal active edges", sourceTempEdge, true);
//    ap.refOption("n", "Target filename", target, true);
//    ap.parse();
    
    sourceLGF       = "/Users/sonneundasche/Documents/FLI/DATA/03 Daten - Schwein/porkNEW_.lgf";
    sourceTempEdge  = "/Users/sonneundasche/Documents/FLI/DATA/03 Daten - Schwein/porkNEW__time_tmpArcIDs.txt";
    target          = "Test";
    
    // ------------------------------
    // --- Graph creation
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
//    int timeSteps = times.size();
    
    // ------------------------------
    // ---  Create the graph though active arcs
    SmartDigraph::ArcMap<bool>      actArcs(mGraph, false);
    FilterArcs<SmartDigraph>        mySubGraph(mGraph, actArcs);
    vector< int >   componentTrack;
    int             compNr = -2;
    int             i = initTime;
    
    while ( compNr != 1) {
        for ( auto arc : mTimeToArcs[ times[i] ] ){
            actArcs[ arc] = true;
        }
        i++;
        compNr = countConnectedComponents( undirector( mySubGraph ) );
        cout << i << " : " << compNr << endl;
        // track components
    }

    // ------------------------------
    // --- Cal the degree
//    InDegMap< decltype(mySubGraph) >            inDegree(mySubGraph);
//    OutDegMap< decltype(mySubGraph) >           outDegree(mySubGraph);
//    AddMap<decltype(inDegree), decltype(outDegree) > mDegree(inDegree, outDegree);

    
//    
//    // --- Filename: add the nr of timesteps
//    target += "_";
//    stringstream ss;
//    ss << timeSteps;
//    target += ss.str();
//    target += "steps.txt";
//    
//    ofstream file( target.c_str() );
//    file << "label\tdegree\tactivity\n";
//    for (SmartDigraph::NodeIt n(mGraph); n!=INVALID; ++n) {
//        file << mGraph.id(n) << "\t" << mDegree[ n ] << "\t" << activity[ n ] << endl;
//    }
//    
//    file.close();
    
    //    digraphWriter(mGraph, target)
    //    .nodeMap("degree", mDegree)
    //    .nodeMap("activity", activity)
    //    .skipArcs()
    //    .run();
    
}
