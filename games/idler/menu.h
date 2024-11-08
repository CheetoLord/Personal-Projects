


#ifndef _MENU_H_
#define _MENU_H_

#include <iostream>
#include <fstream>
#include <cassert>
#include <sys/stat.h>

#include "screen.h"
#include <thread>
#include <chrono>
#include <ctime>


struct DATETIME {
    DATETIME(string dateTimeData);
    string year;
    string month;
    string dayOfMonth;
    string dayOfWeek;
    string time;
    string hour;
    string minute;
    string second;
};

DATETIME getDateTime();

class interface {
public:
    interface();
    void makeMainMenuElements();
    void makeGameSelectMenuElements();
    void makeGameRunningNotifElements();
    void setupMessageLog();

    void updateDims();
    void setupScreen();

    void changeMenu(int menuID);
    void tickMenu();

    void loadMainMenu();
    void tickMainMenu();
    void unloadMainMenu();

    void loadGameSelectMenu();
    void tickGameSelectMenu();
    void unloadGameSelectMenu();

    void loadGameRunningNotif();
    void tickGameRunningNotif();
    void unloadGameRunningNotif();

    bool isOpen();
    int getSleepTime();

private:
    //id key:
    //0 = main menu
    //1 = game select menu
    //2 = game running notif
    int myMenuID;

    bool openedLog;
    ofstream messageLog;

    screen *theScreen;
    int screenWidth;
    int screenHeight;

    //main menu
    textBox mainText;
    inputBox messageBox;
    button submitButton;
    textBox savedAlert;
    int submitAnimTimer;
    textBox offerViewGames;
    button viewGames;

    //game select menu
    // int selctionScreenNum;
    // button leftButton;
    // button rightButton;
    button backButton;
    button rhythmGame;
    button CYOA;

    //game running notif
    textBox GRnotif;

    bool programClosed;
};



#endif