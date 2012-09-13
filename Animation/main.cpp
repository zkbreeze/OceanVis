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
#include <kvs/glew/RayCastingRenderer>
#include <kvs/StructuredVolumeObject>
#include <kvs/CommandLine>
#include "StructuredVolumeImporter.h"
#include "Argument.cpp"
#include <float.h>

kvs::StructuredVolumeObject** object;
kvs::glut::Timer* glut_timer;
std::string filename;
std::string objectname = "object";
kvs::TransferFunction tfunc( 256 );

int    msec;
int    nsteps;
int    time_step;

std::string createString( const int step, const std::string filename )
{
    char* buf = new char[256];
    sprintf( buf, "_%d.rawiv", step );
    std::string str = filename;
    str += buf;
    return( str );
}

void initialize( void )
{
    time_step = 0;
    filename = "../../../../Data/RV2/Gogcm_fnk.ctl";

    msec = 50;
    nsteps = 50;
    object = new kvs::StructuredVolumeObject*[nsteps];
    const size_t vindex = 3;
    const bool zfilp = true;
    const kvs::StructuredVolumeObject::GridType grid_type = kvs::StructuredVolumeObject::Uniform;
    
    kvs::Timer time;
    time.start();
    for ( int i = 0; i < nsteps; i++ )
    {
        size_t tindex = i;
        object[i] = new util::StructuredVolumeImporter( filename, vindex, tindex, zfilp, grid_type );
        object[i]->setName( "object" );
        std::cout << "\r" << i << std::flush;
//        
//        size_t NumberOfNodes = object[i]->nnodes();
//        float maxValue = object[i]->maxValue();
//        float minValue = object[i]->minValue();
//        
//        kvs::StructuredVolumeObject* uOjbect = new util::StructuredVolumeImporter( filename, 0, tindex, zfilp, grid_type );
//        kvs::StructuredVolumeObject* vObject = new util::StructuredVolumeImporter( filename, 1, tindex, zfilp, grid_type );
//        
//        float* uScalar = (float*)uOjbect->values().pointer();
//        float* vScalar = (float*)vObject->values().pointer();
//        float* opacity = new float[NumberOfNodes];
//        float min = FLT_MAX;
//        float max = 0.0;
//        for ( size_t j = 0; j < NumberOfNodes; j++)
//        {
//            opacity[j] = std::sqrt( uScalar[j] * uScalar[j] + vScalar[j] * vScalar[j] );
//            if ( opacity[j] > max ) max = opacity[j];
//            if ( opacity[j] < min ) min = opacity[j];
//        }
//        
//        //Normalize and evaluation
//        float* pvalues = (float*)object[i]->values().pointer();
//        float range = max - min;
//        for ( size_t k = 0; k < NumberOfNodes; k++)
//        {
//            opacity[k] = ( opacity[k] - min ) / range;
//            pvalues[k] = pvalues[k] * opacity[k];
//        }
//        delete opacity;        
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
        kvs::glew::RayCastingRenderer* renderer = new kvs::glew::RayCastingRenderer();
        renderer->setName( "renderer" );
        renderer->enableShading();
        renderer->setShader( kvs::Shader::Phong( 0.5, 0.5, 0.8, 15.0 ) );
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

        kvs::glew::RayCastingRenderer* renderer = new kvs::glew::RayCastingRenderer();
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
    initialize();
    kvs::glut::Application app( argc, argv );
    
    KeyPressEvent     key_press_event;
    TimerEvent        timer_event;

    kvs::glut::Screen screen( &app );
    glut_timer = new kvs::glut::Timer( msec );
    
    kvs::glew::RayCastingRenderer* renderer = new kvs::glew::RayCastingRenderer();
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
    
    label = new Label( &screen );
    label->setX( screen.width() * 0.25 );
    label->setY( screen.height() - 80 );
    label->show();
    
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

