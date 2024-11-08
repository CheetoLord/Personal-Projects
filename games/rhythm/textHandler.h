
#include <cassert>
#include <iostream>
#include <fstream>
#include <dirent.h>

#ifndef _TEXT_HANDLER_H_
#define _TEXT_HANDLER_H_

using namespace std;



class character {
public:
    character(char c, bool **bits, int FD);
    bool **getBitmap();
    int getLeadWhitespace() {return leadingWhitespace;};
    int getEndWhitespace() {return endingWhitespace;};

private:
    char myChar;
    bool **bitmap;
    int fontDetail;
    int leadingWhitespace;
    int endingWhitespace;
    bool colIsEmpty(int col);
};




class textHandler {
public:
    textHandler(string fileName);
    bool **getChar(char c);
    int getCharLeadWhitespace(char c) {return myChars[(int)c]->getLeadWhitespace();};
    int getCharEndWhitespace(char c) {return myChars[(int)c]->getEndWhitespace();};
    bool hasChar(char c);

    int getFontDetail() {return fontDetail;};
    int getSpacewidth() {return spacewidth;};
    int getCharSpacing() {return charspacing;};
    string getFontName() {return fontName;};
    string getFontFileName() {return fontFileName;};

    void reloadFromFile();

private:
    character **myChars;
    int fontDetail;
    int spacewidth;
    int charspacing;
    string fontName;
    string fontFileName;

};




int getAvailFonts(string *&fileNames);



#endif
