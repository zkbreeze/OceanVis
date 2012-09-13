#include <kvs/glut/Application>
#include <kvs/glut/Screen>
#include <kvs/StructuredVolumeObject>
#include <kvs/StructuredVolumeImporter>
#include <kvs/UnstructuredVolumeObject>
#include <kvs/glew/RayCastingRenderer>
#include <kvs/CommandLine>
#include "StructuredVolumeImporter.h"

class Argument : public kvs::CommandLine
{
public:
    
    std::string filename;
    size_t block_size;
    kvs::TransferFunction tfunc;
    size_t rl;
    bool h;
    bool s;
    
    Argument( int argc, char** argv ) : CommandLine ( argc, argv )
    {
        add_help_option();
        add_option( "f", "input filename", 1,  false );
        add_option( "b", "block size", 1, false );
        add_option( "t", "transfer function", 1, false );
        add_option( "rl", "repeat level", 1, false );
        add_option( "s", "stochastic renderer", 1, false );
        add_value( "hydrogen", false );
    }
    
    void exec()
    {
        block_size = 1;
        tfunc.create( 256 );
        rl = 50;
        s = false;
        h = false;
        
        if( !this->parse() ) exit( EXIT_FAILURE );
        if( this->hasOption( "f" ) ) filename = this->optionValue<std::string>( "f" );
        if( this->hasOption( "b" ) ) block_size = this->optionValue<size_t>( "b" );
        if( this->hasOption( "t" ) ) tfunc = kvs::TransferFunction( this->optionValue<std::string>( "t" ) );
        if( this->hasOption( "rl" ) ) rl = this->optionValue<size_t>( "rl" );
        if( this->hasOption( "s" ) ) s = this->optionValue<bool>( "s" );
        if( this->hasValues() ) h = true;

    }
};

int main( int argc, char** argv )
{
    kvs::glut::Application app( argc, argv );
    Argument param( argc, argv );
    param.exec();

    const size_t vindex = 3;
    const size_t tindex = 100;
    const bool zfilp = true;
    const kvs::StructuredVolumeObject::GridType grid_type = kvs::StructuredVolumeObject::Uniform;
    
    kvs::StructuredVolumeObject* volume = new util::StructuredVolumeImporter( param.filename, vindex, tindex, zfilp, grid_type );

    std::cout << "load succeed" << std::endl;
     
    kvs::glew::RayCastingRenderer* renderer = new kvs::glew::RayCastingRenderer();

    kvs::glut::Screen screen( &app );
    
    screen.registerObject( volume, renderer );
    screen.setTitle( "RayCasting Renderer");
    screen.setGeometry( 0, 0, 800, 600 );
    screen.show();

    return( app.run() );
}
