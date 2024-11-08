
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cassert>
#include "alsa/asoundlib.h"

using namespace std;

#ifndef _SOUND_HANDLER_H_
#define _SOUND_HANDLER_H_



class sound {
public:
    sound(string fileName, string name);
    void loadSound();
    bool isReady();
    vector<int> *getSamples();
    int getSampleLength();

private:

    void loadWav();


    vector<int> mySamples;
    bool soundIsReady;
    string myFile;
    string myName;

};



class soundHandler {
public:
    soundHandler();
    void initAudioDevice();
    void loadSound(string fileName, string name);
    void playSound(string name);
    void runSounds();
    void playSection(int section);

    void sanityCheck();

    struct soundData {
        soundData(string soundName) {
            name = soundName;
            soundPos = 0;
            amp = 1;
            paused = false;
        };
        string name;
        long soundPos;
        int amp;
        bool paused;
    };

private:
    int mySampleRate;
    snd_pcm_t *myHandle;
    snd_pcm_sframes_t frameCount; 
    map<string, sound*> myLoadedSounds;
    vector<soundData> myPlayingSounds;

};




#endif
