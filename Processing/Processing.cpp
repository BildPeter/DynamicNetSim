//
//  Processing.cpp
//  DynamicNetSim
//
//  Created by Peter on 27.06.13.
//  Copyright (c) 2013 Peter A. Kolski. All rights reserved.
//

#include "Processing.h"


int main(){


    ElementWise_TimeSeries  mProcessor;
    
    mProcessor.readGraph("/Volumes/Augenweide/HIT_Jul2006_CLEAN.lgf", "/Volumes/Augenweide/HIT_Jul2006_CLEAN_time_tmpArcIDs_amountOnArc.txt", true );
    
    mProcessor.process();
    
    mProcessor.writeData("/Volumes/Augenweide/Export_Test");
    
}