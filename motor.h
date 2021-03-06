// Copyright 2018 by Kevin Dahlhausen

#include <ArduinoLog.h>
#include <Audio.h>

#define SHORT_FADE_TIME_MS 400
#define LONG_FADE_TIME_MS 800

/*
 
   Handles the motor sound.
   Starter sound plays as long as start-button held down.
   Then transitions to starting sound for one play,
   then to idle.

*/

class Motor
{
    public:

        Motor();

        // viaStarter = if true, will run the 'starting' sound
        // until MOTOR_STARTER_STOP event received
        // otherwise will run the start sound once and transition
        // to starting sound
        void start(bool viaStarter=false);

        void stopStarter();
        
        void stop();

        // 0 - 100, TODO: define 0 speed, idle speed etc
        void setSpeed(int speed);
        
        void onEvent(int event, void *param);
        void update();



    protected:

        enum State {
            stopped,
            starter_starting,
            starter_looping,
            starting,
            idle,
            running,
            fading

        };
        State state;

        int speed; // 0-100?

        typedef struct {
            AudioPlaySdWav &sdWav;
            AudioEffectFade &fader;
            char finalMixerChannel;  // for setting gain
            bool loop;
            uint32_t timeStarted;
            const char *absolutePathToSound;
        } SoundChannel;

        SoundChannel channel1;
        SoundChannel channel2;

        SoundChannel *currentChannel;
        SoundChannel *nextChannel;

        // set true is we receive 'stop'
        // (will complete any transitions and then transition to stopped)
        bool shouldStop;

        // TODO: refactor to 'bool autoStartSequence'
        bool shouldLoopStarter; // loop starting sound until MOTOR_STARTER_STOP?

        // state to change to after current fade
        State nextStateAfterFade;
        int currentFadeTimeMs;

        void fadeTo( const char *fileName, float gain, bool loop,  State nextState, int fadeTimeMs=SHORT_FADE_TIME_MS);
        // fade into the sound at fileName and transition to 'nextState'

        inline bool soundStartDelayHasPassed()
        // has enough time passed since the sound started that isPlaying is valid?
        {
            return millis() >= currentChannel->timeStarted + 5; // PJRC max 3 ms to start sound
        }

        inline bool currentChannelIsPlaying()
        {
            return currentChannel->sdWav.isPlaying();
        }

        void changeFromStarterToStarting();
        // this is done in multiple places so refctored to a func.
};
