//
//  Process_EraseLowActivity.cpp
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
    // ------------------------------
    string  sourceLGF, sourceTempEdge, target;
    int     threshold = 3;
    
    ArgParser       ap( argc, argv);
    ap.refOption("thres", "Initial time to start from", threshold, true);
    ap.refOption("g", "Graph file of LEMON GRAPH FORMAT", sourceLGF, true);
    ap.refOption("t", "Temporal active edges", sourceTempEdge, true);
    ap.refOption("n", "Target filename", target, true);
    ap.parse();
    
//    sourceLGF       = "/Users/sonneundasche/Documents/FLI/DATA/02 Daten - Schaf/Schaf_NEW.lgf";
//    sourceTempEdge  = "/Users/sonneundasche/Documents/FLI/DATA/02 Daten - Schaf/Schaf_NEW_time_tmpArcIDs.txt";
//    target          = "Test";
    
    // ------------------------------
    // --- Graph creation
    // ------------------------------
    ListDigraph                    mGraph;
    
    try {
        digraphReader(mGraph, sourceLGF)
        .run();
    } catch (lemon::Exception) {
        cerr << "Error reading";
        exit(0);
    }
        
    // ------------------------------
    // --- Temporal Edges
    // ------------------------------
    vector<int> times;
    boost::unordered_map< int,  vector<ListDigraph::Arc > >   mTimeToArcs;
    times = tempGR::readTemporalArcList(mGraph, mTimeToArcs, sourceTempEdge);
    //    int timeSteps = times.size();
    
    // ------------------------------
    // --- Count the activity
    // ------------------------------
    ListDigraph::NodeMap<int>      activity(mGraph);
    for ( auto t : times){
        for (auto arc :mTimeToArcs[t]){
            activity[ mGraph.source( arc ) ]++;
            activity[ mGraph.target( arc ) ]++;
        }
    }

    // ------------------------------
    // --- Mark nodes with low activity
    // ------------------------------
    vector< ListDigraph::Node >    eraseNodes;
    for (ListDigraph::NodeIt n(mGraph); n!=INVALID; ++n) {
        if ( activity[n] <= threshold) {
            eraseNodes.push_back(n);
        }
    }
    
    cout << "The complete graph has " << countConnectedComponents( undirector( mGraph ) )
    << " components and " << countNodes(mGraph) << " nodes" <<  endl;
    
    
    // ------------------------------
    // --- erase the nodes
    // ------------------------------
    for (auto n : eraseNodes){
        mGraph.erase( n );
    }

    cout << "The new graph has " << countConnectedComponents( undirector( mGraph ) )
    << " components and " << countNodes(mGraph) << " nodes" << endl;
   
    ListDigraph     newGraph;
    ListDigraph::ArcMap< ListDigraph::Arc >   arcRef( mGraph);

    digraphCopy( mGraph, newGraph)
    .arcRef(arcRef)  // arcMap[ oldArc ] = newArc
    .run();
    
    // ------------------------------
    // --- erase the invalid edges
    // ------------------------------
    vector< vector< ListDigraph::Arc > >    newTempArcs;
    for (auto i : times){
        vector<ListDigraph::Arc>  validArcs;
        for (auto arc : mTimeToArcs[i]){
            if ( mGraph.valid( arc ) )
                validArcs.push_back( arcRef[ arc ] );
        }
        newTempArcs.push_back( validArcs ); // assign new arc set
    }
    
    // ------------------------------
    // --- save the static graph
    // ------------------------------
    string  pathLFG = target;
    pathLFG += "_";
    stringstream ss;
    ss << threshold;
    pathLFG += ss.str();
    pathLFG += "thres.lfg";
    
    digraphWriter( mGraph, pathLFG)
    .run();
    
    // ------------------------------
    // --- save the temporal arcs
    // ------------------------------
    string  pathTempArc = target;
    pathTempArc += "_";
    stringstream ss2;
    ss2 << threshold;
    pathTempArc += ss2.str();
    pathTempArc += "thres_tempArcs.txt";

    tempGR::writeTempGraph<ListDigraph>( mGraph, newTempArcs, pathTempArc);
    
}
