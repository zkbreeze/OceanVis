//
//  main.cpp
//  
//
//  Created by Kun Zhao on 12/09/11.
//  Copyright (c) 2012 Kyoto University. All rights reserved.
//

#include <iostream>
#include <fstream>

using namespace std;

int main( int argc, char** argv )
{
    float i;
    char *inname = argv[1];
    ifstream infile(inname);
    
    if (!infile) {
        cout << "There was a problem opening file "
        << inname
        << " for reading."
        << endl;
        return 0;
    }
    cout << "Opened " << inname << " for reading." << endl;
    
    float* index = new float[78];
    float* depth = new float[78];
    float* dz = new float[78];
    
    for ( size_t i = 0; i < 78; i ++ )
    {
        infile >> index[i];
        infile >> depth[i]; std::cout << depth[i] << std::endl;
        infile >> dz[i];
    }
//    while (infile >> i) {
//        cout << "Value from file is " << i << endl;
//    }
    return 0;
}