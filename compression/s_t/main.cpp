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
#include "CubeToTetrahedraBspline.h"
#include "LinearInterpolator.cpp"

using namespace std;

class Argument : public kvs::CommandLine
{
public:
    
    std::string filename_s;
    std::string filename_t;
    size_t block_size;
    size_t sp;
    float samplingstep;
//    kvs::TransferFunction tfunc;
    size_t rl;
    
    Argument( int argc, char** argv ) : CommandLine ( argc, argv )
    {
        add_help_option();
        add_option( "s", "filename of s", 1, true );
        add_option( "t", "filename of t", 1, true );
        add_option( "b", "block size", 1, false );
        add_option( "sp", "subpixel level", 1 , false );
        add_option( "ss", "sampling step", 1, false );
//        add_option( "t", "transfer function", 1, false );
//        add_option( "rl", "repeat level", 1, false );
        add_option( "PBVR", "with renderer of PBVR", 0, false );
        add_option( "SPT", "with renderer of SPT", 0, false );
        add_option( "Edge", "extract the edge of the the volume", 0, false );
        add_option( "Bspline", "with evaluation mehod of bspline", 0, false );
        add_option( "write", "write the block divided volume to this folder", 0, false );
        add_option( "writezk", "write the zk file", 0, false );

    }
    
    void exec()
    {
        block_size = 1;
        sp = 3;
        samplingstep = 0.5;
//        tfunc.create( 256 );
        
        if( !this->parse() ) exit( EXIT_FAILURE );
        if( this->hasOption( "s" ) ) filename_s = this->optionValue<std::string>( "s" );
        if( this->hasOption( "t" ) ) filename_t = this->optionValue<std::string>( "t" );
        if( this->hasOption( "b" ) ) block_size = this->optionValue<size_t>( "b" );
        if( this->hasOption( "sp" ) ) sp = this->optionValue<size_t>( "sp" );
        if( this->hasOption( "ss" ) ) samplingstep = this->optionValue<float>( "ss" );
//        if( this->hasOption( "t" ) ) tfunc = kvs::TransferFunction( this->optionValue<std::string>( "t" ) );
//        if( this->hasOption( "rl" ) ) rl = this->optionValue<size_t>( "rl" );
        
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

kvs::StructuredVolumeObject* RectToUniform( std::string filename )
{
    kvs::StructuredVolumeObject* object = new kvs::StructuredVolumeImporter( filename );
    size_t nx_ori = object->resolution().x();
    size_t ny_ori = object->resolution().y();
    size_t nz_ori = object->resolution().z();
    size_t nx = nx_ori;
    size_t ny = ny_ori;
    size_t nz = 200;
    
    float* pvalues = (float*)object->values().pointer();
    // value processing
    unsigned int n = object->nnodes();
    float min = 31;
    float max = 35;
    for ( size_t i = 0; i < n ; i++ )
    {
        if ( pvalues[i] < min ) pvalues[i] = min;
        if ( pvalues[i] > max ) pvalues[i] = max;
    }
    
    // read the depth from outside
    ifstream infile( "depth.txt" );    
    float* index = new float[78];
    float* depth = new float[78];
    float* dz = new float[78];
    for ( size_t i = 0; i < 78; i ++ )
    {
        infile >> index[i];
        infile >> depth[i];
        infile >> dz[i];
    }
    
    std::cout << nz_ori << std::endl;
    
    // interpolate the value linearly
    // build the interpolator
    LinearInterpolator interp[ nx * ny ];
    for ( size_t k = 8; k < nz_ori; k ++ )
        for ( size_t j = 0; j < ny_ori; j ++ )
            for ( size_t i = 0; i < nx_ori; i ++ )
            {
                size_t index = i + j * nx + k * nx * ny;
                interp[i + j * nx_ori].addDataPoint( depth[nz_ori - k], pvalues[index] );
            }
    // interpolate the value
    kvs::AnyValueArray values;
    float* buf = static_cast<float*>( values.allocate<float>( nx * ny * nz ) );
    for ( size_t k = 0; k < nz; k ++ )
        for ( size_t j = 0; j < ny; j ++ )
            for ( size_t i = 0; i < nx; i ++ )
            {
                size_t index = i + j * nx + k * nx * ny;
                float depth_new = 1000.0 - k * 5.0;
                buf[index] = interp[i + j * nx].interpolate( depth_new );
            }
    
    kvs::Vector3ui resolution( nx, ny, nz );
    kvs::VolumeObjectBase::GridType grid_type = kvs::VolumeObjectBase::Uniform;
    kvs::StructuredVolumeObject* t_object = new kvs::StructuredVolumeObject();
    t_object->setGridType( grid_type);
    t_object->setVeclen( 1 );
    t_object->setResolution( resolution );
    t_object->setValues( values );
    
    t_object->updateMinMaxCoords();
    t_object->updateMinMaxValues();
    
    return ( t_object );
}


kvs::StructuredVolumeObject* ValueProcessing( kvs::StructuredVolumeObject* object_s, kvs::StructuredVolumeObject* object_t )
{
    size_t nx_ori = object_s->resolution().x();
    size_t ny_ori = object_s->resolution().y();
    size_t nz_ori = object_s->resolution().z();
    size_t nx = nx_ori;
    size_t ny = ny_ori;
    size_t nz = nz_ori;

    float* pvalues_s = (float*)object_s->values().pointer();
    float* pvalues_t = (float*)object_t->values().pointer();
    
    // value processing
    unsigned int n = object_t->nnodes();
    kvs::AnyValueArray values;
    float* pvalues = static_cast<float*>( values.allocate<float>( n ) );

    // color range 0 ~ 7
   float purple = 0.5;
   float blue = 1.5;
   float water = 2.5;
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
            pvalues[i] = 0;

    }
        
    kvs::Vector3ui resolution( nx, ny, nz );
    kvs::VolumeObjectBase::GridType grid_type = kvs::VolumeObjectBase::Uniform;
    kvs::StructuredVolumeObject* object = new kvs::StructuredVolumeObject();
    object->setGridType( grid_type);
    object->setVeclen( 1 );
    object->setResolution( resolution );
    object->setValues( values );
    
    object->updateMinMaxCoords();
    object->updateMinMaxValues();
    
    return ( object );
}

void WriteKVSML( kvs::StructuredVolumeObject* object, std::string filename )
{
    kvs::KVSMLObjectStructuredVolume* kvsml = new kvs::StructuredVolumeExporter<kvs::KVSMLObjectStructuredVolume>( object );
    kvsml->setWritingDataType( kvs::KVSMLObjectStructuredVolume::ExternalBinary );
    kvsml->write( filename );
}


int main( int argc, char** argv )
{
    kvs::glut::Application app( argc, argv );
    kvs::glut::Screen screen( &app );
    screen.show();

    Argument param( argc, argv );
    param.exec();
    
    //Load Volume Data
    kvs::StructuredVolumeObject* volume_s = RectToUniform( param.filename_s );
    kvs::StructuredVolumeObject* volume_t = RectToUniform( param.filename_t );
    kvs::StructuredVolumeObject* volume = ValueProcessing( volume_s, volume_t );
    WriteKVSML( volume, "Uniform0715st.kvsml" );
    
    kvs::glew::RayCastingRenderer* renderer
    = new kvs::glew::RayCastingRenderer();
    renderer->disableShading();
    float ka = 0.3;
    float kd = 0.5;
    float ks = 0.8;
    float n  = 100;
    renderer->setShader( kvs::Shader::Phong( ka, kd, ks, n ) );
    
    TransferFunctionEditor editor( &screen );
    editor.setVolumeObject( volume );
    editor.show();
    
    screen.registerObject( volume, renderer );
    
    return( app.run() );
}
