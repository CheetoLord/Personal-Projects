
#include "soundHandler.h"


unsigned char readChar(ifstream &theFile) {
    unsigned char retval;
    theFile >> retval;
    return retval;
}


unsigned short readShort(ifstream &theFile, bool bigEndian) {
    unsigned short retval;
    if(bigEndian) {
        retval = ((unsigned short) readChar(theFile)) << 8;
        retval += (unsigned short) readChar(theFile);
    } else {
        retval = (unsigned short) readChar(theFile);
        retval += ((unsigned short) readChar(theFile)) << 8;
    }
    return retval;
}
unsigned short readShort(ifstream &theFile) {return readShort(theFile, true);}


unsigned int readInt(ifstream &theFile, bool bigEndian) {
    unsigned int retval;
    if(bigEndian) {
        retval = ((unsigned int) readShort(theFile)) << 16;
        retval += (unsigned int) readShort(theFile);
    } else {
        retval = (unsigned int) readShort(theFile, false);
        retval += ((unsigned int) readShort(theFile, false)) << 16;
    }
    return retval;
}
unsigned int readInt(ifstream &theFile) {return readInt(theFile, true);}


sound::sound(string fileName, string name) {
    myName = name;
    myFile = fileName;
    soundIsReady = false;

    string extension = fileName.substr(fileName.length()-3);
    if(extension == "wav") {
        loadWav();
    } else {
        throw runtime_error("Couldn't recognize file extension \'" + extension +
                            "\' (in " + fileName + ").\nThis could be a typo in the extension name," +
                            "\nor it is possible that this filetype is not yet supported.");
    }
    soundIsReady = true;
}

bool sound::isReady() {
    return soundIsReady;
}

vector<int> *sound::getSamples() {
    return &mySamples;
}

int sound::getSampleLength() {
    return mySamples.size();
}

void sound::loadWav() {
    ifstream wavFile(myFile);
    unsigned int ChunkID = readInt(wavFile);
    unsigned int ChunkSize = readInt(wavFile, false);
    unsigned int Format = readInt(wavFile);
    unsigned int SubChunk1ID = readInt(wavFile);
    unsigned int SubChunk1Size = readInt(wavFile, false);
    unsigned short AudioFormat = readShort(wavFile, false);
    unsigned short NumChannels = readShort(wavFile, false);
    unsigned int SampleRate = readInt(wavFile, false);
    unsigned int ByteRate = readInt(wavFile, false);
    unsigned short BlockAlign = readShort(wavFile, false);
    unsigned short BitsPerSample = readShort(wavFile, false);
    unsigned int SubChunk2ID = readInt(wavFile);
    unsigned int SubChunk2Size = readInt(wavFile, false);

    bool printData = false;
    if(printData) {
        cout << "ChunkID: " << ChunkID << endl;
        cout << "ChunkSize: " << ChunkSize << endl;
        cout << "Format: " << Format << endl;
        cout << "SubChunk1ID: " << SubChunk1ID << endl;
        cout << "SubChunk1Size: " << SubChunk1Size << endl;
        cout << "AudioFormat: " << AudioFormat << endl;
        cout << "NumChannels: " << NumChannels << endl;
        cout << "SampleRate: " << SampleRate << endl;
        cout << "ByteRate: " << ByteRate << endl;
        cout << "BlockAlign: " << BlockAlign << endl;
        cout << "BitsPerSample: " << BitsPerSample << endl;
        cout << "SubChunk2ID: " << SubChunk2ID << endl;
        cout << "SubChunk2Size: " << SubChunk2Size << endl;
    }

    int integersToRead = SubChunk2Size/4;
    cout << integersToRead << endl;
    mySamples.resize(integersToRead);
    for(int i = 0; i < integersToRead; i++) {
        if(i % 10000000 == 0) {
            cout << i << "/" << integersToRead << endl;
        }
        mySamples[i] = readInt(wavFile, false);
    }
    return;
}








soundHandler::soundHandler() {
    mySampleRate = 44100;
}

void soundHandler::initAudioDevice() {
    int error;
    error = snd_pcm_open(&myHandle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if(error < 0) {
        cout << "Couldn't open da ting: " << snd_strerror(error) << endl;
        throw runtime_error("it didn't open");
    }

    error = snd_pcm_set_params(myHandle, SND_PCM_FORMAT_S32_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 2, mySampleRate, 1, 1000000);
    if(error < 0) {
        cout << "Couldn't set da params: " << snd_strerror(error) << endl;
        throw runtime_error("da params won't set");
    }
}

void soundHandler::loadSound(string fileName, string name) {
    myLoadedSounds[name] = new sound(fileName, name);
}

void soundHandler::playSound(string name) {
    myPlayingSounds.push_back(soundData(name));
}

void soundHandler::runSounds() {
    vector<unsigned int> theBuffer;
    theBuffer.resize(2*mySampleRate);
    for(int i = 0; i < 2*mySampleRate; i++) {
        theBuffer[i] = 0;
    }
    for(int i = 0; i < myPlayingSounds.size(); i++) {
        soundData *theSoundData = &myPlayingSounds[i];
        sound *theSound = myLoadedSounds[theSoundData->name];
        if(!theSound->isReady()) {continue;}
        if(theSoundData->paused) {continue;}
        vector<int> *theSamples = theSound->getSamples();

        for(int i = theSoundData->soundPos; i < theSoundData->soundPos + 2*mySampleRate && i < theSound->getSampleLength(); i++) {
            theBuffer[i % (mySampleRate*2)] += theSamples->operator[](i) * theSoundData->amp;
        }
        theSoundData->soundPos += 2*mySampleRate;
    }
   
    for(int i = 0; i < 3; i++) {
        cout << theBuffer[i] << endl;
    }
    cout << endl;
    int frames = snd_pcm_writei(myHandle, (void*)(&theBuffer[0]), theBuffer.size()/2);
    cout << frames << " frames played\n";
}

void soundHandler::playSection(int section) {
    vector<int> theBuffer;
    theBuffer.resize(2*mySampleRate);
    for(int i = 0; i < 2*mySampleRate; i++) {
        theBuffer[i] = 0;
    }
    for(int i = 0; i < myPlayingSounds.size(); i++) {
        soundData *theSoundData = &myPlayingSounds[i];
        sound *theSound = myLoadedSounds[theSoundData->name];
        if(!theSound->isReady()) {continue;}
        if(theSoundData->paused) {continue;}
        vector<int> *theSamples = theSound->getSamples();

        for(int i = 2*mySampleRate*section; i < 2*mySampleRate*(section+1); i++) {
            theBuffer[i % (mySampleRate*2)] += theSamples->operator[](i);
        }
    }

    int frames = snd_pcm_writei(myHandle, (void*)(&theBuffer[0]), theBuffer.size()/2);
    printf("frames: %i\n", frames);
    if (frames < 0) {
        frames = snd_pcm_recover(myHandle, frames, 0);
        cout << "recovered\n";
    }

    if (frames < 0) {
        printf("snd_pcm_writei failed: %s\n", snd_strerror(frames));
    }
    if (frames > 0 && frames < theBuffer.size()/2)
        printf("Short write (expected %li, wrote %i)\n", theBuffer.size()/2, frames);
}


void soundHandler::sanityCheck() {
    for (; ;) {
        vector<int> theBuffer;
        theBuffer.resize(2*mySampleRate);
        for(int i = 0; i < 2*mySampleRate; i++) {
            theBuffer[i] = 0;
        }
        for(int i = 0; i < myPlayingSounds.size(); i++) {
            soundData *theSoundData = &myPlayingSounds[i];
            sound *theSound = myLoadedSounds[theSoundData->name];
            if(!theSound->isReady()) {continue;}
            if(theSoundData->paused) {continue;}
            vector<int> *theSamples = theSound->getSamples();

            for(int i = theSoundData->soundPos; i < theSoundData->soundPos + 2*mySampleRate && i < theSound->getSampleLength(); i++) {
                theBuffer[i % (mySampleRate*2)] += theSamples->operator[](i) * theSoundData->amp;
            }
            
            theSoundData->soundPos += 2*mySampleRate;
        }

        int frames = snd_pcm_writei(myHandle, (void*)(&theBuffer[0]), theBuffer.size()/2);
	    printf("frames: %i\n", frames);
        if (frames < 0)
            frames = snd_pcm_recover(myHandle, frames, 0);

        if (frames < 0) {
            printf("snd_pcm_writei failed: %s\n", snd_strerror(frames));
            break;
        }
        if (frames > 0 && frames < theBuffer.size()/2)
            printf("Short write (expected %li, wrote %i)\n", theBuffer.size()/2, frames);
    }
}
