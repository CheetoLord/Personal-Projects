
#include <string>
using namespace std;

#ifndef _NOTE_H_
#define _NOTE_H_

class Note {
public:
    Note();
    Note(int delay, char key);
    string asString() const;

    int myDelay;
    char myKey;
};


#endif