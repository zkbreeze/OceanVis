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
#include <kvs/UnstructuredVolumeObject>
#include <kvs/LineObject>
#include <kvs/PointObject>
#include <kvs/CellByCellMetropolisSampling>
#include <kvs/glew/ParticleVolumeRenderer>
#include <kvs/glew/RayCastingRenderer>
#include <kvs/KVSMLObjectUnstructuredVolume>
#include <kvs/UnstructuredVolumeExporter>
#include <kvs/TransferFunction>
#include <kvs/glut/TransferFunctionEditor>
#include <kvs/ExtractEdges>
#include <kvs/ExtractVertices>
#include <kvs/CommandLine>
#include <kvs/Timer>
#include <kvs/KVSMLObjectStructuredVolume>
#include <kvs/StructuredVolumeExporter>
#include "BlockLoader.h"

using namespace std;

class Argument : public kvs::CommandLine
{
public:
    
    std::string filename_s;
    std::string filename_t;
    size_t sp;
    kvs::TransferFunction tfunc;
    size_t rl;
    
    Argument( int argc, char** argv ) : CommandLine ( argc, argv )
    {
        add_help_option();
        addOption( "s", "filename of s", 1, true );
        addOption( "t", "filename of t", 1, true );
        addOption( "sp", "subpixel", 1, false  );
        addOption( "tfunc", "transfer function", 1, false );        
    }
    
    void exec()
    {        
        sp = 3;
        tfunc.create( 256 );
        if( !this->parse() ) exit( EXIT_FAILURE );
        if( this->hasOption( "s" ) ) filename_s = this->optionValue<std::string>( "s" );
        if( this->hasOption( "t" ) ) filename_t = this->optionValue<std::string>( "t" );
        if( this->hasOption( "sp" ) ) sp = this->optionValue<size_t>( "sp" );
        if( this->hasOption( "tfunc" ) ) tfunc = kvs::TransferFunction( this->optionValue<std::string>( "tfunc" ) );
        rl = sp * sp;
    }
};

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

kvs::UnstructuredVolumeObject* ValueProcessing( kvs::UnstructuredVolumeObject* object_s, kvs::UnstructuredVolumeObject* object_t )
{    
    float* pvalues_s = (float*)object_s->values().pointer();
    float* pvalues_t = (float*)object_t->values().pointer();
    
    // value processing
    unsigned int n = object_s->nnodes();
    kvs::AnyValueArray values;
    float* pvalues = static_cast<float*>( values.allocate<float>( n ) );
    
    // color range 0 ~ 7
    float purple = 1;
    float blue = 2;
    float water = 3;
    float green = 4;
    float orange = 5;
    float red = 7;
    // float purple = 0;
    // float blue = 10;
    // float water = 0;
    // float green = 40;
    // float orange = 0;
    // float red = 70;
    
    
    for ( size_t i = 0; i < n ; i++ )
    {
        
        if ( 33.67 <= pvalues_s[i] && pvalues_s[i] < 35 && 3 <= pvalues_t[i] && pvalues_t[i] < 6 ) 
            pvalues[i] = purple;
        else if ( 32 <= pvalues_s[i] && pvalues_s[i] < 33.33 && 0 <= pvalues_t[i] && pvalues_t[i] < 2 )
            pvalues[i] = blue;
        else if ( 31 <= pvalues_s[i] && pvalues_s[i] < 32 && 0 <= pvalues_t[i] && pvalues_t[i] < 2 )
            pvalues[i] = water;
        else if ( 32 <= pvalues_s[i] && pvalues_s[i] < 33.33 && 2 <= pvalues_t[i] && pvalues_t[i] < 25 )
            pvalues[i] = green;
        else if ( 33.33 <= pvalues_s[i] && pvalues_s[i] < 35 && 0 <= pvalues_t[i] && pvalues_t[i] < 3
                 || 33.33 <= pvalues_s[i] && pvalues_s[i] < 33.67 && 0 <= pvalues_t[i] && pvalues_t[i] < 25 )
            pvalues[i] = orange;
        else if ( 33.67 <= pvalues_s[i] && pvalues_s[i] < 35 && 6 <= pvalues_t[i] && pvalues_t[i] < 25 )
            pvalues[i] = red;
        else
            pvalues[i] = 1;
        
    }
    
//    kvs::UnstructuredVolumeObject* object = new kvs::UnstructuredVolumeObject();
//    object->setVeclen( 1 );
//    object->setNNodes( object_s->nnodes() );
//    object->setCellType( kvs::UnstructuredVolumeObject::Tetrahedra );
//    object->setCoords( object_s->coords() );
//    object->setConnections( object_s->connections() );    
//    object->setValues( values );    
//    object->updateMinMaxCoords();
//    object->updateMinMaxValues();
    object_s->setValues( values );
    object_s->updateMinMaxValues();
    
    return ( object_s );
}

int main( int argc, char** argv )
{
    kvs::glut::Application app( argc, argv );
    kvs::glut::Screen screen( &app );
    
    Argument param( argc, argv );
    param.exec();
    
    //Load Volume Data
    kvs::UnstructuredVolumeObject* volume_s = new kvs::BlockLoader( param.filename_s );
    kvs::UnstructuredVolumeObject* volume_t = new kvs::BlockLoader( param.filename_t );
    kvs::UnstructuredVolumeObject* volume = ValueProcessing( volume_s, volume_t );
    
    std::cout << "max value of new volume:" << volume->maxValue() << std::endl;
    std::cout << "min value of new volume:" << volume->minValue() << std::endl;
    kvs::TransferFunction t = param.tfunc;
    t.setRange(0, 7);
    
    kvs::PointObject* object = new kvs::CellByCellMetropolisSampling(
                                                                     volume,
                                                                     param.sp,
                                                                     0.5,
                                                                     t,
                                                                     0.0f
                                                                     );
    kvs::glew::ParticleVolumeRenderer* renderer_PBVR = new kvs::glew::ParticleVolumeRenderer();
    renderer_PBVR->setRepetitionLevel( param.rl );
    renderer_PBVR->enableShading();
    renderer_PBVR->setShader( kvs::Shader::Phong( 0.5, 0.5, 0.8, 15.0 ) );
    //renderer_PBVR->disableShading();
    //renderer_PBVR->disableZooming();
    
    screen.registerObject( object, renderer_PBVR );
    screen.setTitle( "PBVR Renderer");
    
    std::cout << "PBVR process has been done" << std::endl;
    std::cout << *object << std::endl;
    
    screen.background()->setColor( kvs::RGBColor( 255, 255, 255 ));
    //    screen.camera()->scale( kvs::Vector3f( 0.5 ) );
    screen.setGeometry( 0, 0, 1024, 768 );
    screen.show();
    
    return( app.run() );
}
