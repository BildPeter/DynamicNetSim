//
//  Process_DegreeActivity.cpp
//  DynamicNetSim
//
//  Created by Peter A. Kolski on 13.02.13.
//  Copyright (c) 2013 Peter. All rights reserved.
//

/*
    AIM:
    Process Data to output a node list with the degree and nr of activities
    
    Activity is defined for a node as EACH EDGE which interacted with it.
    Scaling by division through the total nr. of current interactions (arcs).
 
    INPUT:      2 files - LGF and tempEdges
    OUTPUT:     NodeList with degree and activity (TimeSpan in Filename).txt
 
    TODO:
        - output filename with timespan (steps)
        - LMap to cout activity
 */

#include <lemon/smart_graph.h>
#include <lemon/arg_parser.h>
#include <lemon/maps.h>
#include <lemon/lgf_reader.h>
#include <lemon/lgf_writer.h>

#include <peter/temporal_graph_handler.h>
#include <boost/unordered_map.hpp>

#include <fstream>

using namespace lemon;
using namespace std;

int main(int argc, char** argv){

    // ------------------------------
    // --- INPUT - OUTPUT
    // ------------------------------
    string  sourceLGF, sourceTempEdge, target;
    ArgParser       ap( argc, argv);
    ap.refOption("g", "Graph file of LEMON GRAPH FORMAT", sourceLGF, true);
    ap.refOption("t", "Temporal active edges", sourceTempEdge, true);
    ap.refOption("n", "Target filename", target, true);
    ap.parse();
    
//    sourceLGF       = "/Users/sonneundasche/Documents/FLI/DATA/02 Daten - Schaf/Schaf_NEW_.lgf";
//    sourceTempEdge  = "/Users/sonneundasche/Documents/FLI/DATA/02 Daten - Schaf/Schaf_NEW__time_tmpArcIDs.txt";
//    target          = "Test";

    // ------------------------------
    // --- Graph creation
    // ------------------------------
    SmartDigraph                    mGraph;
    try {
        digraphReader(mGraph, sourceLGF)
        .run();
    } catch (lemon::Exception) {
        exit(0);
    }
    
    SmartDigraph::NodeMap< double >      activity(mGraph);
    InDegMap<SmartDigraph>          inDegree(mGraph);
    OutDegMap<SmartDigraph>         outDegree(mGraph);
    AddMap<InDegMap<SmartDigraph>, OutDegMap<SmartDigraph> > mDegree(inDegree, outDegree);
    
    
    // ------------------------------
    // --- Temporal Edges
    // ------------------------------
    vector<int> times;
    boost::unordered_map< int,  vector<SmartDigraph::Arc > >   mTimeToArcs;
    times = tempGR::readTemporalArcList(mGraph, mTimeToArcs, sourceTempEdge);
    int timeSteps = times.size();
    
    // ------------------------------
    // --- Count the activity
    // ------------------------------
    for ( auto t : times){
        for (auto arc :mTimeToArcs[t]){
            activity[ mGraph.source( arc ) ]++;
            activity[ mGraph.target( arc ) ]++;
        }
    }
  
    // ------------------------------
    // --- Scaling
    // ------------------------------
    // -- count divisor
    double  mDiv = 0;
    for ( auto t : times){
        mDiv += mTimeToArcs[t].size();
    }
    // scale
    for (SmartDigraph::NodeIt n(mGraph); n!=INVALID; ++n) {
        activity[ n ] /= mDiv;
    }
    
    // ------------------------------
    // --- Filename: add the nr of timesteps
    // ------------------------------
    target += "_";
    stringstream ss;
    ss << timeSteps;
    target += ss.str();
    target += "steps.txt";
    
    ofstream file( target.c_str() );
    file << "label\tdegree\tactivity\n";
    for (SmartDigraph::NodeIt n(mGraph); n!=INVALID; ++n) {
        file << mGraph.id(n) << "\t" << mDegree[ n ] << "\t" << activity[ n ] << endl;
    }
    
    file.close();
    
//    digraphWriter(mGraph, target)
//    .nodeMap("degree", mDegree)
//    .nodeMap("activity", activity)
//    .skipArcs()
//    .run();
    
}