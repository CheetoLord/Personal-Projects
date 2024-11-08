


#ifndef _MENU_H_
#define _MENU_H_

#include <iostream>
#include <fstream>
#include <cassert>

#include "screen.h"
#include <unistd.h>
#include <math.h>


class interface {
public:
    interface();

    void setupScreen();

    void changeMenu(int menuID);
    void tickMenu();

    void loadFontSelectMenu();
    void tickFontSelectMenu();
    void unloadFontSelectMenu();

    bool isOpen();

private:
    //id key:
    //0 = font select screen
    int myMenuID;

    int screenWidth;
    int screenHeight;
    screen *theScreen;


    //font select menu
    

    bool programClosed;
};



#endif