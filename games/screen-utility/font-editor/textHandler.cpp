
#include "textHandler.h"

#include <fstream>
#include <iostream>

using namespace std;



textHandler::textHandler(string fileName) {
    //set default metadata
    fontFileName = fileName;
    fontDetail = 5;
    fontName = "unknown";
    spacewidth = 5;
    charspacing = 1;

    myChars = new character*[128];
    for(int i = 0; i < 128; i++) {
        myChars[i] = nullptr;
    }

    //open font data file
    ifstream font(fileName);
    assert(font.is_open());

    string metaData = "";
    string firstLine;
    string fontString = "";
    getline(font, firstLine);

    //check if should read metadata
    bool readingMeta = firstLine == "meta";
    if(!readingMeta) {
        fontString += firstLine + "\n";
    }

    //read metadata
    string line;
    while(readingMeta) {
        if(!getline(font, line)) {break;}
        if(line == "end meta") {readingMeta = false; break;}

        int spacePos = line.find(" ");
        if(spacePos == string::npos) {continue;}

        string key = line.substr(0, spacePos);
        string data = line.substr(spacePos+1);
        if(key == "detail") {fontDetail = stoi(data);}
        else if(key == "name") {fontName = data;}
        else if(key == "spacewidth") {spacewidth = stoi(data);}
        else if(key == "charspacing") {charspacing = stoi(data);}
        else {cout << "Unknown metadata key: " << key << endl;}
    }

    //read rest of file
    while(!font.eof()) {
        getline(font, line);
        fontString += line + "\n";
    }
    font.close();

    //read all character datas
    string nextChar;
    int nextSplitPos = fontString.find("\n\n");
    while(nextSplitPos != string::npos) { //while there is next character
        nextChar = fontString.substr(0, nextSplitPos);
        fontString = fontString.substr(nextSplitPos+2);
        nextSplitPos = fontString.find("\n\n");
        
        char theChar = nextChar[0];
        nextChar = nextChar.substr(2);
        int currBitPos = 0;

        bool **bits = new bool*[fontDetail];
        for (int i = 0; i < fontDetail; i++) {
            bool *line = new bool[fontDetail];
            for (int j = 0; j < fontDetail; j++) {
                line[j] = nextChar[currBitPos] == '1';
                currBitPos++;
            }
            currBitPos++;
            bits[i] = line;
        }

        myChars[(int)theChar] = new character(theChar, bits, fontDetail);

    }

}

character *textHandler::getCharacterClass(char c) {
    if(!hasChar(c)) {
        myChars[(int)c] = new character(c, fontDetail);
        return myChars[(int)c];
    }
    return myChars[(int)c];
}

bool **textHandler::getChar(char c) {
    return myChars[(int)c]->getBitmap();
}

bool textHandler::hasChar(char c) {
    return myChars[(int)c] != nullptr;
}


void textHandler::reloadFromFile() {
    //set default metadata
    fontDetail = 5;
    fontName = "unknown";
    spacewidth = 5;
    charspacing = 1;

    myChars = new character*[128];
    for(int i = 0; i < 128; i++) {
        myChars[i] = nullptr;
    }

    //open font data file
    ifstream font(fontFileName);
    assert(font.is_open());

    string metaData = "";
    string firstLine;
    string fontString = "";
    getline(font, firstLine);

    //check if should read metadata
    bool readingMeta = firstLine == "meta";
    if(!readingMeta) {
        fontString += firstLine + "\n";
    }

    //read metadata
    string line;
    while(readingMeta) {
        if(!getline(font, line)) {break;}
        if(line == "end meta") {readingMeta = false; break;}

        int spacePos = line.find(" ");
        if(spacePos == string::npos) {continue;}

        string key = line.substr(0, spacePos);
        string data = line.substr(spacePos+1);
        if(key == "detail") {fontDetail = stoi(data);}
        else if(key == "name") {fontName = data;}
        else if(key == "spacewidth") {spacewidth = stoi(data);}
        else if(key == "charspacing") {charspacing = stoi(data);}
        else {cout << "Unknown metadata key: " << key << endl;}
    }

    //read rest of file
    while(!font.eof()) {
        getline(font, line);
        fontString += line + "\n";
    }
    font.close();

    //read all character datas
    string nextChar;
    int nextSplitPos = fontString.find("\n\n");
    while(nextSplitPos != string::npos) { //while there is next character
        nextChar = fontString.substr(0, nextSplitPos);
        fontString = fontString.substr(nextSplitPos+2);
        nextSplitPos = fontString.find("\n\n");
        
        char theChar = nextChar[0];
        nextChar = nextChar.substr(2);
        int currBitPos = 0;

        bool **bits = new bool*[fontDetail];
        for (int i = 0; i < fontDetail; i++) {
            bool *line = new bool[fontDetail];
            for (int j = 0; j < fontDetail; j++) {
                line[j] = nextChar[currBitPos] == '1';
                currBitPos++;
            }
            currBitPos++;
            bits[i] = line;
        }

        myChars[(int)theChar] = new character(theChar, bits, fontDetail);

    }
}

void textHandler::saveFontChangesToFile() {
    ofstream outputFile(fontFileName);
    assert(outputFile.is_open());

    //write metadata
    outputFile << "meta\n";
    outputFile << "detail " << to_string(fontDetail) << endl;
    outputFile << "name " << fontName << endl;
    outputFile << "spacewidth " << to_string(spacewidth) << endl;
    outputFile << "charspacing " << to_string(charspacing) << endl;
    outputFile << "end meta\n";

    //write chars
    for(int i = 0; i < 128; i++) {
        if(hasChar((char)i)) {
            outputFile << (char)i << endl;
            bool **bitmap = myChars[i]->getBitmap();
            for(int y = 0; y < fontDetail; y++) {
                for(int x = 0; x < fontDetail; x++) {
                    outputFile << bitmap[y][x] ? "1" : "0";
                }
                outputFile << endl;
            }
            outputFile << endl;
        }
    }
}





character::character(char c, bool **bits, int FD) {
    fontDetail = FD;
    myChar = c;
    
    bitmap = new bool*[fontDetail];
    bool *line;
    bool isEmpty = true;
    for (int i = 0; i < fontDetail; i++) {
        line = new bool[fontDetail];
        for (int j = 0; j < fontDetail; j++) {
            line[j] = bits[i][j];
            if(bits[i][j]) {
                isEmpty = false;
            }
        }
        bitmap[i] = line;
    }

    leadingWhitespace = 0;
    endingWhitespace = 0;
    if(isEmpty) { //following loops will read outide bounds if this is not checked
        return;
    }

    while(colIsEmpty(leadingWhitespace)) {
        leadingWhitespace++;
    }

    while(colIsEmpty(fontDetail - endingWhitespace - 1)) {
        endingWhitespace++;
    }
}

character::character(char c, int FD) {
    fontDetail = FD;
    myChar = c;
    
    bitmap = new bool*[fontDetail];
    bool *line;
    for (int i = 0; i < fontDetail; i++) {
        line = new bool[fontDetail];
        for (int j = 0; j < fontDetail; j++) {
            line[j] = false;
        }
        bitmap[i] = line;
    }

    leadingWhitespace = 0;
    endingWhitespace = 0;
}

void character::updateBitmap(bool **newBitmap) {
    bitmap = newBitmap;

    bool isEmpty = true;
    for (int i = 0; i < fontDetail; i++) {
        for (int j = 0; j < fontDetail; j++) {
            if(newBitmap[i][j]) {
                isEmpty = false;
                break;
            }
        }
        if(!isEmpty) {break;}
    }

    leadingWhitespace = 0;
    endingWhitespace = 0;
    if(isEmpty) { //following loops will read outide bounds if this is not checked
        return;
    }

    while(colIsEmpty(leadingWhitespace)) {
        leadingWhitespace++;
    }

    while(colIsEmpty(fontDetail - endingWhitespace - 1)) {
        endingWhitespace++;
    }
}

bool character::colIsEmpty(int col) {
    for(int i = 0; i < fontDetail; i++) {
        if(bitmap[i][col]) {
            return false;
        }
    }
    return true;
}


bool **character::getBitmap() {
    return bitmap;
}



int getAvailFonts(string *&fileNames) {
    DIR *fontsDir = opendir("./fonts");

    //skip the . and .. paths
    readdir(fontsDir);
    readdir(fontsDir);

    dirent *nextEntry = readdir(fontsDir);
    int totalFonts = 0;
    while(nextEntry != nullptr) {
        totalFonts++;
        nextEntry = readdir(fontsDir);
    }

    fileNames = new string[totalFonts];

    rewinddir(fontsDir);
    //skip the . and .. paths
    readdir(fontsDir);
    readdir(fontsDir);
    
    for(int i = 0; i < totalFonts; i++) {
        nextEntry = readdir(fontsDir);
        fileNames[i] = nextEntry->d_name;
    }

    return totalFonts;
}