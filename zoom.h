// Copyright 2018 by Kevin Dahlhausen

#ifndef _zoom_h_
#define _zoom_h_

#include "random_sound.h"
#include <Adafruit_LIS3DH.h>

/*

 Play a 'zoom' sound when acceleration of the plane
 exceeds a set value.

 Also responds to ZOOM1-ZOOMn messages to play the zooms.

*/
class Zoom
{
    public:
        Zoom();
        void onEvent(int event, void *param); 
        void update();

    private:
        RandomSound sounds;

        Adafruit_LIS3DH lis3dh;    
        uint32_t timeOfLastZoom;

};

#endif
