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
#include "LinearInterpolator.cpp"

using namespace std;
size_t nx = 385;
size_t ny = 349;
size_t nz = 200;

class Argument : public kvs::CommandLine
{
public:
    
    std::string filename;
    size_t block_size;
    size_t sp;
    float samplingstep;
    kvs::TransferFunction tfunc;
    size_t rl;
    
    Argument( int argc, char** argv ) : CommandLine ( argc, argv )
    {
        add_help_option();
        add_option( "f", "filename", 1, true );
        add_option( "b", "block size", 1, false );
        add_option( "sp", "subpixel level", 1 , false );
        add_option( "ss", "sampling step", 1, false );
        add_option( "t", "transfer function", 1, false );
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
        tfunc.create( 256 );
        
        if( !this->parse() ) exit( EXIT_FAILURE );
        if( this->hasOption( "f" ) ) filename = this->optionValue<std::string>( "f" );
        if( this->hasOption( "b" ) ) block_size = this->optionValue<size_t>( "b" );
        if( this->hasOption( "sp" ) ) sp = this->optionValue<size_t>( "sp" );
        if( this->hasOption( "ss" ) ) samplingstep = this->optionValue<float>( "ss" );
        if( this->hasOption( "t" ) ) tfunc = kvs::TransferFunction( this->optionValue<std::string>( "t" ) );
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
    
//    // value processing
    float* pvalues = (float*)object->values().pointer();
//    unsigned int n = object->nnodes();
//    float min = 31;
//    float max = 35;
//    for ( size_t i = 0; i < n ; i++ )
//    {
//        if ( pvalues[i] < min ) pvalues[i] = min;
//        if ( pvalues[i] > max ) pvalues[i] = max;
//    }
    
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
    
    // interpolate the value linearly
    // build the interpolator
    LinearInterpolator interp[ nx * ny ];
    for ( size_t k = 11; k < nz_ori; k ++ )
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
                float depth_new = 997.0 - k * 5.0;
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
    kvs::StructuredVolumeObject* volume = RectToUniform( param.filename );
    WriteKVSML( volume, "Uniform0315s.kvsml" );
    
    kvs::glew::RayCastingRenderer* renderer
    = new kvs::glew::RayCastingRenderer();
    renderer->disableShading();
    
    // value range
    kvs::TransferFunction tfunc( 256 );
    tfunc.setRange( 31, 35 );
    
    TransferFunctionEditor editor( &screen );
    editor.setTransferFunction( tfunc );
    renderer->setTransferFunction( editor.transferFunction() );
    editor.setVolumeObject( volume );
    editor.show();
    
    screen.registerObject( volume, renderer );
    
    return( app.run() );
}
