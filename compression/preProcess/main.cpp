//
//  main.cpp
//  
//
//  Created by Kun Zhao on 12/09/21.
//  Copyright (c) 2012 Kyoto University. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <kvs/glut/Application>
#include <kvs/glut/Screen>
#include <kvs/StructuredVolumeObject>
#include <kvs/StructuredVolumeImporter>
#include <kvs/UnstructuredVolumeObject>
#include <kvs/LineObject>
#include <kvs/PointObject>
#include <kvs/CellByCellMetropolisSampling>
#include <kvs/glew/ParticleVolumeRenderer>
#include <kvs/glew/RayCastingRenderer>
#include <kvs/KVSMLObjectStructuredVolume>
#include <kvs/StructuredVolumeExporter>
#include <kvs/KVSMLObjectUnstructuredVolume>
#include <kvs/UnstructuredVolumeExporter>
#include <kvs/TransferFunction>
#include <kvs/glut/TransferFunctionEditor>
#include <kvs/ExtractEdges>
#include <kvs/ExtractVertices>
#include <kvs/CommandLine>
#include <kvs/Timer>
#include "peel.h"
#include "Interpolator.h"

class Argument : public kvs::CommandLine
{
public:
    
    std::string filename_s;
    std::string filename_t;
    std::string outname;
    
    Argument( int argc, char** argv ) : CommandLine ( argc, argv )
    {
        add_help_option();
        addOption( "s", "filename of s", 1, false );
        addOption( "t", "filename of t", 1, false );
        addOption( "outname", "filename of output", 1, false );
    }
    
    void exec()
    {
        if( !this->parse() ) exit( EXIT_FAILURE );
        if( this->hasOption( "s" ) ) filename_s = this->optionValue<std::string>( "s" );
        if( this->hasOption( "t" ) ) filename_t = this->optionValue<std::string>( "t" );
        if( this->hasOption( "outname" ) ) outname = this->optionValue<std::string>( "outname" );
    }
};

void WriteKVSML( kvs::StructuredVolumeObject* object, std::string filename )
{
    kvs::KVSMLObjectStructuredVolume* kvsml = new kvs::StructuredVolumeExporter<kvs::KVSMLObjectStructuredVolume>( object );
    kvsml->setWritingDataType( kvs::KVSMLObjectStructuredVolume::ExternalBinary );
    kvsml->write( filename );
}

class TransferFunctionEditor : public kvs::glut::TransferFunctionEditor
{
    
public:
    
    TransferFunctionEditor( kvs::ScreenBase* screen ) :
    kvs::glut::TransferFunctionEditor( screen )
    {
    }
    
    void apply( void )
    {
        kvs::RendererBase* r = screen()->rendererManager()->renderer();
        kvs::glew::RayCastingRenderer* renderer = static_cast<kvs::glew::RayCastingRenderer*>( r );
        renderer->setTransferFunction( transferFunction() );
        screen()->redraw();
    }
    
};


int main( int argc, char** argv )
{
    kvs::glut::Application app( argc, argv );
    kvs::glut::Screen screen( &app );
    screen.show();
    
    Argument param( argc, argv );
    param.exec();
    
    kvs::StructuredVolumeObject* object;
    //Load Volume Data
    if (param.hasOption( "s" ))
    {
        kvs::StructuredVolumeObject* volume = new kvs::StructuredVolumeImporter( param.filename_s );
        kvs::StructuredVolumeObject* peel_object = peel( volume, 3 );
        delete volume;
        object = Interpolator( peel_object, 31, 35 );
        delete peel_object;
    }
    if ( param.hasOption( "t" ))
    {
        kvs::StructuredVolumeObject* volume = new kvs::StructuredVolumeImporter( param.filename_t );
        kvs::StructuredVolumeObject* peel_object = peel( volume, 3 );
        delete volume;
        object = Interpolator( peel_object, 0, 25 );
        delete peel_object;
    }
    
    std::cout << *object << std::endl;
    
    WriteKVSML( object, param.outname );
    
    kvs::glew::RayCastingRenderer* renderer
    = new kvs::glew::RayCastingRenderer();
    renderer->disableShading();
    
    TransferFunctionEditor editor( &screen );
    
    editor.setVolumeObject( object );
    editor.show();
    
    screen.registerObject( object, renderer );
    
    return( app.run() ); 
}
