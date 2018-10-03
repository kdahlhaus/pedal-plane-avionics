// Copyright 2018 by Kevin Dahlhausen
//
#ifndef _interpreter_h
#define _interpreter_h

#include <SerialCommand.h>

/*
    Monitor a serial device for simple commands.
    Most commands end up sending an event.
    Commands are defined in interpreter.cpp.

*/

class SerialInterpreter
{
    public:
        SerialInterpreter(Stream &serialToUse=Serial);

    void update()
    {
        serial_command.readSerial();
    }

    friend void navlights_curve();
    friend void set_gain();
    friend void motor_set_speed();

    protected:
        SerialCommand serial_command;
};



#endif