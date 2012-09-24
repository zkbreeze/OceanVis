//
//  main.cpp
//  
//
//  Created by コン チョウ on 12/01/31.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#include <kvs/glut/Application>
#include <kvs/glut/Screen>
#include <kvs/glut/Slider>
#include <kvs/glut/Label>
#include <kvs/glut/LegendBar>
#include <kvs/glew/PartcileVolumeRenderer>
#include <kvs/PointObject>
#include <kvs/PointImporter>
#include <kvs/CommandLine>
#include <float.h>

#include <kvs/Directory>
#include <kvs/FileList>

kvs::PointObject** object;
kvs::glut::Timer* glut_timer;
std::string filename;
std::string objectname = "object";
kvs::TransferFunction tfunc( 256 );
float MIN = FLT_MAX;
float MAX = -FLT_MAX;

int    msec;
int    nsteps;
int    time_step;

void initialize( std::string filename )
{
    time_step = 0;
//    filename = "5/Density";

    msec = 20;
    nsteps = 0;
    kvs::Directory directory( filename );
    const kvs::FileList files = directory.fileList();
    int file_length = files.size();
    std::vector<std::string> file_name;
    for ( int i = 0; i < file_length; i++ )
    {
        const kvs::File file = files[i];
        if( file.extension() == "kvsml" )
        {
            nsteps++;
            file_name.push_back( file.filePath() );
        }
    }
    
    object = new kvs::PointObject*[nsteps];
    std::cout << nsteps << std::endl;
    
    kvs::Timer time;
    time.start();
    for ( int i = 0; i < nsteps; i++ )
    { 
        object[i] = new kvs::PointImporter( file_name[i] );
        object[i]->setName( "object" );
        std::cout << "\r" << i << std::flush;        
    }
    time.stop();
    std::cout << "\r" << "                           " << std::flush;
    std::cout << "\r" << "Finish Reading." << std::endl;
    std::cout <<"Loading time: " <<time.msec()<<"msec"<<std::endl;
    time_step = 0;
}

class Label : public kvs::glut::Label
{
public:
    
    Label( kvs::ScreenBase* screen ):
    kvs::glut::Label( screen )
    {
        setTextColor( kvs::RGBColor( 0, 0, 0 ) );
        setMargin( 10 );
    }
    
    void screenUpdated( void )
    {
        char* buf = new char[256];
        sprintf( buf, "Time step : %03d", time_step );
        setText( std::string( buf ).c_str() );
    }
};

class TimeSlider : public kvs::glut::Slider
{
public:
    
    TimeSlider( kvs::glut::Screen* screen ):
    kvs::glut::Slider( screen ){};
    
    void valueChanged( void )
    {
        glut_timer->stop();
        time_step = int( this->value() );
        kvs::glew::PartcileVolumeRenderer* renderer = new kvs::glew::PartcileVolumeRenderer();
        renderer->setName( "renderer" );
        renderer->enableShading();
        renderer->setShader( kvs::Shader::Phong( 0.5, 0.5, 0.8, 15.0 ) );
//        tfunc.setRange( 10.0, object[time_step]->maxValue() );
        renderer->setTransferFunction( tfunc );
        
        screen()->objectManager()->change( "object", object[time_step], false );
        screen()->rendererManager()->change( "renderer", renderer, true );
        screen()->redraw();
    }
};

Label* label;
TimeSlider* slider;

class TimerEvent : public kvs::TimerEventListener
{
    void update( kvs::TimeEvent* event )
    {

        kvs::glew::PartcileVolumeRenderer* renderer = new kvs::glew::PartcileVolumeRenderer();
        renderer->setName( "renderer" );
        renderer->enableShading();
        renderer->setShader( kvs::Shader::Phong( 0.5, 0.5, 0.8, 15.0 ) );
        renderer->setTransferFunction( tfunc );
        
        screen()->objectManager()->change( "object", object[time_step++], false );
        screen()->rendererManager()->change( "renderer", renderer, true );
        slider->setValue( (float)time_step );
        std::cout << "\r" << time_step <<std::flush;
        screen()->redraw();
        if( time_step == nsteps ) 
        {
            time_step = 0;
            glut_timer->stop();
        }
    }
};

class KeyPressEvent : public kvs::KeyPressEventListener
{
    void update( kvs::KeyEvent* event )
    {
        switch ( event->key() )
        {
            case kvs::Key::s:
            {
                if ( glut_timer-> isStopped() )
                {
                    glut_timer->start();
                    screen()->redraw();
                }
                else
                {
                    glut_timer->stop();
                    screen()->redraw();
                }
                break;
                
            }
        }
    }
};

int main( int argc, char** argv )
{
    initialize( argv[1] );
    if ( argc > 2 )
        tfunc = kvs::TransferFunction( argv[2]);
    
    kvs::glut::Application app( argc, argv );
    
    KeyPressEvent     key_press_event;
    TimerEvent        timer_event;

    kvs::glut::Screen screen( &app );
    glut_timer = new kvs::glut::Timer( msec );
    
    kvs::glew::PartcileVolumeRenderer* renderer = new kvs::glew::PartcileVolumeRenderer();
    renderer->setName( "renderer" );
    renderer->enableShading();
    renderer->setShader( kvs::Shader::Phong( 0.5, 0.5, 0.8, 15.0 ) );
    renderer->setTransferFunction( tfunc );
    screen.registerObject( object[0], renderer );

    screen.addKeyPressEvent( &key_press_event );
    screen.addTimerEvent( &timer_event, glut_timer );
    screen.setGeometry( 0, 0, 800, 600 );
//    screen.background()->setColor( kvs::RGBColor( 0, 0, 0 ) );
    screen.setTitle( "AnimationRayCasting" );
    screen.show();
    
    //lable
    label = new Label( &screen );
    label->setX( screen.width() * 0.25 );
    label->setY( screen.height() - 80 );
    label->show();
    
    // slider
    slider = new TimeSlider( &screen );
    slider->setSliderColor( kvs::RGBColor( 0, 0, 0 ) );
    slider->setX( screen.width() * 0.25 );
    slider->setY( screen.height() - 80 );
    slider->setWidth( screen.width() / 2 );
    slider->setValue( 0.0 );
    slider->setRange( 0.0, nsteps );
    slider->setMargin( 15 );
    slider->setCaption("");
    slider->setTextColor( kvs::RGBColor( 0, 0, 0 ) );
    slider->show();
    
    glut_timer->start( msec );
    glut_timer->stop();
    
    return( app.run() );
}

