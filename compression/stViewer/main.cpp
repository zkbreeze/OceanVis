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

using namespace std;

class Argument : public kvs::CommandLine
{
public:
    
    std::string filename_s;
    std::string filename_t;
    kvs::TransferFunction tfunc;
    
    Argument( int argc, char** argv ) : CommandLine ( argc, argv )
    {
        add_help_option();
        addOption( "s", "filename of s", 1, true );
        addOption( "t", "filename of t", 1, true );
        addOption( "tfunc", "tfunc", 1, false );
    }
    
    void exec()
    {        
        tfunc.create( 256 );
        if( !this->parse() ) exit( EXIT_FAILURE );
        if( this->hasOption( "s" ) ) filename_s = this->optionValue<std::string>( "s" );
        if( this->hasOption( "t" ) ) filename_t = this->optionValue<std::string>( "t" );
        if( this->hasOption( "tfunc" ) ) tfunc = kvs::TransferFunction( this->optionValue<std::string>( "tfunc" ) );
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

kvs::StructuredVolumeObject* ValueProcessing( kvs::StructuredVolumeObject* object_s, kvs::StructuredVolumeObject* object_t )
{
    size_t nx_ori = object_s->resolution().x();
    size_t ny_ori = object_s->resolution().y();
    size_t nz_ori = object_s->resolution().z();

    float* pvalues_s = (float*)object_s->values().pointer();
    float* pvalues_t = (float*)object_t->values().pointer();
    
    // value processing
    unsigned int n = object_t->nnodes();
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
            pvalues[i] = 0;

    }
        
    kvs::Vector3ui resolution( nx_ori, ny_ori, nz_ori );
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
    kvs::StructuredVolumeObject* volume_s = new kvs::StructuredVolumeImporter( param.filename_s );
    kvs::StructuredVolumeObject* volume_t = new kvs::StructuredVolumeImporter( param.filename_t );
    kvs::StructuredVolumeObject* volume = ValueProcessing( volume_s, volume_t );
    WriteKVSML( volume, "Uniform0715st.kvsml");
    
    kvs::glew::RayCastingRenderer* renderer
    = new kvs::glew::RayCastingRenderer();
    renderer->disableShading();
    
    TransferFunctionEditor editor( &screen );
    editor.setTransferFunction( param.tfunc );
    renderer->setTransferFunction( editor.transferFunction() );
    editor.setVolumeObject( volume );
    editor.show();
    
    screen.registerObject( volume, renderer );
    
    return( app.run() );
}
