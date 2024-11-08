
#include "menu.h"


//constructor
interface::interface() {
    myMenuID = 0;

    setupScreen();

    changeMenu(0);

}

//setups


//screen setup
void interface::setupScreen() {

    theScreen = makeScreen("X Rhythm");
    theScreen->toggleMaximize();
    screenWidth = theScreen->getWidth();
    screenHeight = theScreen->getHeight();
    setBackground(220, 220, 220);
}


// loading/ticking/unloading menus
void interface::changeMenu(int menuID) {
    switch(myMenuID) {
        case 0: unloadFontSelectMenu();
    }

    myMenuID = menuID;

    switch(myMenuID) {
        case 0: loadFontSelectMenu();
    }
}

void interface::tickMenu() {
    switch(myMenuID) {
        case 0: tickFontSelectMenu();
    }

    programClosed = !theScreen->isOpen();
}


// font select menu
void interface::loadFontSelectMenu() {
    
}

void interface::tickFontSelectMenu() {
    
}

void interface::unloadFontSelectMenu() {
    
}



bool interface::isOpen() {
    return !programClosed;
}