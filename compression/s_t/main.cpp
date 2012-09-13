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
#include "CubeToTetrahedraBspline.h"
#include "LinearInterpolator.cpp"

using namespace std;
size_t nx = 385;
size_t ny = 349;
size_t nz = 200;

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

kvs::StructuredVolumeObject* RectToUniform( std::string filename_s, std::string filename_t )
{
    kvs::StructuredVolumeObject* object_s = new kvs::StructuredVolumeImporter( filename_s );
    size_t nx_ori = object_s->resolution().x();
    size_t ny_ori = object_s->resolution().y();
    size_t nz_ori = object_s->resolution().z();
    
    kvs::StructuredVolumeObject* object_t = new kvs::StructuredVolumeImporter( filename_t );
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
    
//    // read the depth from outside
//    ifstream infile( "depth.txt" );    
//    float* index = new float[78];
//    float* depth = new float[78];
//    float* dz = new float[78];
//    for ( size_t i = 0; i < 78; i ++ )
//    {
//        infile >> index[i];
//        infile >> depth[i];
//        infile >> dz[i];
//    }
//        
//    // interpolate the value linearly
//    // build the interpolator
//    LinearInterpolator interp[ nx * ny ];
//    for ( size_t k = 11; k < nz_ori; k ++ )
//        for ( size_t j = 0; j < ny_ori; j ++ )
//            for ( size_t i = 0; i < nx_ori; i ++ )
//            {
//                size_t index = i + j * nx + k * nx * ny;
//                interp[i + j * nx_ori].addDataPoint( depth[nz_ori - k], pvalues[index] );
//            }
//    // interpolate the value
//    kvs::AnyValueArray values;
//    float* buf = static_cast<float*>( values.allocate<float>( nx * ny * nz ) );
//    for ( size_t k = 0; k < nz; k ++ )
//        for ( size_t j = 0; j < ny; j ++ )
//            for ( size_t i = 0; i < nx; i ++ )
//            {
//                size_t index = i + j * nx + k * nx * ny;
//                float depth_new = 997.0 - k * 5.0;
//                buf[index] = interp[i + j * nx].interpolate( depth_new );
//            }
    
    kvs::Vector3ui resolution( nx, ny, nz );
    kvs::VolumeObjectBase::GridType grid_type = kvs::VolumeObjectBase::Uniform;
    kvs::StructuredVolumeObject* t_object = new kvs::StructuredVolumeObject();
    t_object->setGridType( grid_type);
    t_object->setVeclen( 1 );
    t_object->setResolution( object_t->resolution() );
    t_object->setValues( values );
    
    t_object->updateMinMaxCoords();
    t_object->updateMinMaxValues();
    
    return ( t_object );
}

int main( int argc, char** argv )
{
    kvs::glut::Application app( argc, argv );
    kvs::glut::Screen screen( &app );
    screen.show();

    Argument param( argc, argv );
    param.exec();
    
    //Load Volume Data
    kvs::StructuredVolumeObject* volume = RectToUniform( param.filename_s, param.filename_t );
    
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

    
    
//    std::string volumeName;
//        
//    size_t nx = volume->resolution().x();
//    size_t ny = volume->resolution().y();
//    size_t nz = volume->resolution().z();
//    
//    kvs::glut::Screen screen( &app );
//
//    //Block Division
//    if ( param.hasOption( "b" ) )
//    {
//        kvs::UnstructuredVolumeObject* tet;
//        if( param.hasOption( "Bspline") )
//        {
//            std::cout << "With the Bspline evaluation" << std::endl;
//	    kvs::Timer time;
//	    time.start();
//            tet = new kvs::CubeToTetrahedraBspline( volume, param.block_size );
//	    time.stop();
//	    std::cout << "Processing time: " << time.msec() << "msec" << std::endl;
//
//        }
//        else
//        {
//	    std::cout << "With the Linear evaluation" << std::endl;
//	    kvs::Timer time;
//	    time.start();
//            tet = new kvs::CubeToTetrahedraLinear( volume, param.block_size );
//	    time.stop();
//	    std::cout << "Processing time: " << time.msec() << "msec" << std::endl;
//        }
//        delete volume;
//    
//        //Write Data
//        if( param.hasOption( "write" ) )
//        {
//            kvs::KVSMLObjectUnstructuredVolume* output_volume = new kvs::UnstructuredVolumeExporter<kvs::KVSMLObjectUnstructuredVolume>( tet );
//            output_volume->setWritingDataType( kvs::KVSMLObjectUnstructuredVolume::ExternalBinary );
//            char block_char[256];
//            sprintf( block_char, "%ld", param.block_size );
//            std::string num = std::string( block_char ); 
//            std::string output_filename = volumeName + "BsplineBlock_" + num + ".kvsml";
//            output_volume->write( output_filename.c_str() );
//            std::cout << "finish writing" << std::endl;
//        }
//        //Write Values Data
//        if( param.hasOption( "writezk" ) )
//        {
//            unsigned int length = tet->nnodes() + 4;
//            float* buf = new float[length];
//            buf[0] = (float)param.block_size;
//            buf[1] = (float)nx;
//            buf[2] = (float)ny;
//            buf[3] = (float)nz;
//            float* ori_values = (float*)tet->values().pointer();
//            for( size_t i = 0; i < tet->nnodes(); i++ ) buf[i + 4] = ori_values[i];
//            
//            char block_char[256];
//            sprintf( block_char, "%ld", param.block_size );
//            std::string num = std::string( block_char ); 
//            std::string outputName = volumeName + "Bspline_" + num + ".zk";
//            FILE* outputFile = fopen( outputName.c_str(), "wb" );
//            fwrite( buf, sizeof(float), length, outputFile );
//        }
//        
//        if( param.hasOption( "SPT" ) )
//        {   
//            kvs::glew::StochasticTetrahedraRenderer* renderer_SPT = new kvs::glew::StochasticTetrahedraRenderer();
//            renderer_SPT->setRepetitionLevel( param.rl );
//            renderer_SPT->setTransferFunction( param.tfunc );
//            renderer_SPT->enableLODControl();
//            
//            screen.registerObject( tet, renderer_SPT );
//            screen.setTitle( "SPT Renderer");
//            
//            std::cout << "SPT process has been done" << std::endl;
//        }
//        if( param.hasOption( "PBVR" ) )
//        {
//            kvs::PointObject* object = new kvs::CellByCellMetropolisSampling(
//                                                                             tet,
//                                                                             param.sp,
//                                                                             param.samplingstep,
//                                                                             param.tfunc,
//                                                                             0.0f
//                                                                             );
//            kvs::glew::ParticleVolumeRenderer* renderer_PBVR = new kvs::glew::ParticleVolumeRenderer();
//            renderer_PBVR->setRepetitionLevel( param.rl );
//            renderer_PBVR->enableShading();
//            renderer_PBVR->setShader( kvs::Shader::Phong( 0.5, 0.5, 0.8, 15.0 ) );
//            //renderer_PBVR->disableShading();
//            //renderer_PBVR->disableZooming();
//            
//            screen.registerObject( object, renderer_PBVR );
//            screen.setTitle( "PBVR Renderer");
//            
//            std::cout << "PBVR process has been done" << std::endl;
//	    std::cout << *object << std::endl;
//        }
//        if( param.hasOption( "Edge" ) )
//        {
//            kvs::LineObject* line = new kvs::ExtractEdges( tet );
//            screen.registerObject( line );
//            screen.setTitle( "ExtractEdges");
//            
//            std::cout << "ExtractEdges process has been done" << std::endl;
//        }
//    }
//    else
//    {
//        kvs::PointObject* object = new kvs::CellByCellMetropolisSampling(
//                                                                         volume,
//                                                                         param.sp,
//                                                                         param.samplingstep,
//                                                                         param.tfunc,
//                                                                         0.0f
//                                                                         );
//        kvs::glew::ParticleVolumeRenderer* renderer_PBVR = new kvs::glew::ParticleVolumeRenderer();
//        renderer_PBVR->setRepetitionLevel( param.rl );
//        renderer_PBVR->enableShading();
//        renderer_PBVR->setShader( kvs::Shader::Phong( 0.5, 0.5, 0.8, 15.0 ) );
//        //renderer_PBVR->disableShading();
//        //renderer_PBVR->disableZooming();
//
//        screen.registerObject( object, renderer_PBVR );
//        screen.setTitle( "PBVR Renderer");
//        
//        std::cout << "PBVR process has been done" << std::endl;
//	std::cout << *object << std::endl;
//    }
//    
//    screen.background()->setColor( kvs::RGBColor( 255, 255, 255 ));
//    screen.camera()->scale( kvs::Vector3f( 0.5 ) );
//    screen.setGeometry( 0, 0, 1024, 768 );
//    screen.show();
//    
//    return( app.run() );
}
