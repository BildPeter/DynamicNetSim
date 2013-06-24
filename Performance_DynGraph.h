//
//  Performance_DynGraph.h
//  DynamicNetSim
//
//  Created by Peter A. Kolski on 09.01.13.
//  Copyright (c) 2013 Peter A. Kolski. All rights reserved.
//

#ifndef __DynamicNetSim__Performance_DynGraph__
#define __DynamicNetSim__Performance_DynGraph__

#include <iostream>
#include <netevo.h>

using namespace netevo;

class SIRdynamic : public NodeDynamic {
public:
    double beta;
    double lambda;
    
    string  getName()   { return "SIRdynamic"; }
    int     getStates() { return 3; }
    void    setDefaultParams( Node n, System &sys ){
        sys.nodeData( n ).dynamicParams.push_back( beta );
        sys.nodeData( n ).dynamicParams.push_back( lambda );
    }
    
    void fn( Node n, System &sys, const State &x, State &dx, const double t ) {
        int stateID = sys.stateID( n );
        vector< double > &nodeParams = sys.nodeData( n ).dynamicParams;
        double N = x[ stateID ] + x[ stateID + 1 ] + x[ stateID + 2 ];
        
        //    dS = - S * I * beta
        //    dI =
        //    dR = lambda * I
        
        dx[ stateID     ] = ( (-1) * x[ stateID ] * x[ stateID + 1 ] * nodeParams[ 0 ]  ) / N;
        dx[ stateID + 1 ] = ( x[ stateID ] * x[ stateID + 1 ] * nodeParams[ 0 ]  ) / N - nodeParams[ 1 ] * x[ stateID + 1 ];
        dx[ stateID + 2 ] = nodeParams[ 1 ] * x[ stateID + 1 ];
    }
    
    // ----- Constructor -----
    SIRdynamic( double tmpBeta, double tmpLambda ){
        beta    = tmpBeta;
        lambda  = tmpLambda;
    }
};


class ActiveArcsInteractionMap    : public NodeDynamic {
public:
    
    const System::ArcMap<bool>        *mActiveArcs;
    
    string  getName()   { return "InteractionMap"; }
    int     getStates() { return 3; }
    void    setDefaultParams( Node n, System &sys ){
        sys.nodeData( n ).dynamicParams.push_back( 0.1 );  // Parameter um die Transport-Rate festzulegen
        //        sys.nodeData( n ).dynamicParams.push_back( 1 );  // Population size
    }
    
    void    fn( Node n, System &sys, const State &x, State &dx, double t ){
        
        double stateID = sys.stateID( n );
        
        double sumS = 0.0;
        double sumI = 0.0;
        double sumR = 0.0;
        
        // Checks, if the incoming arcs are turned on
        int     hasArcs = 0;
        for (System::InArcIt Arc( sys, n ); Arc != INVALID; ++Arc ){
            hasArcs += (*mActiveArcs)[ Arc ];
        }
        
        if ( hasArcs ){
            double sourcePopulation, nodePopulation;
            double unchangedRest, demandRatio;
            
            // ----- Get all the states of the sources
            for (System::InArcIt Arc( sys, n ); Arc != INVALID; ++Arc ){
                if ( (*mActiveArcs)[ Arc ] ) {                               // Hier wird entschieden, ob die Kante an ist
                    sumS += x[ sys.stateID( sys.source( Arc ) )     ];
                    sumI += x[ sys.stateID( sys.source( Arc ) ) + 1 ];
                    sumR += x[ sys.stateID( sys.source( Arc ) ) + 2 ];
                }
            }
            
            // ----- Population amount of the node
            // Conditions are
            // 1.) There are InArcs
            // 2.) The sum of values is not ZERO (not all arcs InActive)
            sourcePopulation = sumS + sumI + sumR;
            
            if ( sourcePopulation > 0 ) {
                nodePopulation =  x[ stateID ] + x[ stateID + 1 ] + x[ stateID + 2 ];
                
                // ----- Amount of 'demand' for the current node
                unchangedRest  = 1 - sys.nodeData( n ).dynamicParams[ 0 ];
                demandRatio =     sys.nodeData( n ).dynamicParams[ 0 ];
                
                // ----- Only the 'demand' amount will be scaled and changed by the interaction
                dx[ stateID     ] = x[ stateID     ] * unchangedRest + ( sumS * nodePopulation / sourcePopulation * demandRatio );
                dx[ stateID + 1 ] = x[ stateID + 1 ] * unchangedRest + ( sumI * nodePopulation / sourcePopulation * demandRatio );
                dx[ stateID + 2 ] = x[ stateID + 2 ] * unchangedRest + ( sumR * nodePopulation / sourcePopulation * demandRatio );
            }
            else{
                //                cout << " FEHLER!! SourcePopulation = 0 für Node " << sys.id( n ) << endl << endl;  // Fehler abfangen
                dx[ stateID     ] = x[ stateID     ] ;
                dx[ stateID + 1 ] = x[ stateID + 1 ] ;
                dx[ stateID + 2 ] = x[ stateID + 2 ] ;
            }
            
        }
        else{
            dx[ stateID     ] = x[ stateID     ] ;
            dx[ stateID + 1 ] = x[ stateID + 1 ] ;
            dx[ stateID + 2 ] = x[ stateID + 2 ] ;
        }
    }
    
    ActiveArcsInteractionMap( const  System::ArcMap<bool> &activeArcsMap ){
        mActiveArcs = &activeArcsMap;
    }
};


#endif /* defined(__DynamicNetSim__Performance_DynGraph__) */
