// Copyright 2018 by Kevin Dahlhausen
#include <stdio.h>
#include "sound_manager.h"
#include <ArduinoLog.h>

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioPlaySdWav           playSdWav2;     //xy=173,175
AudioPlaySdWav           playSdWav1;     //xy=174,118
AudioPlaySdWav           playSdWav3;     //xy=174,239
AudioPlaySdWav           motorSdWav2;     //xy=208,473
AudioPlaySdWav           motorSdWav1;     //xy=210,412
AudioMixer4              sfxMixer1;         //xy=368,143
AudioMixer4              mixer2;         //xy=370,259
AudioEffectFade          motorFade1;          //xy=383,411
AudioEffectFade          motorFade2;          //xy=384,473
AudioMixer4              finalMixer;         //xy=689,283
AudioAmplifier           amp1;           //xy=845,283
AudioOutputI2S           i2s1;           //xy=996,284
AudioConnection          patchCord1(playSdWav2, 0, sfxMixer1, 2);
AudioConnection          patchCord2(playSdWav2, 1, sfxMixer1, 3);
AudioConnection          patchCord3(playSdWav1, 0, sfxMixer1, 0);
AudioConnection          patchCord4(playSdWav1, 1, sfxMixer1, 1);
AudioConnection          patchCord5(playSdWav3, 0, mixer2, 0);
AudioConnection          patchCord6(playSdWav3, 1, mixer2, 1);
AudioConnection          patchCord7(motorSdWav2, 0, motorFade2, 0);
AudioConnection          patchCord8(motorSdWav2, 1, motorFade2, 0);
AudioConnection          patchCord9(motorSdWav1, 0, motorFade1, 0);
AudioConnection          patchCord10(motorSdWav1, 1, motorFade1, 0);
AudioConnection          patchCord11(sfxMixer1, 0, finalMixer, 0);
AudioConnection          patchCord12(mixer2, 0, finalMixer, 1);
AudioConnection          patchCord13(motorFade1, 0, finalMixer, 2);
AudioConnection          patchCord14(motorFade2, 0, finalMixer, 3);
AudioConnection          patchCord15(finalMixer, amp1);
AudioConnection          patchCord16(amp1, 0, i2s1, 0);
AudioConnection          patchCord17(amp1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=882,396
// GUItool: end automatically generated code



// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14
 
// Use these with the Teensy 3.5 & 3.6 SD card
//#define SDCARD_CS_PIN    BUILTIN_SDCARD
//#define SDCARD_MOSI_PIN  11  // not actually used
//#define SDCARD_SCK_PIN   13  // not actually used

// Use these for the SD+Wiz820 or other adaptors
//#define SDCARD_CS_PIN    4
//#define SDCARD_MOSI_PIN  11
//#define SDCARD_SCK_PIN   13


// # of SFX sounds that can play at once
#define NUMBER_OF_CHANNELS 3 

#define NO_CHANNEL -1
#define NO_PRIORITY -1

#define HANDLE_TO_INDEX(h) ((int)(h)-1)

typedef struct ChannelInfo_s {
    const char *filename;
    int priority; 
    bool loop;
    AudioPlaySdWav& sdWav;
    ChannelInfo_s(AudioPlaySdWav& s) : priority(NO_PRIORITY), loop(false), sdWav(s) {}
} ChannelInfo;

ChannelInfo channels[] = {
    ChannelInfo(playSdWav1),
    ChannelInfo(playSdWav2),
    ChannelInfo(playSdWav3)
};


void *SoundManager::play(const char *filename, int priority, bool loop, float gain)
{
    char fbuf[10];
    dtostrf(gain, 4, 2, fbuf);
    Log.trace(F("play(%s, pri=%d, loop=%b, gain=%s)\n"), filename, priority, loop, fbuf);

    //find min priority
    int min_priority = 999;
    int min_priority_channel = NO_CHANNEL;
    int channel_of_equal_priority = NO_CHANNEL;

    // find candidate channel
    for (int i=0; i< NUMBER_OF_CHANNELS; i++)
    {
        // first not playing channel
        if (!channels[i].sdWav.isPlaying())
        {
            min_priority_channel = i;
            Log.trace(F("found not playing channel %d\n"), i);
            break;
        }
    }
    if (min_priority_channel == NO_CHANNEL)
    {
        for (int i=0; i<NUMBER_OF_CHANNELS; i++)
        {
            // determine channel w/minium priority that is less than new sound priority
            int channel_priority = channels[i].priority;
            if (channel_priority < min_priority && channel_priority < priority)
            {
                min_priority = channel_priority;
                min_priority_channel = i;
            }
        }
    }
    // new sounds of equal priority replace old sounds of that same priority
    if (min_priority_channel == NO_CHANNEL)
    {
        // find channel of equal priority
        for (int i=0; i< NUMBER_OF_CHANNELS; i++)
        {
            if (channels[i].priority == priority)
            {
                min_priority = channels[i].priority;
                min_priority_channel = i;
                break;
            }
        } 

    }
    if (min_priority_channel == NO_CHANNEL)
    {
        // not able to play sound at this time
        Log.trace("no channels available for sound\n");
        return (void *)0;
    }

    
    // start playing on channel 'min_priority_channel'
    Log.trace(F("starting snd on %d"), min_priority_channel);
    if (channels[min_priority_channel].sdWav.isPlaying()) {
        channels[min_priority_channel].sdWav.stop(); 
        Log.trace(F("stopped sound to make room for new one\n"));
    }
    channels[min_priority_channel].priority = priority;
    channels[min_priority_channel].sdWav.play(filename);
    channels[min_priority_channel].loop = loop;
    channels[min_priority_channel].filename = filename;

    sfxMixer1.gain(min_priority_channel*2, gain);
    sfxMixer1.gain(min_priority_channel*2+1, gain);

    // wait for sound to start playing
    for (int i=0; i<20; i++)
    {
        if (channels[min_priority_channel].sdWav.isPlaying())
        {
            break;
        }
        delay(5);
    }
    
    return (void *)(min_priority_channel + 1); // sound 'handle' is channel + 1
}

void SoundManager::setGain(float gain)
{
    amp1.gain(gain);
    char fbuf[10];
    dtostrf(gain, 4, 2, fbuf); 
    Log.trace(F("SM.setGain(%s) 1\n"), fbuf);
}


void SoundManager::update()
{

    //sgtl5000_1.volume(vol); 
    float newVol = readVolumePotentiometer();

    if (abs(newVol - currentVolume)>0.01) {
        currentVolume = newVol;
        setGain(currentVolume);
    }

    // handle 'loop' sounds
    for (int i=0; i<NUMBER_OF_CHANNELS; i++)
    {
        if (channels[i].loop && !channels[i].sdWav.isPlaying())
        {
            channels[i].sdWav.play(channels[i].filename);
            // wait for sound to start playing
            for (int i=0; i<20; i++)
            {
                if (channels[i].sdWav.isPlaying())
                {
                    break;
                }
                delay(5);
            }
            Log.trace("restarted %s\n", channels[i].filename);
        }
    }

}


void SoundManager::stop(void *handle)
{
    int index = HANDLE_TO_INDEX(handle);
    if (handle && channels[index].sdWav.isPlaying())
    {
        channels[index].sdWav.stop();
        channels[index].loop = false;
        channels[index].priority = -1;
    }
}

bool SoundManager::is_playing(void *handle)
{
    bool ip = handle && channels[HANDLE_TO_INDEX(handle)].sdWav.isPlaying(); 
    //Log.trace("ip(%d)=%b\n", (int)(handle), ip);
    return ip;
}

void SoundManager::setup()
{
    AudioMemory(8);

    // Comment these out if not using the audio adaptor board.
    // This may wait forever if the SDA & SCL pins lack
    // pullup resistors
    sgtl5000_1.enable();
    sgtl5000_1.volume(0.5);

    SPI.setMOSI(SDCARD_MOSI_PIN);
    SPI.setSCK(SDCARD_SCK_PIN);
    if (!(SD.begin(SDCARD_CS_PIN))) {
        // stop here, but print a message repetitively
        while (1) {
          Log.error("Unable to access the SD card\n");
          delay(500);
        }
    } 

    currentVolume = readVolumePotentiometer();
    setGain(currentVolume);
}

float SoundManager::readVolumePotentiometer() 
{
    // comment this if the optional volume
    // pot was not added to the audio board
    float vol = analogRead(15);
    return vol / 1024; 
} 