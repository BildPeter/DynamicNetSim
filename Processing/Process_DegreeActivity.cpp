//
//  Process_DegreeActivity.cpp
//  DynamicNetSim
//
//  Created by Peter A. Kolski (BildPeter Visuals) on 13.02.13.
//  Copyright (c) 2013 Peter A. Kolski.
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  For the GNU General Public License see <http://www.gnu.org/licenses/>.
//
//
//

/*
    AIM:
    Process Data to output a node list with the degree and nr of activities
    Activity is defined for a node as EACH EDGE which interacted with it.
 
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
    string  sourceLGF, sourceTempEdge, target;
//    ArgParser       ap( argc, argv);
//    ap.refOption("g", "Graph file of LEMON GRAPH FORMAT", sourceLGF, true);
//    ap.refOption("t", "Temporal active edges", sourceTempEdge, true);
//    ap.refOption("n", "Target filename", target, true);
//    ap.parse();
    
    sourceLGF       = "/Users/sonneundasche/Documents/FLI/DATA/02 Daten - Schaf/Schaf_NEW_.lgf";
    sourceTempEdge  = "/Users/sonneundasche/Documents/FLI/DATA/02 Daten - Schaf/Schaf_NEW__time_tmpArcIDs.txt";
    target          = "Test";

    // ------------------------------
    // --- Graph creation
    SmartDigraph                    mGraph;
    try {
        digraphReader(mGraph, sourceLGF)
        .run();
    } catch (lemon::Exception) {
        exit(0);
    }
    
    SmartDigraph::NodeMap<int>      activity(mGraph);
    InDegMap<SmartDigraph>          inDegree(mGraph);
    OutDegMap<SmartDigraph>         outDegree(mGraph);
    AddMap<InDegMap<SmartDigraph>, OutDegMap<SmartDigraph> > mDegree(inDegree, outDegree);
    
    
    // ------------------------------
    // --- Temporal Edges
    vector<int> times;
    boost::unordered_map< int,  vector<SmartDigraph::Arc > >   mTimeToArcs;
    times = tempGR::readTemporalArcList(mGraph, mTimeToArcs, sourceTempEdge);
    int timeSteps = times.size();
    
    // ------------------------------
    // --- Count the activity
    for ( auto t : times){
        for (auto arc :mTimeToArcs[t]){
            activity[ mGraph.source( arc ) ]++;
            activity[ mGraph.target( arc ) ]++;
        }
    }
    
    // --- Filename: add the nr of timesteps
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