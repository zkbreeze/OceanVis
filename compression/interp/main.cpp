//
//  main.cpp
//  
//
//  Created by Kun Zhao on 12/09/11.
//  Copyright (c) 2012 Kyoto University. All rights reserved.
//

#include <iostream>
#include <map>
#include <vector>

class LinearInterpolator {
public:
    LinearInterpolator() {}
    
    void addDataPoint(float x, float &d) {
        // just add it to the map
        data[x] = d;
    }
    
    float interpolate(float x ) {
        // loop through all the keys in the map
        // to find one that is greater than our intended value
        std::map< float, float >::iterator it = data.begin();
        bool found = false;
        while(it != data.end() && !found) {
            if(it->first >= x) {
                found = true;
                break;
            }
            
            // advance the iterator
            it++;
        }
        
        // check to see if we're outside the data range
        if(it == data.begin()) {
            return data.begin()->second;
        }
        else if(it == data.end()) {
            // move the point back one, as end() points past the list
            it--;
            return it->second;
        }
        // check to see if we landed on a given point
        else if(it->first == x) {
            return it->second;
        }
        
        // nope, we're in the range somewhere
        // collect some values
        float xb = it->first;
        float yb = it->second;
        it--;
        float xa = it->first;
        float ya = it->second;
        
        // and calculate the result!
        // formula from Wikipedia
        return (ya + (yb - ya) * (x - xa) / (xb - xa));
    }
    
private:
    std::map<float, float> data;
};

int main() {
	LinearInterpolator interp;
    
    float* values = new float[10];
    for ( size_t i = 0 ; i < 10 ; i++ )
        values[i] = i * 5;

    
	// add data points to the interpolator
	interp.addDataPoint(0, values[0]);
	interp.addDataPoint(0.04, values[1]);
	interp.addDataPoint(0.05, values[2]);
	interp.addDataPoint(0.06, values[3]);
	interp.addDataPoint(0.07, values[4]);
	interp.addDataPoint(0.1, values[5]);
    
	// now loop through a bunch of points and interpolate at each of them
	std::cout << "x\trho\tG" << std::endl;
	for(float d = -0.01; d <= 0.11; d += 0.01) {
		std::cout << d << "\t" << interp.interpolate(d) << std::endl;
	}
}