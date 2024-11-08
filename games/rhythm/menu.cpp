
#include "menu.h"

//other game related stuff
bool checkStart(screen *inputMethod) {
    if(inputMethod->newMouseEvents() || inputMethod->newKeyEvents()) {
        while(inputMethod->newKeyEvents()) {inputMethod->getKeyEvent();} //clear keyboard inputs
        int thrw1, thrw2;
        while(inputMethod->newMouseEvents()) {inputMethod->getMouseEvent(thrw1, thrw2);} //clear mouse inputs
        return true;
    }
    return false;
}

int getPosFromKey(char key) {
    if(key < 97) {key += 97-65;}
    switch(key) {
        case 'a': return 0; break;
        case 's': return 1; break;
        case 'd': return 2; break;
        case 'f': return 3; break;
        case 'h': return 4; break;
        case 'j': return 5; break;
        case 'k': return 6; break;
        case 'l': return 7; break;
    }
    return -1;
}

int shiftNotes(bool **notes) {
    bool *emptyRow = new bool[8];
    for(int i = 0; i < 8; i++) {
        emptyRow[i] = false;
    }

    int misses = 0;
    for(int i = 0; i < 8; i++) {
        if(notes[7][i]) {misses++;}
    }

    for(int i = 7; i > 0; i--) {
        notes[i] = notes[i-1];
    }
    notes[0] = emptyRow;

    return misses;
}

int shiftNotesWithRandomness(bool **notes, float density) {
    bool *newRow = new bool[8];
    for(int i = 0; i < 8; i++) {
        float rand_float =  static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        newRow[i] = rand_float < density;
    }

    int misses = 0;
    for(int i = 0; i < 8; i++) {
        if(notes[7][i]) {misses++;}
    }

    for(int i = 7; i > 0; i--) {
        notes[i] = notes[i-1];
    }
    notes[0] = newRow;

    return misses;
}

void getKeyPresses(screen *inputMethod, int *newPresses, bool* arePressed, int *noteStates) {
    while(inputMethod->newKeyEvents()) {
        char event = inputMethod->getKeyEvent();
        char key = event & 0b01111111;
        int keyIndex = getPosFromKey(key);

        if(keyIndex == -1) {continue;}

        bool unpress = event & 0b10000000;

        if(!unpress) {
            newPresses[keyIndex]++;
            arePressed[keyIndex] = true;
        } else {
            arePressed[keyIndex] = false;
            noteStates[keyIndex] = 0;
        }
    }
}

int checkNoteHits(bool **notePosses, int *keysPressed, int *noteStates, int &extraPresses) {
    int misses = 0;
    int hits = 0;
    for(int x = 0; x < 8; x++) {
        if (keysPressed[x] == 0) {continue;}

        if(notePosses[7][x]) {
            notePosses[7][x] = false;
            keysPressed[x]--;
            noteStates[x] = 1;
            hits++;
        }
        if(notePosses[6][x] && keysPressed[x] > 0) {
            notePosses[6][x] = false;
            keysPressed[x]--;
            noteStates[x] = 1;
            hits++;
        }
        
        if(keysPressed[x] > 0) {
            misses += keysPressed[x];
            noteStates[x] = 2;
        }
        keysPressed[x] = 1;
    }
    extraPresses = misses;
    return hits;
}

int getNextNotes(bool **notePos, list<Note> &notes, int *totalNotes) {
    if(notes.empty()) {
        return 100;
    }

    Note currNote;
    Note nextNote;

    do {
        currNote = notes.front();
        notes.pop_front();
        int pos = getPosFromKey(currNote.myKey);
        notePos[0][pos] = true;

        if(notes.empty()) {
            return 100;
        }
        nextNote = notes.front();
        (*totalNotes)++;
    } while(nextNote.myDelay == 0);

    return nextNote.myDelay;
}

int readNotesFromFile(list<Note> &notes, string fileName, int &speed) {
    notes.clear();

    ifstream songFile(fileName);
    assert(songFile.is_open());

    string speedLine;
    getline(songFile, speedLine);
    speed = stoi(speedLine.substr(6));

    string line;
    int cumulativeDelay = 0;
    int totalTicks = 0;
    while(getline(songFile, line)) {
        int linePos = 0;
        while(line.length() > linePos) {
            char key = line[linePos];
            linePos += 2;
            Note newNote = Note(cumulativeDelay, key);
            cumulativeDelay = 0;
            notes.push_back(newNote);
        }
        cumulativeDelay++;
        totalTicks++;
    }
    return totalTicks;
}












//----------------------------------interface----------------------------------

//constructor
interface::interface() {
    srand (static_cast <unsigned> (time(0))); //seed random number generator

    myMenuID = 0;

    setupScreen();

    makeButtons();
    makeRects();
    makeTexts();

    programClosed = false;
}

//setups
void interface::makeButtons() {

    startButton.redefine(screenWidth/2 - 100, 300, 200, 60, "PLAY");
    startButton.setColor(100, 255, 50);
    startButton.setTextColor(255, 255, 255);
    startButton.setFontSize(40);
    startButton.setStrokeWeight(6);
    startButton.setStrokeColor(0, 0, 0);
    startButton.hide();

    openEditor.redefine(screenWidth/2 - 100, 400, 200, 60, "OPEN\nEDITOR");
    openEditor.setColor(255, 50, 255);
    openEditor.setTextColor(255, 255, 255);
    openEditor.setFontSize(20);
    openEditor.setStrokeWeight(6);
    openEditor.setStrokeColor(0, 0, 0);
    openEditor.hide();

    quitButton.redefine(screenWidth/2 - 100, 500, 200, 60, "QUIT");
    quitButton.setColor(60, 60, 60);
    quitButton.setTextColor(230, 230, 230);
    quitButton.setFontSize(40);
    quitButton.setStrokeWeight(6);
    quitButton.setStrokeColor(0, 0, 0);
    quitButton.hide();

    porknbeans.redefine(50, 200, 300, 100, "Pork And\nBeans");
    porknbeans.setColor(50, 200, 50);
    porknbeans.setTextColor(255, 255, 255);
    porknbeans.setFontSize(25);
    porknbeans.setStrokeWeight(6);
    porknbeans.setStrokeColor(0, 0, 0);
    porknbeans.hide();

    firenflames.redefine(screenWidth - 350, 330, 300, 100, "Through\nthe Fire\nAnd Flames");
    firenflames.setColor(100, 0, 0);
    firenflames.setTextColor(255, 255, 255);
    firenflames.setFontSize(20);
    firenflames.setStrokeWeight(6);
    firenflames.setStrokeColor(0, 0, 0);
    firenflames.hide();

    schoolhousetrouble.redefine(50, 330, 300, 100, "Schoolhouse\nTrouble");
    schoolhousetrouble.setColor(255, 100, 0);
    schoolhousetrouble.setTextColor(255, 255, 255);
    schoolhousetrouble.setFontSize(20);
    schoolhousetrouble.setStrokeWeight(6);
    schoolhousetrouble.setStrokeColor(0, 0, 0);
    schoolhousetrouble.hide();

    axelf.redefine(screenWidth - 350, 200, 300, 100, "Axel F");
    axelf.setColor(220, 220, 0);
    axelf.setTextColor(255, 255, 255);
    axelf.setFontSize(30);
    axelf.setStrokeWeight(6);
    axelf.setStrokeColor(0, 0, 0);
    axelf.hide();

    endless.redefine(screenWidth/2 - 200, 460, 400, 60, "Endless Mode");
    endless.setColor(255, 0, 0);
    endless.setTextColor(255, 255, 255);
    endless.setFontSize(30);
    endless.setStrokeWeight(6);
    endless.setStrokeColor(0, 0, 0);
    endless.hide();

    returnToMain.redefine(screenWidth/2 - 100, 550, 200, 100, "MAIN\nMENU");
    returnToMain.setTextColor(255, 255, 255);
    returnToMain.setColor(0, 200, 0);
    returnToMain.setFontSize(25);
    returnToMain.setStrokeWeight(6);
    returnToMain.setStrokeColor(0, 0, 0);
    returnToMain.hide();

    retryButton.redefine(screenWidth/2 - 100, 400, 200, 100, "RETRY");
    retryButton.setTextColor(255, 255, 255);
    retryButton.setColor(255, 200, 0);
    retryButton.setFontSize(25);
    retryButton.setStrokeWeight(6);
    retryButton.setStrokeColor(0, 0, 0);
    retryButton.hide();

    int availNoteWidth = screenWidth - 2*sideBuffer - middleZoneBuffer;
    int availNoteHeight = screenHeight - topBuffer - extraMissZoneBuffer;
    int buffer = 2;
    int noteWidth = availNoteWidth/8 - buffer*2;
    int noteHeight = availNoteHeight/8 - buffer*2;
    editorButtons = new button*[8];

    int startX = buffer + sideBuffer + extraMissZoneBuffer;
    int startY = buffer + topBuffer;
    for(int i = 0; i < 8; i++) {
        editorButtons[i] = new button[8];
        for(int j = 0; j < 8; j++) {
            int x = i*noteWidth + i*2*buffer + startX;
            int y = j*noteHeight + j*2*buffer + startY;
            if(i >= 4) {
                x += middleZoneBuffer;
            }
            editorButtons[i][j].redefine(x, y, noteWidth, noteHeight);
            editorButtons[i][j].setColor(220, 220, 220);
            editorButtons[i][j].noStroke();
            editorButtons[i][j].hide();
        }
    }

    editorSaveQuit.redefine(screenWidth - 110, 10, 100, 70, "Save\nand\nQuit");
    editorSaveQuit.setTextColor(255, 255, 255);
    editorSaveQuit.setColor(30, 200, 30);
    editorSaveQuit.setFontSize(15);
    editorSaveQuit.setStrokeWeight(6);
    editorSaveQuit.setStrokeColor(0, 0, 0);
    editorSaveQuit.hide();

}

void interface::makeRects() {
    int availNoteWidth = screenWidth - 2*sideBuffer - middleZoneBuffer;
    int availNoteHeight = screenHeight - topBuffer - extraMissZoneBuffer;
    int buffer = 2;
    int noteWidth = availNoteWidth/8 - buffer*2;
    int noteHeight = availNoteHeight/8 - buffer*2;
    noteDisplay = new rectangle*[8];

    int startX = buffer + sideBuffer;
    int startY = buffer + topBuffer;
    for(int i = 0; i < 8; i++) {
        noteDisplay[i] = new rectangle[8];
        for(int j = 0; j < 8; j++) {
            int x = j*noteWidth + j*2*buffer + startX;
            int y = i*noteHeight + i*2*buffer + startY;
            if(i == 7) {
                y += extraMissZoneBuffer;
            }
            if(j >= 4) {
                x += middleZoneBuffer;
            }
            noteDisplay[i][j].redefine(x, y, noteWidth, noteHeight);
            noteDisplay[i][j].setColor(220, 220, 220);
            noteDisplay[i][j].hide();
        }
    }


    startHintBack.redefine(screenWidth/2 - 275, 180, 550, 150);
    startHintBack.setColor(0, 0, 0);
    startHintBack.hide();

    lifeMeterGood.redefine(10, 200, 30, 100);
    lifeMeterGood.setColor(0, 100, 0); //0 255 0
    lifeMeterGood.hide();

    lifeMeterOK.redefine(10, 300, 30, 100);
    lifeMeterOK.setColor(100, 100, 0); //220 220 0
    lifeMeterOK.hide();

    lifeMeterBad.redefine(10, 400, 30, 100);
    lifeMeterBad.setColor(100, 0, 0); //255 0 0
    lifeMeterBad.hide();

    lifeMeterIndicator.redefine(5, 350, 40, 4);
    lifeMeterIndicator.setColor(255, 255, 255);
    lifeMeterIndicator.hide();

    remainingSongBack.redefine(30, 80, 100, 15);
    remainingSongBack.setColor(50, 0, 100);
    remainingSongBack.hide();

    remainingSongBar.redefine(30, 80, 0, 15);
    remainingSongBar.setColor(100, 0, 200);
    remainingSongBar.hide();

    editorScrollBarBack.redefine(10, 100, 30, 400);
    editorScrollBarBack.setColor(130, 130, 130);
    editorScrollBarBack.noStroke();
    editorScrollBarBack.hide();

    editorScrollBar.redefine(10, 100, 30, 10);
    editorScrollBar.setColor(200, 200, 200);
    editorScrollBar.hide();

}

void interface::makeTexts() {

    int availNoteWidth = screenWidth - 2*sideBuffer - middleZoneBuffer;
    int availNoteHeight = screenHeight - topBuffer - extraMissZoneBuffer;
    int buffer = 2;
    int noteWidth = availNoteWidth/8 - buffer*2;
    int noteHeight = availNoteHeight/8 - buffer*2;

    int startX = buffer + sideBuffer + noteWidth/2;
    int startY = buffer + topBuffer + noteHeight/2;
    for(int i = 0; i < 8; i++) {
        int x = i*noteWidth + i*2*buffer + startX;
        int y = 6*noteHeight + 6*2*buffer + startY;
        if(i >= 4) {
            x += middleZoneBuffer;
        }

        noteLabels[i].redefine(x, y);
        noteLabels[i].setAlignment("center");
        noteLabels[i].setTextColor(0, 0, 0);
        noteLabels[i].setFontSize(20);
        noteLabels[i].setText(string("ASDFHJKL").substr(i, 1));
        noteLabels[i].hide();
    }

    titleText.redefine(screenWidth/2, 40, "RHYTHM\nGAME\nor smth idk");
    titleText.setAlignment("center");
    titleText.setFontSize(40);
    titleText.setTextColor(255, 255, 255);
    titleText.hide();

    songSelectTitle.redefine(screenWidth/2, 40, "SELECT\nA SONG");
    songSelectTitle.setAlignment("center");
    songSelectTitle.setFontSize(40);
    songSelectTitle.setTextColor(30);
    songSelectTitle.hide();

    startHint.redefine(screenWidth/2, 200, "Click or press\nany button\nto start");
    startHint.setAlignment("center");
    startHint.setFontSize(30);
    startHint.setTextColor(255, 255, 255);
    startHint.hide();

    victoryText.redefine(screenWidth/2, 50, "SONG CLEARED!");
    victoryText.setAlignment("center");
    victoryText.setTextColor(255, 255, 255);
    victoryText.setFontSize(40);
    victoryText.hide();

    hitPercent.redefine(screenWidth/2, 130, "%");
    hitPercent.setAlignment("center");
    hitPercent.setTextColor(255, 255, 255);
    hitPercent.setFontSize(50);
    hitPercent.hide();

    songName.redefine(screenWidth/2, 200, "song name");
    songName.setAlignment("center");
    songName.setTextColor(255, 255, 255);
    songName.setFontSize(20);
    songName.hide();

    missedNotes.redefine(100, 310, "You missed: _ notes");
    missedNotes.setTextColor(200, 200, 200);
    missedNotes.setFontSize(20);
    missedNotes.hide();

    extraStrokes.redefine(100, 350, "You hit: _ extra keys");
    extraStrokes.setTextColor(200, 200, 200);
    extraStrokes.setFontSize(20);
    extraStrokes.hide();

    inGameSongName.redefine(screenWidth/2, 20, "Song name here");
    inGameSongName.setAlignment("center");
    inGameSongName.setTextColor(255, 255, 255);
    inGameSongName.setFontSize(30);
    inGameSongName.hide();

    scoreDisplay.redefine(screenWidth/2, 70, "score here");
    scoreDisplay.setAlignment("center");
    scoreDisplay.setTextColor(220, 220, 220);
    scoreDisplay.setFontSize(20);
    scoreDisplay.hide();

    finalScoreDisplay.redefine(screenWidth/2, 250, "score here");
    finalScoreDisplay.setAlignment("center");
    finalScoreDisplay.setTextColor(220, 220, 220);
    finalScoreDisplay.setFontSize(20);
    finalScoreDisplay.hide();

    failMessage.redefine(screenWidth/2, 100, "You Failed!");
    failMessage.setAlignment("center");
    failMessage.setTextColor(255, 255, 255);
    failMessage.setFontSize(50);
    failMessage.hide();

}


//screen setup
void interface::setupScreen() {
    extraMissZoneBuffer = 10;
    middleZoneBuffer = 50;
    topBuffer = 100;
    sideBuffer = 150;

    screenWidth = 400 + middleZoneBuffer + 2*sideBuffer;
    screenHeight = 600 + extraMissZoneBuffer + topBuffer;
    theScreen = makeScreen("Rhythm Game", screenWidth, screenHeight);
    setBackground(50, 50, 50);
}



// loading/ticking/unloading menus
void interface::changeMenu(int menuID) {
    switch(myMenuID) {
        case 0: unloadMainMenu(); break;
        case 1: unloadSongSelectMenu(); break;
        case 2: unloadGame(); break;
        case 3: unloadResultsMenu(); break;
        case 4: unloadFailMenu(); break;
        case 5: unloadEndlessMode(); break;
        case 6: unloadEndlessResultsMenu(); break;
        case 99: unloadEditSelectMenu(); break;
        case 100: unloadEditor(); break;
    }

    myMenuID = menuID;

    switch(myMenuID) {
        case 0: loadMainMenu(); break;
        case 1: loadSongSelectMenu(); break;
        case 2: loadGame(); break;
        case 3: loadResultsMenu(); break;
        case 4: loadFailMenu(); break;
        case 5: loadEndlessMode(); break;
        case 6: loadEndlessResultsMenu(); break;
        case 99: loadEditSelectMenu(); break;
        case 100: loadEditor(); break;
    }
}

void interface::tickMenu() {
    switch(myMenuID) {
        case 0: tickMainMenu(); break;
        case 1: tickSongSelectMenu(); break;
        case 2: tickGame(); break;
        case 3: tickResultsMenu(); break;
        case 4: tickFailMenu(); break;
        case 5: tickEndlessMode(); break;
        case 6: tickEndlessResultsMenu(); break;
        case 99: tickEditSelectMenu(); break;
        case 100: tickEditor(); break;
    }

    programClosed = !theScreen->isOpen() || programClosed;
}

// main menu
void interface::loadMainMenu() {
    setBackground(0, 30, 200);
    titleText.show();
    startButton.show();
    openEditor.show();
    quitButton.show();
}

void interface::tickMainMenu() {
    if(startButton.getClickedState()) {
        changeMenu(1);
    } else if(openEditor.getClickedState()) {
        changeMenu(99);
    } else if(quitButton.getClickedState()) {
        programClosed = true;
    }
}

void interface::unloadMainMenu() {
    titleText.hide();
    startButton.hide();
    openEditor.hide();
    quitButton.hide();
}

// song select menu
void interface::loadSongSelectMenu() {
    setBackground(50, 150, 200);
    songSelectTitle.show();
    firenflames.show();
    porknbeans.show();
    axelf.show();
    schoolhousetrouble.show();
    endless.show();
    returnToMain.show();
}

void interface::tickSongSelectMenu() {
    if(firenflames.getClickedState()) {
        chosenSong = "Through the Fire And Flames";
        chosenSongFile = "firenflames.txt";
        changeMenu(2);
    } else if(porknbeans.getClickedState()) {
        chosenSong = "Pork And Beans";
        chosenSongFile = "porknbeans.txt";
        changeMenu(2);
    } else if(axelf.getClickedState()) {
        chosenSong = "Axel F";
        chosenSongFile = "axelf.txt";
        changeMenu(2);
    } else if(schoolhousetrouble.getClickedState()) {
        chosenSong = "Schoolhouse Trouble";
        chosenSongFile = "schoolhousetrouble.txt";
        changeMenu(2);
    } else if(endless.getClickedState()) {
        chosenSong = "Endless Mode";
        changeMenu(5);
    } else if(returnToMain.getClickedState()) {
        changeMenu(0);
    }
}

void interface::unloadSongSelectMenu() {
    songSelectTitle.hide();
    firenflames.hide();
    porknbeans.hide();
    axelf.hide();
    schoolhousetrouble.hide();
    endless.hide();
    returnToMain.hide();
}

// game menu
void interface::loadGame() {
    totalTicks = readNotesFromFile(myNotes, "./songs/" + chosenSongFile, speed);

    sleepPerTick = 60.0/speed * 1000;
    currDelay = 0;
    remainingTicks = totalTicks+20;

    notePos = new bool*[8];
    for(int i = 0; i < 8; i++) {
        notePos[i] = new bool[8];
        for(int j = 0; j < 8; j++) {
            notePos[i][j] = false;
        }
    }

    for(int i = 0; i < 8; i++) {
        newPresses[i] = 0;
        arePressed[i] = false;
        noteStates[i] = 0;
    }

    hasStarted = false;
    misses = 0;
    extraHits = 0;
    totalNotes = 0;

    checkStart(theScreen); //clear any keyboard/mouse events

    setBackground(20, 20, 150);
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            noteDisplay[i][j].show();
        }
        noteLabels[i].show();
    }

    hasStarted = false;
    startHint.show();
    startHintBack.show();

    lifeMeterGood.setColor(0, 100, 0);
    lifeMeterGood.show();

    lifeMeterOK.setColor(220, 220, 0);
    lifeMeterOK.show();

    lifeMeterBad.setColor(100, 0, 0);
    lifeMeterBad.show();

    lifeMeterIndicator.show();
    lifeMeterIndicator.setY(350);

    remainingSongBack.show();

    remainingSongBar.show();
    remainingSongBar.setWidth(0);

    inGameSongName.show();
    inGameSongName.setText(chosenSong);
    inGameSongName.setFontSize(30);
    int bestFontSize = 30;
    theScreen->setFontSize(bestFontSize);
    while(theScreen->getTextWidth(chosenSong) >= theScreen->getWidth()) {
        bestFontSize -= 3;
        theScreen->setFontSize(bestFontSize);
    }
    inGameSongName.setFontSize(bestFontSize);

    scoreDisplay.show();
    scoreDisplay.setText("0");

    gameScore = 0;
    remainingLife = 50;

}

void interface::tickGame() {

    if(!hasStarted) {
        hasStarted = checkStart(theScreen);
        return;
    } else {
        startHint.hide();
        startHintBack.hide();
    }


    if(remainingTicks <= 0) {
        changeMenu(3);
        return;
    }


    getKeyPresses(theScreen, newPresses, arePressed, noteStates);

    int extraStrokes;
    int hits = checkNoteHits(notePos, newPresses, noteStates, extraStrokes);
    extraHits += extraStrokes;
    gameScore -= 10*extraStrokes;
    gameScore += 50*hits;

    remainingLife -= extraStrokes*2;
    remainingLife += hits;

    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            if(notePos[i][j] && i == 7) { //note is at very bottom (has been missed)
                noteDisplay[i][j].setColor(100, 0, 150);
            } else if(notePos[i][j]) { //note at this position
                noteDisplay[i][j].setColor(0, 0, 255);
            } else if(i == 6 && noteStates[j] == 1 && (arePressed[j] || newPresses[j])) { //note was hit and key is still held
                noteDisplay[i][j].setColor(0, 255, 0);
            } else if(i == 6 && noteStates[j] == 2 && (arePressed[j] || newPresses[j])) { //note was missed and key is still held
                noteDisplay[i][j].setColor(255, 0, 0);
            } else if((totalTicks - remainingTicks + 20) % 8 == i) { //this is part of the tempo bar
                noteDisplay[i][j].setColor(200, 200, 200);
            } else { //nothing
                noteDisplay[i][j].setColor(220, 220, 220);
            }
        }
    }

    for(int i = 0; i < 8; i++) {
        newPresses[i] = 0;
    }

    int newMisses = shiftNotes(notePos);
    misses += newMisses;

    gameScore -= newMisses*10;
    scoreDisplay.setText(to_string(gameScore));

    remainingLife -= newMisses*4;
    if(remainingLife > 100) {
        remainingLife = 100;
    } else if (remainingLife <= 0) {
        changeMenu(4);
    }

    lifeMeterIndicator.setY(500 - (remainingLife*3));
    if(remainingLife < 34) {
        lifeMeterGood.setColor(0, 100, 0);
        lifeMeterOK.setColor(100, 100, 0);
        lifeMeterBad.setColor(255, 0, 0);
    } else if(remainingLife < 67) {
        lifeMeterGood.setColor(0, 100, 0);
        lifeMeterOK.setColor(220, 220, 0);
        lifeMeterBad.setColor(100, 0, 0);
    } else {
        lifeMeterGood.setColor(0, 255, 0);
        lifeMeterOK.setColor(100, 100, 0);
        lifeMeterBad.setColor(100, 0, 0);
    }

    int completedTicksPercent = 100 - (((remainingTicks)*100)/totalTicks);
    if(completedTicksPercent > 100) {
        completedTicksPercent = 100;
    }
    remainingSongBar.setWidth(completedTicksPercent);

    if(currDelay <= 0) {
        currDelay = getNextNotes(notePos, myNotes, &totalNotes);
    }
    currDelay--;
    remainingTicks--;
}

void interface::unloadGame() {
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            noteDisplay[i][j].hide();
        }
        noteLabels[i].hide();
    }

    startHint.hide();
    startHintBack.hide();
    lifeMeterGood.hide();
    lifeMeterOK.hide();
    lifeMeterBad.hide();
    lifeMeterIndicator.hide();
    remainingSongBack.hide();
    remainingSongBar.hide();
    inGameSongName.hide();
    scoreDisplay.hide();
}

// results menu
void interface::loadResultsMenu() {
    setBackground(20, 120, 50);
    victoryText.setText("SONG CLEARED!");
    victoryText.show();
    int percentHit = (int)floor((float)(totalNotes - misses)/totalNotes*100);
    hitPercent.setText(to_string(percentHit) + "%");
    hitPercent.show();
    songName.setText(chosenSong);
    songName.show();
    missedNotes.setText("You missed: " + to_string(misses) + " notes");
    missedNotes.show();
    extraStrokes.setText("You hit: " + to_string(extraHits) + " extra keys");
    extraStrokes.show();
    returnToMain.show();
    retryButton.show();
    finalScoreDisplay.setText(to_string(gameScore));
    finalScoreDisplay.show();
}

void interface::tickResultsMenu() {
    if(returnToMain.getClickedState()) {
        changeMenu(0);
    } else if(retryButton.getClickedState()) {
        changeMenu(2);
    }
}

void interface::unloadResultsMenu() {
    victoryText.hide();
    hitPercent.hide();
    songName.hide();
    missedNotes.hide();
    extraStrokes.hide();
    retryButton.hide();
    returnToMain.hide();
    finalScoreDisplay.hide();
}

//fail menu
void interface::loadFailMenu() {
    setBackground(200, 20, 20);
    retryButton.show();
    failMessage.show();
    returnToMain.show();
}

void interface::tickFailMenu() {
    if(retryButton.getClickedState()) {
        changeMenu(2);
    } else if(returnToMain.getClickedState()) {
        changeMenu(0);
    }
}

void interface::unloadFailMenu() {
    retryButton.hide();
    failMessage.hide();
    returnToMain.hide();
}

// endless mode
void interface::loadEndlessMode() {

    speed = 250;
    endlessNoteDensity = 0.03;
    elapsedTicks = 0;

    sleepPerTick = 60.0/speed * 1000;

    notePos = new bool*[8];
    for(int i = 0; i < 8; i++) {
        notePos[i] = new bool[8];
        for(int j = 0; j < 8; j++) {
            notePos[i][j] = false;
        }
    }

    for(int i = 0; i < 8; i++) {
        newPresses[i] = 0;
        arePressed[i] = false;
        noteStates[i] = 0;
    }

    hasStarted = false;
    misses = 0;
    extraHits = 0;
    totalNotes = 0;
    notesHit = 0;

    checkStart(theScreen); //clear any keyboard/mouse events

    setBackground(120, 20, 50);
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            noteDisplay[i][j].show();
        }
    }

    hasStarted = false;
    startHint.show();
    startHintBack.show();

    lifeMeterGood.setColor(0, 100, 0);
    lifeMeterGood.show();

    lifeMeterOK.setColor(220, 220, 0);
    lifeMeterOK.show();

    lifeMeterBad.setColor(100, 0, 0);
    lifeMeterBad.show();

    lifeMeterIndicator.show();
    lifeMeterIndicator.setY(350);

    inGameSongName.show();
    inGameSongName.setText(chosenSong);
    inGameSongName.setFontSize(30);

    scoreDisplay.show();
    scoreDisplay.setText("0");

    gameScore = 0;
    remainingLife = 50;

}

void interface::tickEndlessMode() {

    if(!hasStarted) {
        hasStarted = checkStart(theScreen);
        return;
    } else {
        startHint.hide();
        startHintBack.hide();
    }

    getKeyPresses(theScreen, newPresses, arePressed, noteStates);

    int extraStrokes;
    int hits = checkNoteHits(notePos, newPresses, noteStates, extraStrokes);
    notesHit += hits;
    extraHits += extraStrokes;
    gameScore -= 10*extraStrokes;
    gameScore += 50*hits;

    remainingLife -= extraStrokes*2;
    remainingLife += hits;

    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            if(notePos[i][j] && i == 7) { //note is at very bottom (has been missed)
                noteDisplay[i][j].setColor(100, 0, 150);
            } else if(notePos[i][j]) { //note at this position
                noteDisplay[i][j].setColor(0, 0, 255);
            } else if(i == 6 && noteStates[j] == 1 && (arePressed[j] || newPresses[j])) { //note was hit and key is still held
                noteDisplay[i][j].setColor(0, 255, 0);
            } else if(i == 6 && noteStates[j] == 2 && (arePressed[j] || newPresses[j])) { //note was missed and key is still held
                noteDisplay[i][j].setColor(255, 0, 0);
            } else if((elapsedTicks) % 8 == i) { //this is part of the tempo bar
                noteDisplay[i][j].setColor(200, 200, 200);
            } else { //nothing
                noteDisplay[i][j].setColor(220, 220, 220);
            }
        }
    }

    for(int i = 0; i < 8; i++) {
        newPresses[i] = 0;
    }

    int newMisses = shiftNotesWithRandomness(notePos, endlessNoteDensity);
    misses += newMisses;

    gameScore -= newMisses*10;
    scoreDisplay.setText(to_string(gameScore));

    remainingLife -= newMisses*4;
    if(remainingLife > 100) {
        remainingLife = 100;
    } else if (remainingLife <= 0) {
        totalNotes = notesHit + misses;
        changeMenu(6);
    }

    lifeMeterIndicator.setY(500 - (remainingLife*3));
    if(remainingLife < 34) {
        lifeMeterGood.setColor(0, 100, 0);
        lifeMeterOK.setColor(100, 100, 0);
        lifeMeterBad.setColor(255, 0, 0);
    } else if(remainingLife < 67) {
        lifeMeterGood.setColor(0, 100, 0);
        lifeMeterOK.setColor(220, 220, 0);
        lifeMeterBad.setColor(100, 0, 0);
    } else {
        lifeMeterGood.setColor(0, 255, 0);
        lifeMeterOK.setColor(100, 100, 0);
        lifeMeterBad.setColor(100, 0, 0);
    }

    if(elapsedTicks%3 == 0) {
        speed++;
        sleepPerTick = 60.0/speed * 1000;
    }

    endlessNoteDensity += 0.0002;

    elapsedTicks++;
}

void interface::unloadEndlessMode() {
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            noteDisplay[i][j].hide();
        }
    }

    startHint.hide();
    startHintBack.hide();
    lifeMeterGood.hide();
    lifeMeterOK.hide();
    lifeMeterBad.hide();
    lifeMeterIndicator.hide();
    inGameSongName.hide();
    scoreDisplay.hide();
}

// endless mode results menu
void interface::loadEndlessResultsMenu() {
    setBackground(20, 120, 50);
    
    victoryText.setText("ENDLESS MODE RESULTS");
    victoryText.setFontSize(35);
    victoryText.show();

    if(totalNotes == 0) {totalNotes = 1;}
    int percentHit = (int)floor(100*(float)(notesHit)/totalNotes);
    hitPercent.setText(to_string(percentHit) + "%");
    hitPercent.show();

    songName.setText(chosenSong);
    songName.show();

    missedNotes.setText("You missed: " + to_string(misses) + " notes");
    missedNotes.show();

    extraStrokes.setText("You hit: " + to_string(extraHits) + " extra keys");
    extraStrokes.show();

    returnToMain.show();
    retryButton.show();

    finalScoreDisplay.setText(to_string(gameScore));
    finalScoreDisplay.show();
}

void interface::tickEndlessResultsMenu() {
    if(returnToMain.getClickedState()) {
        changeMenu(0);
    } else if(retryButton.getClickedState()) {
        changeMenu(5);
    }
}

void interface::unloadEndlessResultsMenu() {
    victoryText.hide();
    hitPercent.hide();
    songName.hide();
    missedNotes.hide();
    extraStrokes.hide();
    retryButton.hide();
    returnToMain.hide();
    finalScoreDisplay.hide();
}

//editor song select menu
void interface::loadEditSelectMenu() {
    setBackground(150, 50, 200);
    songSelectTitle.show();
    firenflames.show();
    porknbeans.show();
    axelf.show();
    schoolhousetrouble.show();
    returnToMain.show();
}

void interface::tickEditSelectMenu() {
    if(firenflames.getClickedState()) {
        chosenSong = "Through the Fire And Flames";
        chosenSongFile = "firenflames.txt";
        changeMenu(100);
    } else if(porknbeans.getClickedState()) {
        chosenSong = "Pork And Beans";
        chosenSongFile = "porknbeans.txt";
        changeMenu(100);
    } else if(axelf.getClickedState()) {
        chosenSong = "Axel F";
        chosenSongFile = "axelf.txt";
        changeMenu(100);
    } else if(schoolhousetrouble.getClickedState()) {
        chosenSong = "Schoolhouse Trouble";
        chosenSongFile = "schoolhousetrouble.txt";
        changeMenu(100);
    } else if(returnToMain.getClickedState()) {
        changeMenu(0);
    }
}

void interface::unloadEditSelectMenu() {
    songSelectTitle.hide();
    firenflames.hide();
    porknbeans.hide();
    axelf.hide();
    schoolhousetrouble.hide();
    returnToMain.hide();
}

//editor
void interface::loadEditor() {
    editorScrollBar.show();
    editorScrollBarBack.show();
    editorSaveQuit.show();

    editorNoteStorage = new vector<bool>[8];
    for(int i = 0; i < 8; i++) {
        editorNoteStorage[i] = vector<bool>();
    }

    ifstream songFile("songs/" + chosenSongFile);

    string speedLine;
    getline(songFile, speedLine);
    speed = stoi(speedLine.substr(6));

    string line;
    while(getline(songFile, line)) {
        for(int i = 0; i < 8; i++) {
            editorNoteStorage[i].push_back(false);
        }

        int linePos = 0;
        while(line.length() > linePos) {
            char key = line[linePos];
            int column = getPosFromKey(key);
            editorNoteStorage[column].back() = true;
            linePos += 2;
        }
    }

    editorScrollPos = 0;

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            updateEditorButton(i,j);
            editorButtons[i][j].show();
        }
    }
}

void interface::editorHandleScroll() {
    while(theScreen->newScrollEvents()) {
        bool scrollUp = theScreen->getScrollEvent();
        if(!scrollUp) {
            editorScrollPos += 1;
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    updateEditorButton(i, j);
                }
            }
        } else if(editorScrollPos > 0) { //scrollUp == true
            editorScrollPos -= 1;
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    updateEditorButton(i, j);
                }
            }
        }
    }

    if(editorScrollBarBack.getClickedState()) {
        int reqProgress = 100 - ((theScreen->getMouseY() - editorScrollBarBack.getY()) / 4);
        int reqScrollPos = reqProgress * editorNoteStorage[0].size() / 100;
        editorScrollPos = reqScrollPos;
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                updateEditorButton(i, j);
            }
        }
    }
}

void interface::updateEditorButton(int x, int y) {
    bool inRange = (7-y) + editorScrollPos < editorNoteStorage[x].size();
    if(inRange && editorNoteStorage[x][(7-y) + editorScrollPos]) {
        editorButtons[x][y].setColor(0, 0, 255);
    } else if ((editorScrollPos+6) % 8 == y) {
        editorButtons[x][y].setColor(220, 220, 220);
    } else {
        editorButtons[x][y].setColor(255, 255, 255);
    }
}

void interface::tickEditor() {
    editorHandleScroll();

    //note toggles
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            if(editorButtons[i][j].getClickedState()) {
                int y = (7-j) + editorScrollPos;
                //extend storage if necessary
                while(y >= editorNoteStorage[i].size()) {
                    for(int k = 0; k < 8; k++) {
                        editorNoteStorage[k].push_back(false);
                    }
                }
                //toggle note state
                editorNoteStorage[i][y] = !editorNoteStorage[i][y];
                updateEditorButton(i, j);
            }
        }
    }

    //update scroll bar
    int scrollBarSize = (8 * editorScrollBarBack.getHeight()) / editorNoteStorage[0].size();
    editorScrollBar.setHeight(scrollBarSize);

    int scrollBarPos = (editorScrollPos * editorScrollBarBack.getHeight()) / editorNoteStorage[0].size();
    if(scrollBarPos > editorScrollBarBack.getHeight() - scrollBarSize) {
        scrollBarPos = editorScrollBarBack.getHeight() - scrollBarSize;
    }
    int scrollBarY = editorScrollBarBack.getHeight() + editorScrollBarBack.getY() - scrollBarPos - scrollBarSize;
    editorScrollBar.setY(scrollBarY);

    //check save+quit button
    if(editorSaveQuit.getClickedState()) {
        changeMenu(0);
    }
}

void interface::unloadEditor() {
    editorScrollBar.hide();
    editorScrollBarBack.hide();
    editorSaveQuit.hide();
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            editorButtons[i][j].hide();
        }
    }

    ofstream songFile("songs/" + chosenSongFile);
    assert(songFile.is_open());

    songFile << "speed " << speed << endl;

    char notes[8] = {'a','s','d','f','h','j','k','l'};
    for (int i = 0; i < editorNoteStorage[0].size(); i++) {
        for (int j = 0; j < 8; j++) {
            if(editorNoteStorage[j][i]) {
                songFile << notes[j] << " ";
            }
        }
        songFile << endl;
    }
}

bool interface::isOpen() {
    return !programClosed;
}

int interface::getSleepTime() {
    if(myMenuID == 2 || myMenuID == 5) {
        return sleepPerTick;
    } else {
        return 20;
    }
}