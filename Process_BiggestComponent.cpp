//
//  Process_BiggestComponent.cpp
//  DynamicNetSim
//
//  Created by Peter on 14.02.13.
//  Copyright (c) 2013 Peter. All rights reserved.
//


/*
 AIM:
 Determine the components of the graph systems
 Find the biggest one
 Clean the graph and the temporal edges
 Export the new graph with temp arcs
 
 INPUT:
 2 files - LGF and tempEdges
 
 OUTPUT:
 Final graph (LGF) with map of old node-ids
 Temporal arcs list
 
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
    
//    ArgParser       ap( argc, argv);
//    ap.refOption("g", "Graph file of LEMON GRAPH FORMAT", sourceLGF, true);
//    ap.refOption("t", "Temporal active edges", sourceTempEdge, true);
//    ap.refOption("n", "Target filename", target, true);
//    ap.parse();

//    sourceLGF       = "/Users/sonneundasche/Documents/FLI/DATA/02Sheep/Schaf_NEW.lgf";
//    sourceTempEdge  = "/Users/sonneundasche/Documents/FLI/DATA/02Sheep/Schaf_NEW_time_tmpArcIDs.txt";
        sourceLGF       = "/Users/sonneundasche/Documents/FLI/DATA/03Pork/porkNEW_.lgf";
        sourceTempEdge  = "/Users/sonneundasche/Documents/FLI/DATA/03Pork/porkNEW__time_tmpArcIDs.txt";
        target          = "Test";
    
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
    
    // ------------------------------
    // --- Determine the components
    // ------------------------------
    ListDigraph::NodeMap<int>      mCompIds(mGraph);
    mapFill(mGraph, mCompIds, -1);
    int nrComponents        = connectedComponents( undirector( mGraph ), mCompIds );
    
    // Determine the ID with the biggest component
    vector<int>     mIDcount(nrComponents,0);
    for (ListDigraph::NodeIt n(mGraph); n!=INVALID; ++n) {
        mIDcount[ mCompIds[ n ] ]++;
    }
    auto a = max_element( mIDcount.begin(), mIDcount.end() );
    int maxCompID = distance( mIDcount.begin(), a );

    cout << "Nodes: " << countNodes( mGraph ) << " components: " << countConnectedComponents( undirector( mGraph ) ) << endl;
    cout << "Max Comp ID: " << maxCompID << " size: " << *a << endl;
//    int i = 0;
//    for ( auto x : mIDcount ){
//    cout << "Comp ID: " << i++ << " size: " << x << endl;
//    }
    
    // ------------------------------
    // --- Mark nodes of the biggest component
    // ------------------------------
    vector< ListDigraph::Node >    eraseNodes;
    for (ListDigraph::NodeIt n(mGraph); n!=INVALID; ++n) {
        if ( mCompIds[ n ] != maxCompID) {
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
    ListDigraph::ArcMap< ListDigraph::Arc >     arcRef( mGraph);
    ListDigraph::NodeMap< ListDigraph::Node>    nodeRef( mGraph );
    
    digraphCopy( mGraph, newGraph)
    .arcRef(arcRef)  // arcMap[ oldArc ] = newArc
    .nodeRef(nodeRef)
    .run();
    
    // ------------------------------
    // --- erase the invalid temporal edges
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
    pathLFG += "_LargesComponent.lfg";
    
    digraphWriter( mGraph, pathLFG)
    .run();
    
    // ------------------------------
    // --- save the temporal arcs
    // ------------------------------
    string  pathTempArc = target;
    pathTempArc += "_LargesComponent_tempArcs.txt";
    
    tempGR::writeTempGraph<ListDigraph>( mGraph, newTempArcs, pathTempArc);
    
}
