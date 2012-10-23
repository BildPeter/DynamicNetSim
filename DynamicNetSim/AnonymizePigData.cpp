//
//  AnonymizePigData.cpp
//  NE2_SIR
//
//  Created by BildPeter Visuals on 11.01.12.
//  Copyright (c) 2012 BildPeter Visuals. All rights reserved.
//

// C++
#include <iostream>
#include <unordered_set>
#include <utility>
//#include <algorithm>
#include <vector>

// NetEvo
#include <netevo.h>

// Boost
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

// LEMON
#include <lemon/lgf_reader.h>
#include <lemon/lgf_writer.h>
#include <lemon/maps.h>
#include <lemon/random.h>
#include <lemon/time_measure.h>


using namespace std;
using namespace lemon;
using namespace netevo;

int main( void ){
    
    /*
     --- Daten Anordnung ---
     From	To	Amount	Date
     560739	254569	7	2682
     913338	356536	1	3497
     */
    
    typedef boost::unordered_map< unsigned int, unsigned int >   myMap;
    myMap timeTranscode;
    string edgeListSource   = "/Users/sonneundasche/Desktop/D_sw_uvwd_cmpl.txt";
    string edgeListAnonym   = "/Users/sonneundasche/Desktop/Playground_UniqueSchaf/HugeDataSet.txt";
    vector< vector<unsigned int> >  transports;
    ifstream                        edgeList( edgeListSource );
    vector <unsigned int>           myVec(4);
    unsigned int                    i = 0;
    Timer                           t;
    
    // Einlesen der Werte in die Menge.
    while ( edgeList.good() ){
        
        edgeList >> myVec[0];
        edgeList >> myVec[1];
        edgeList >> myVec[2];           myVec[2] = myVec[ 2 ] * 2 ; // Zufälliger Amount Wert
        edgeList >> myVec[3];
        
        // Prüfen ob Array3 schon in der Map ist
        if ( timeTranscode.find( myVec[3] ) == timeTranscode.end() ){
            i++;
            timeTranscode.insert( myMap::value_type (myVec[3], i) );
        }
        
        myVec[ 3 ] = timeTranscode.find( myVec[3] )->second;
        
        transports.push_back( myVec ); 
    }
    cout << "Transport activities: " << transports.size() << endl;
    
    // ----------------------------------------------------------------------------------------
    // Schreiben der EdgeList mit neuen Werten
    // ----------------------------------------------------------------------------------------
    
    ofstream ResultFile( edgeListAnonym );
    
    for (auto iter = transports.begin(); iter != transports.end(); iter++) {
        ResultFile << (*iter)[0] << "\t" << (*iter)[1]<< "\t" << (*iter)[2]<< "\t" << (*iter)[3] << endl;
    }

    ResultFile.close();
    
    cout << "Dauer der Umwandlung: " << t.realTime() << endl;
    
}