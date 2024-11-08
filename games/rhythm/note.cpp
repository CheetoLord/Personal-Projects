
#include "note.h"


Note::Note() {
    myDelay = 1;
    myKey = 'a';
}

Note::Note(int delay, char key) {
    myDelay = delay;
    myKey = key;
}

string Note::asString() const {
    string retString = "Note(key=\'";
    retString.push_back(myKey);
    return retString + "\', delay=" + to_string(myDelay) + ")";
}