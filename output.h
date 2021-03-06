// Copyright 2018 Kevin Dahlhausen

#ifndef _output_h
#define _output_h

#include <ArduinoLog.h>

/*
    A generic output that responds to events.
    The constructor will set the pin mode. 
*/

class Output
{
    public:
        Output(int pin, int high_event, int low_event);
        void setHigh();
        void setLow();
        void onEvent(int event, void *param);

    protected:
        int pin;
        int high_event;
        int low_event;
};

#endif
