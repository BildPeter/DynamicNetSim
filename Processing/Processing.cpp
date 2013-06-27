//
//  Processing.cpp
//  DynamicNetSim
//
//  Created by Peter on 27.06.13.
//  Copyright (c) 2013 Peter A. Kolski. All rights reserved.
//

#include "Processing.h"


int main(){
    Timer T(true);

    ElementWise_TimeSeries  mProcessor;
    
    
    
    mProcessor.readGraph("/Users/sonneundasche/Dropbox/FLI/DATA/18TradeOffTemp/data/largeNet_10000Nd_10000R_0.1T_20A.lgf",
                         "/Users/sonneundasche/Dropbox/FLI/DATA/18TradeOffTemp/data/largeNet_10000Nd_10000R_0.1T_20A.dynEdges", false );
    cout << "read time: " << T.realTime() <<"\n";
    T.restart();
    
    mProcessor.process();
    cout << "pressing time: " << T.realTime() << "\n";
    
    mProcessor.writeData("/Users/sonneundasche/Dropbox/FLI/DATA/18TradeOffTemp/data/TempTradeOff_TimeSeries_Elem");
    cout << "writing time: " << T.realTime() << "\n";
    
}