//
//  Read_istreamTest.cpp
//  DynamicNetSim
//
//  Created by Peter A. Kolski (BildPeter Visuals) on 12.12.12.
//  Copyright (c) 2012 Peter A. Kolski.
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
                PROBLEM:
    Ich weiss nicht, wie ich das END OF LINE detektiere, um meine Daten einzulesen. Brauch die Zuweisung des ersten Elements (time)
    Mit GetLine würde es gehen, aber es liest in einen char -> d.h. ich muss die Größe vorher festlegen char [256]. Da könnte ich zu wenig Platz freigeben.
 
    --- GELÖST
    ABER MIT STRINGS :-S
 */


#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>

#include <unordered_map>

using namespace std;

int main(){

    unordered_map< int, vector<int> >   myMap;
    
    ifstream    mFile( "/Users/sonneundasche/Programmierung/Extern/DynamicNetSim/DynamicNetSim/Test_forPairReading.txt" );
    
    int in;
    
//    while (mFile ) {
////        cout << mFile;
//        mFile >> in;
//        cout <<  in << " - ";
//    }
    
/*
    ifstream input( "/Users/sonneundasche/Programmierung/Extern/DynamicNetSim/DynamicNetSim/Test_forPairReading.txt" );
    char const row_delim = '\n';
    char const field_delim = '\t';
    for (string row; getline(input, row, row_delim); ) {
        istringstream ss(row);

        string pairs;
        // -- read first line
        getline(ss, pairs, field_delim);
        cout << pairs << "\t";
        
        for (; getline(ss, pairs, field_delim); ) {
            
            string  value;
            istringstream ss(pairs);
            getline(ss, value, ' ');
            cout << value << "+";
            getline(ss, value, ' ');
            cout << value << "\t";

        }
        cout << endl;
    }
 */
    
    ifstream input( "/Users/sonneundasche/Programmierung/Extern/DynamicNetSim/DynamicNetSim/Test_forTABReading" );
    
    for (string line; getline(input, line, '\n'); ) {
        istringstream lineStream(line);
        int     timeInt;
        string tmpData;
        // -- read first number
        lineStream >> timeInt;
        cout << timeInt;
        for (; getline(lineStream, tmpData, '\t'); ) {
            lineStream >> timeInt;
            cout << "\t" << timeInt;
        }
        cout << endl;
    }
    
    
}