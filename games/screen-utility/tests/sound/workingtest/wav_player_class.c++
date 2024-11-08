#include <stdio.h>
#include <alsa/asoundlib.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

using namespace std;

#ifndef WAV_PLAYER
#define WAV_PLAYER




int getFileLength(string filename)
{
    FILE *p_file = NULL;
    p_file = fopen(filename.c_str(),"rb");
    fseek(p_file,0,SEEK_END);
    int size = ftell(p_file);
    fclose(p_file);
    return size;
}

string toBin(unsigned char a) {
	cout << (unsigned int) a << " (" << a << "): ";
	string result = "";
	if(a & (0b10000000)) {result += "1";} else {result += "0";}
	if(a & (0b01000000)) {result += "1";} else {result += "0";}
	if(a & (0b00100000)) {result += "1";} else {result += "0";}
	if(a & (0b00010000)) {result += "1";} else {result += "0";}
	if(a & (0b00001000)) {result += "1";} else {result += "0";}
	if(a & (0b00000100)) {result += "1";} else {result += "0";}
	if(a & (0b00000010)) {result += "1";} else {result += "0";}
	if(a & (0b00000001)) {result += "1";} else {result += "0";}
	return result;
}

unsigned char readChar(ifstream &theFile) {
    char retval;
    theFile.get(retval);
	//cout << toBin(retval) << endl;
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


class WavPlayer {
public:
	WavPlayer(string fileName) {
		WavFilePath = fileName;

		canPlay = false;

		ReadHeader();

		result = initALSI();
		if(result) {
			cout << "Error! Could not initialize ALSI.\n\n";
			return;
		} else {
			canPlay = true;
		}
	}

	void play() {
		WavFile = open(WavFilePath.c_str(), O_RDONLY);

		cout << "\n\nplaying (" << seconds << " seconds)\n";

		for (loops = (seconds * 1000000) / temp; loops > 0; loops--) {
			
			result = read(WavFile, buff, buff_size);
			if (result == 0) {
				printf("Early EOF\n");
				free(buff);
				return;
			}

			result = snd_pcm_writei(pcm_handle, buff, period);
			if(result == -EPIPE) {
				printf("XRUN occured\n");
				snd_pcm_prepare(pcm_handle);
			} else if (result < 0) {
				printf("Cannot write to PCM Device: %s\n", snd_strerror(result));
			}
		}
	}

	void close() {
		snd_pcm_drain(pcm_handle);
		snd_pcm_close(pcm_handle);
		free(buff);
	}

private:
	void ReadHeader() {
		ifstream wavFile(WavFilePath);

    	ChunkID = readInt(wavFile);
		if(ChunkID != 1380533830) { //no header
			cout << "no header!\n";
			NumChannels = 2;
			SampleRate = 44100;
			SubChunk2Size = -1;
			BitsPerSample = 16;
		} else {
			ChunkSize = readInt(wavFile, false);
			Format = readInt(wavFile);
			SubChunk1ID = readInt(wavFile);
			SubChunk1Size = readInt(wavFile, false);
			AudioFormat = readShort(wavFile, false);
			NumChannels = readShort(wavFile, false);
			SampleRate = readInt(wavFile, false);
			ByteRate = readInt(wavFile, false);
			BlockAlign = readShort(wavFile, false);
			BitsPerSample = readShort(wavFile, false);
			SubChunk2ID = readInt(wavFile);
			SubChunk2Size = readInt(wavFile);
		}

		cout << "ChunkID: " << ChunkID << endl;
		// cout << "ChunkSize: " << ChunkSize << endl;
		// cout << "Format: " << Format << endl;
		// cout << "SubChunk1ID: " << SubChunk1ID << endl;
		// cout << "SubChunk1Size: " << SubChunk1Size << endl;
		// cout << "AudioFormat: " << AudioFormat << endl;
		// cout << "NumChannels: " << NumChannels << endl;
		// cout << "SampleRate: " << SampleRate << endl;
		// cout << "ByteRate: " << ByteRate << endl;
		// cout << "BlockAlign: " << BlockAlign << endl;
		cout << "BitsPerSample: " << BitsPerSample << endl;
		// cout << "SubChunk2ID: " << SubChunk2ID << endl;
		cout << "SubChunk2Size: " << SubChunk2Size << endl;		

		wavFile.close();
	}

	int initALSI() {
		rate = SampleRate;
		channels = NumChannels;

		int sampleCount = getFileLength(WavFilePath) / 4;
		cout << sampleCount << endl;
		seconds = sampleCount / SampleRate;

		//Open PCM_DEVICE for playback
		result = snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
		if (result < 0) {
			printf("Cannot open PCM Device: %s\n", snd_strerror(result));
			return 1;
		}


		//Allocate Hardware Parameters Object
		snd_pcm_hw_params_alloca(&hw_params);

		snd_pcm_hw_params_any(pcm_handle, hw_params);

		//Set Parameters
		result = snd_pcm_hw_params_set_access(pcm_handle, hw_params,
							SND_PCM_ACCESS_RW_INTERLEAVED);
		if (result < 0) {
			printf("Unable to set interleaved mode: %s\n", snd_strerror(result));
			return 1;
		}

		result = snd_pcm_hw_params_set_format(pcm_handle, hw_params,
							SND_PCM_FORMAT_S16_LE);
		if(result < 0) {
			printf("Cannot set format: %s\n", snd_strerror(result));
			return 1;
		}


		result = snd_pcm_hw_params_set_channels(pcm_handle, hw_params, channels);
		if (result < 0) {
			printf("Cannot set channels: %s\n", snd_strerror(result));
			return 1;
		}


		result = snd_pcm_hw_params_set_rate_near(pcm_handle, hw_params, &rate, 0);
		if (result < 0) {
			printf("Unable to set rate: %s\n", snd_strerror(result));
			return 1;
		}

		//Write Params
		result = snd_pcm_hw_params(pcm_handle, hw_params);
		if (result < 0) {
			printf("Cannot write hardware params: %s\n", snd_strerror(result));
			return 1;
		}

		//Get Indentifier of PCM Handle
		printf("PCM name: '%s'\n", snd_pcm_name(pcm_handle));

		printf("PCM state: %s\n", snd_pcm_state_name(snd_pcm_state(pcm_handle)));

		snd_pcm_hw_params_get_channels(hw_params, &temp);
		printf("Channels: %i \n", temp);


		snd_pcm_hw_params_get_rate(hw_params, &temp, 0);
		printf("Rate: %d bps\n", temp);
		printf("Seconds: %d\n", seconds);

		//Extract period size from a configuration space. 
		result = snd_pcm_hw_params_get_period_size(hw_params, &period, 0);
		if(result < 0) {
			printf("Cannot extract period size: %s\n", snd_strerror(result));
			return 1;
		}

		buff_size = period * channels * 2;
		buff = (char *) malloc(buff_size);

		snd_pcm_hw_params_get_period_time(hw_params, &temp, NULL);

		system("amixer sset PCM,0 100%");

		return 0;
	}




	bool canPlay;

	int WavFile;
	string WavFilePath;

	unsigned int ChunkID;
	unsigned int ChunkSize;
	unsigned int Format;
	unsigned int SubChunk1ID;
	unsigned int SubChunk1Size;
	unsigned short AudioFormat;
	unsigned short NumChannels;
	unsigned int SampleRate;
	unsigned int ByteRate;
	unsigned short BlockAlign;
	unsigned short BitsPerSample;
	unsigned int SubChunk2ID;
	unsigned int SubChunk2Size;

	int result = 0;
	unsigned int temp = 0;
	snd_pcm_t *pcm_handle = NULL;
	snd_pcm_hw_params_t *hw_params = NULL;
	snd_pcm_uframes_t period = 0;

	unsigned int rate;
	int channels, seconds;
	int buff_size, loops;
	char *buff;
};



#endif


void startPlayer(string filename) {
	WavPlayer myPlayer(filename);
	myPlayer.play();
	myPlayer.close();
}


int main() {
	thread t1(startPlayer, "../sounds/RickrollButSlow.wav");
	this_thread::sleep_for(chrono::milliseconds(1000));
	startPlayer("../sounds/Rick-Astley-Never-Gonna-Give-You-Up-Official-Music-Video.wav");


	return 0;
}