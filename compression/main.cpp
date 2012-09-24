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
#include <kvs/KVSMLObjectPoint>
#include <kvs/PointExporter>
#include <kvs/UnstructuredVolumeExporter>
#include <kvs/TransferFunction>
#include <kvs/glut/TransferFunctionEditor>
#include <kvs/ExtractEdges>
#include <kvs/ExtractVertices>
#include <kvs/CommandLine>
#include <kvs/Timer>
#include "CubeToTetrahedraBspline.h"
#include "CubeToTetrahedraLinear.h"
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
    std::string outFilename;
    std::string outPoint;
    
    Argument( int argc, char** argv ) : CommandLine ( argc, argv )
    {
        add_help_option();
        addOption( "f", "filename", 1, true );
        addOption( "b", "block size", 1, false );
        addOption( "sp", "subpixel level", 1 , false );
        addOption( "ss", "sampling step", 1, false );
        addOption( "t", "transfer function", 1, false );
//        addOption( "rl", "repeat level", 1, false );
        addOption( "PBVR", "with renderer of PBVR", 0, false );
        addOption( "SPT", "with renderer of SPT", 0, false );
        addOption( "Edge", "extract the edge of the the volume", 0, false );
        addOption( "Bspline", "with evaluation mehod of bspline", 0, false );
        addOption( "write", "write the block divided volume to this folder", 0, false );
        addOption( "writezk", "write the zk file", 0, false );
        addOption( "outname", "input the output filename", 1, false );
        addOption( "outpoint", "input the output point name", 1, false );

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
        if( this->hasOption( "outname" )) outFilename = this->optionValue<std::string>( "outname" );
        if( this->hasOption( "outpoint" )) outPoint = this->optionValue<std::string>( "outpoint" );
        
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

void WriteKVSMLPoint( kvs::PointObject* object, std::string filename )
{
    kvs::KVSMLObjectPoint* kvsml = new kvs::PointExporter<kvs::KVSMLObjectPoint>( object );
    kvsml->setWritingDataType( kvs::KVSMLObjectPoint::ExternalBinary );
    kvsml->write( filename );
}

int main( int argc, char** argv )
{
    kvs::glut::Application app( argc, argv );
    kvs::glut::Screen screen( &app );
    Argument param( argc, argv );
    param.exec();
    
    kvs::TransferFunction transferfunc = param.tfunc;
    
    // load the original volume data
    kvs::StructuredVolumeObject* volume = new kvs::StructuredVolumeImporter( param.filename );
    transferfunc.setRange( volume->minValue(), volume->maxValue() );

    std::string volumeName = param.outFilename;
        
    size_t nx = volume->resolution().x();
    size_t ny = volume->resolution().y();
    size_t nz = volume->resolution().z();
    
    //Block Division
    if ( param.hasOption( "b" ) )
    {
        kvs::UnstructuredVolumeObject* tet;
        if( param.hasOption( "Bspline") )
        {
            std::cout << "With the Bspline evaluation" << std::endl;
            kvs::Timer time;
            time.start();
            tet = new kvs::CubeToTetrahedraBspline( volume, param.block_size );
            time.stop();
            std::cout << "min value of the compressed volume:" << tet->minValue() << std::endl;
            std::cout << "max value of the compressed volume:" << tet->maxValue() << std::endl;
            std::cout << "start processing the value" << std::endl;
            float* pvalues = (float*)tet->values().pointer();
            float min = volume->minValue();
            float max = volume->maxValue();
            for ( size_t i = 0; i < tet->nnodes(); i++ )
            {
                if( pvalues[i] > max ) pvalues[i] = max;
                if( pvalues[i] < min ) pvalues[i] = min;
            }
            
            std::cout << "Processing time: " << time.msec() << "msec" << std::endl;
        }
        else
        {
            std::cout << "With the Linear evaluation" << std::endl;
            kvs::Timer time;
            time.start();
            tet = new kvs::CubeToTetrahedraLinear( volume, param.block_size );
            time.stop();
            std::cout << "Processing time: " << time.msec() << "msec" << std::endl;
        }
        delete volume;
    
        //Write Data
        if( param.hasOption( "write" ) )
        {
            kvs::KVSMLObjectUnstructuredVolume* output_volume = new kvs::UnstructuredVolumeExporter<kvs::KVSMLObjectUnstructuredVolume>( tet );
            output_volume->setWritingDataType( kvs::KVSMLObjectUnstructuredVolume::ExternalBinary );
            char block_char[256];
            sprintf( block_char, "%ld", param.block_size );
            std::string num = std::string( block_char ); 
            std::string output_filename = volumeName + "BsplineBlock_" + num + ".kvsml";
            output_volume->write( output_filename.c_str() );
            std::cout << "finish writing" << std::endl;
        }
        //Write Values Data
        if( param.hasOption( "writezk" ) )
        {
            unsigned int length = tet->nnodes() + 4;
            float* buf = new float[length];
            buf[0] = (float)param.block_size;
            buf[1] = (float)nx;
            buf[2] = (float)ny;
            buf[3] = (float)nz;
            float* ori_values = (float*)tet->values().pointer();
            for( size_t i = 0; i < tet->nnodes(); i++ ) buf[i + 4] = ori_values[i];
            
            char block_char[256];
            sprintf( block_char, "%ld", param.block_size );
            std::string num = std::string( block_char ); 
            std::string outputName = volumeName + "Bspline_" + num + ".zk";
            FILE* outputFile = fopen( outputName.c_str(), "wb" );
            fwrite( buf, sizeof(float), length, outputFile );
            std::cout << "finish writting zk file" << std::endl;
        }
        if( param.hasOption( "PBVR" ) )
        {
            kvs::PointObject* object = new kvs::CellByCellMetropolisSampling(
                                                                             tet,
                                                                             param.sp,
                                                                             param.samplingstep,
                                                                             transferfunc,
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
            
            if( param.hasOption( "outpoint" ) )
                WriteKVSMLPoint( object, param.outPoint );
            std::cout << "Finish writting the point object" << std::endl;
            
        }
        if( param.hasOption( "Edge" ) )
        {
            kvs::LineObject* line = new kvs::ExtractEdges( tet );
            screen.registerObject( line );
            screen.setTitle( "ExtractEdges");
            
            std::cout << "ExtractEdges process has been done" << std::endl;
        }
    }
    else
    {
        kvs::PointObject* object = new kvs::CellByCellMetropolisSampling(
                                                                         volume,
                                                                         param.sp,
                                                                         param.samplingstep,
                                                                         transferfunc,
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
    }
    
    screen.background()->setColor( kvs::RGBColor( 255, 255, 255 ));
//    screen.camera()->scale( kvs::Vector3f( 0.5 ) );
    screen.setGeometry( 0, 0, 1024, 768 );
    screen.show();
    
    return( app.run() );
}
