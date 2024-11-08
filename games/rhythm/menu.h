


#ifndef _MENU_H_
#define _MENU_H_

#include <iostream>
#include <fstream>
#include <cassert>
#include <list>
#include <vector>
#include "note.h"

#include "screen.h"
#include <unistd.h>
#include <math.h>


class interface {
public:
    interface();
    void makeButtons();
    void makeTexts();
    void makeRects();

    void setupScreen();

    void changeMenu(int menuID);
    void tickMenu();

    void loadMainMenu();
    void tickMainMenu();
    void unloadMainMenu();

    void loadSongSelectMenu();
    void tickSongSelectMenu();
    void unloadSongSelectMenu();

    void loadGame();
    void tickGame();
    void unloadGame();

    void loadResultsMenu();
    void tickResultsMenu();
    void unloadResultsMenu();

    void loadFailMenu();
    void tickFailMenu();
    void unloadFailMenu();

    void loadEndlessMode();
    void tickEndlessMode();
    void unloadEndlessMode();

    void loadEndlessResultsMenu();
    void tickEndlessResultsMenu();
    void unloadEndlessResultsMenu();

    void loadEditSelectMenu();
    void tickEditSelectMenu();
    void unloadEditSelectMenu();

    void loadEditor();
    void tickEditor();
    void unloadEditor();
    void editorHandleScroll();
    void updateEditorButton(int x, int y);

    bool isOpen();
    int getSleepTime();

private:
    //id key:
    //0 = main menu
    //2 = game
    //1 = song select screen
    //3 = results screen
    //4 = fail menu
    //5 = endless mode
    //6 = endless result screen
    //99 = editor select menu
    //100 = editor
    int myMenuID;

    int extraMissZoneBuffer;
    int middleZoneBuffer;
    int topBuffer;
    int sideBuffer;
    int screenWidth;
    int screenHeight;
    screen *theScreen;

    textBox titleText;
    textBox songSelectTitle;
    textBox startHint;
    textBox victoryText;
    textBox hitPercent;
    textBox songName;
    textBox noteLabels[8];
    textBox missedNotes;
    textBox extraStrokes;
    textBox inGameSongName;
    textBox scoreDisplay;
    textBox finalScoreDisplay;
    textBox failMessage;

    rectangle **noteDisplay;
    rectangle lifeMeterGood;
    rectangle lifeMeterOK;
    rectangle lifeMeterBad;
    rectangle lifeMeterIndicator;
    rectangle remainingSongBack;
    rectangle remainingSongBar;
    rectangle startHintBack;
    button editorScrollBarBack;
    rectangle editorScrollBar;


    button startButton;
    button openEditor;
    button quitButton;
    button porknbeans;
    button firenflames;
    button schoolhousetrouble;
    button axelf;
    button endless;
    button returnToMain;
    button retryButton;
    button **editorButtons;
    button editorSaveQuit;

    vector<bool> *editorNoteStorage;
    int editorScrollPos;

    list<Note> myNotes;
    int speed;
    int totalTicks;

    string chosenSong;
    string chosenSongFile;

    double sleepPerTick;
    int currDelay;
    int remainingTicks;

    bool **notePos;

    bool arePressed[8];
    int newPresses[8];
    int noteStates[8];

    bool hasStarted;
    int misses;
    int extraHits;
    int totalNotes;
    int gameScore;
    int remainingLife;

    float endlessNoteDensity;
    int notesHit;
    int elapsedTicks;

    bool gameIsRunning;
    bool programClosed;
};



#endif