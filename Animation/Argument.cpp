//
//  Argument.cpp
//  
//
//  Created by Kun Zhao on 12/06/07.
//  Copyright (c) 2012年 Kyoto University. All rights reserved.
//

#include <iostream>
#include <kvs/CommandLine>

class Argument : public kvs::CommandLine
{
public:
    
    std::string filename;
    size_t timestep;
    size_t vindex;
    
    Argument( int argc, char** argv ) : CommandLine( argc, argv )
    {
        add_help_option();
        add_option( "f", "input file name", 1, false );
        add_option( "t", "input time step", 1, false );
        add_option( "v", "input vindex", 1, false );
    }
    
    void exec()
    {
        filename = "../../../../Data/RV2/Gogcm_fnk.ctl";
        timestep = 100;
        vindex = 3;
        
        if ( !this->parse() ) exit( EXIT_FAILURE );
        if ( this->hasOption( "f" ) ) filename = this->optionValue<std::string>( "f" );
        if ( this->hasOption( "t" ) ) timestep = this->optionValue<size_t>( "t" );
        if ( this->hasOption( "v" ) ) vindex = this->optionValue<size_t>( "v" );
    }
};
