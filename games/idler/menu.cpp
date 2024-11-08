
#include "menu.h"




DATETIME::DATETIME(string dateTimeData) {
    dateTimeData = dateTimeData.substr(0, dateTimeData.length()-1); //remove newline

    dayOfWeek = dateTimeData.substr(0, dateTimeData.find(" "));
    dateTimeData = dateTimeData.substr(dateTimeData.find(" ") + 1);

    month = dateTimeData.substr(0, dateTimeData.find(" "));
    dateTimeData = dateTimeData.substr(dateTimeData.find(" ") + 1);

    dayOfMonth = dateTimeData.substr(0, dateTimeData.find(" "));
    if(dayOfMonth[0] == ' ') {dayOfMonth = dayOfMonth.substr(1);}

    dateTimeData = dateTimeData.substr(dateTimeData.find(" ") + 1);

    time = dateTimeData.substr(0, dateTimeData.find(" "));

    year = dateTimeData.substr(dateTimeData.find(" ") + 1);

    string timeCopy = time;

    hour = timeCopy.substr(0, timeCopy.find(":"));
    timeCopy = timeCopy.substr(timeCopy.find(":") + 1);

    minute = timeCopy.substr(0, timeCopy.find(":"));

    second = timeCopy.substr(timeCopy.find(":")+1);
}

DATETIME getDateTime() {
    auto time = chrono::system_clock::now();
 
    time_t end_time = chrono::system_clock::to_time_t(time);
 
    return DATETIME(ctime(&end_time));
}




//-----------------------Interface-----------------------


//constructor
interface::interface() {
    myMenuID = 0;

    setupScreen();

    waitForFrame(150);

    makeMainMenuElements();
    makeGameSelectMenuElements();
    makeGameRunningNotifElements();

    programClosed = false;
}

//setups
void interface::makeMainMenuElements() {

    string mainMessage = "Hello! If you see this, I am currently not at this\n"
                         "computer (duh), but I will be back shortly. If you\n"
                         "would be so kind as to not mess with my things, that\n"
                         "would be greatly appreciated. However, if you must\n"
                         "insist upon tampering with my things while I am\n"
                         "gone, there is a textbox below that you can use to enter\n"
                         "messages that I will read when I return. Thanks, pbl4";
    mainText.redefine(screenWidth/2, 30, mainMessage);
    mainText.setAlignment("center");
    mainText.setTextColor(0, 0, 0);
    mainText.setFontSize(screenWidth/50);
    mainText.hide();

    messageBox.redefine(10, screenHeight*5/8, screenWidth - 20, screenHeight*3/8 - 10);
    messageBox.setHint("Enter your message here");
    messageBox.setHintColor(150, 150, 150);
    messageBox.setFontSize(screenWidth/70);
    messageBox.setColor(255, 255, 255);
    messageBox.setTextColor(0, 0, 0);
    messageBox.clearTextOnSubmit(false);
    messageBox.multiLine(true);
    messageBox.hide();

    submitButton.redefine(screenWidth/2 - 250, (screenHeight*5)/8 - 80, 500, 70, "Submit Entry");
    submitButton.setColor(200, 255, 200);
    submitButton.setFontSize(40);
    submitButton.setTextColor(0, 0, 0);
    submitButton.hide();

    savedAlert.redefine(screenWidth/2, (screenHeight*5)/8 - 120, "Text Submitted!");
    savedAlert.setTextColor(0, 0, 0);
    savedAlert.setFontSize(20);
    savedAlert.setAlignment("center");
    savedAlert.hide();

    offerViewGames.redefine(10, screenHeight*3/8, "Or if you like,\nyou can try some\nof the games I have made");
    offerViewGames.setAlignment("left");
    offerViewGames.setFontSize(30);
    offerViewGames.setTextColor(0, 0, 0);
    offerViewGames.hide();

    viewGames.redefine(10, screenHeight/2, 50, 50, "<-");
    viewGames.setTextColor(0, 0, 0);
    viewGames.setFontSize(20);
    viewGames.setColor(200, 200, 200);
    viewGames.hide();
}

void interface::makeGameSelectMenuElements() {
    backButton.redefine(screenWidth*3/8, screenHeight*3/4, screenWidth/4, screenHeight/8, "Back");
    backButton.setColor(120, 120, 120);
    backButton.setTextColor(0, 0, 0);
    backButton.setFontSize(20);
    backButton.hide();
    
    CYOA.redefine(screenWidth/4, screenHeight*7/16, screenWidth*3/16, screenHeight/8, "Choose Your\nOwn Adventure");
    CYOA.setColor(60, 200, 100);
    CYOA.setTextColor(0, 0, 0);
    CYOA.setFontSize(20);
    CYOA.hide();

    rhythmGame.redefine(screenWidth*9/16, screenHeight*7/16, screenWidth*3/16, screenHeight/8, "Rhythm Game");
    rhythmGame.setColor(200, 200, 50);
    rhythmGame.setTextColor(0, 0, 0);
    rhythmGame.setFontSize(30);
    rhythmGame.hide();
}

void interface::makeGameRunningNotifElements() {
    GRnotif.redefine(screenWidth/2, screenHeight/2, "An opened game is currently running.\nPlease close it to continue.");
    GRnotif.setTextColor(255, 255, 255);
    GRnotif.setAlignment("center");
    GRnotif.setFontSize(40);
    GRnotif.hide();
}


void interface::setupMessageLog() {

    DATETIME dateTime = getDateTime();

    string yearPath = "messageLogs/" + dateTime.year;
    string monthPath = yearPath + "/" + dateTime.month;
    string dayPath = monthPath + "/" + dateTime.dayOfMonth + "-" + dateTime.dayOfWeek;

    struct stat sb;
    mode_t mode = S_IRWXU | S_IWGRP | S_IWOTH;

    if(stat(yearPath.c_str(), &sb) != 0) {mkdir(yearPath.c_str(), mode);}
    if(stat(monthPath.c_str(), &sb) != 0) {mkdir(monthPath.c_str(), mode);}
    if(stat(dayPath.c_str(), &sb) != 0) {mkdir(dayPath.c_str(), mode);}

    mkdir(yearPath.c_str(), mode);

    messageLog = ofstream("messageLogs/" + dateTime.year + "/" + dateTime.month + "/" + dateTime.dayOfMonth +
                          "-" + dateTime.dayOfWeek + "/" + dateTime.hour + ":" + dateTime.minute);
    assert(messageLog.is_open());

}


void waitForDimUpdate(interface *toUpdate) {
    waitForFrame(100);
    toUpdate->updateDims();
}

void interface::updateDims() {
    screenWidth = theScreen->getWidth();
    screenHeight = theScreen->getHeight();
}

//screen setup
void interface::setupScreen() {
    theScreen = makeScreen("AFK script");
    theScreen->toggleMaximize();
    
    thread getNewDimsThread;
    getNewDimsThread = thread(waitForDimUpdate, this);
    getNewDimsThread.detach();

    setBackground(50, 50, 50);
}



// loading/ticking/unloading menus
void interface::changeMenu(int menuID) {
    switch(myMenuID) {
        case 0: unloadMainMenu(); break;
        case 1: unloadGameSelectMenu(); break;
        case 2: unloadGameRunningNotif(); break;
    }

    myMenuID = menuID;

    switch(myMenuID) {
        case 0: loadMainMenu(); break;
        case 1: loadGameSelectMenu(); break;
        case 2: loadGameRunningNotif(); break;
    }
}

void interface::tickMenu() {
    switch(myMenuID) {
        case 0: tickMainMenu(); break;
        case 1: tickGameSelectMenu(); break;
        case 2: tickGameRunningNotif(); break;
    }

    programClosed = !theScreen->isOpen() || programClosed;
}


// main menu
void interface::loadMainMenu() {
    setBackground(50, 200, 50);

    mainText.show();
    messageBox.show();
    submitButton.show();
    offerViewGames.show();
    viewGames.show();

    submitAnimTimer = 0;
}

void interface::tickMainMenu() {

    if(submitButton.getClickedState()) {
        DATETIME dt = getDateTime();
        string message = messageBox.getTypedText();
        messageBox.clearInput();
        if(!openedLog) {
            setupMessageLog();
            openedLog = true;
        }
        messageLog << "[" << dt.time << "]\n" << message << "\n\n";
        submitAnimTimer = 120;
    }

    if(viewGames.getClickedState()) {
        changeMenu(1);
    }

    if(submitAnimTimer > 0) {
        savedAlert.show();
        if(submitAnimTimer > 110) {
            int newRed = 50 - (120-submitAnimTimer)*5;
            int newGreen = 200 - (120-submitAnimTimer)*20;
            int newBlue = 50 - (120-submitAnimTimer)*5;
            savedAlert.setTextColor(newRed, newGreen, newBlue);
        } else if(submitAnimTimer < 50) {
            int newRed = 50-submitAnimTimer;
            int newGreen = (50-submitAnimTimer)*4;
            int newBlue = 50-submitAnimTimer;
            savedAlert.setTextColor(newRed, newGreen, newBlue);
        }
        submitAnimTimer--;
        if(submitAnimTimer <= 0) {
            savedAlert.hide();
        }
    }

}

void interface::unloadMainMenu() {
    mainText.hide();
    messageBox.hide();
    submitButton.hide();
    offerViewGames.hide();
    viewGames.hide();
}



//game select menu
void interface::loadGameSelectMenu() {
    setBackground(20, 100, 20);

    backButton.show();
    CYOA.show();
    rhythmGame.show();
}

void interface::tickGameSelectMenu() {
    if(backButton.getClickedState()) {
        changeMenu(0);
    } else if(CYOA.getClickedState()) {
        changeMenu(2);
        waitForFrame(100);
        system("~/Desktop/\"Choose Your Own Adventure\"");
        changeMenu(1);
    } else if(rhythmGame.getClickedState()) {
        changeMenu(2);
        waitForFrame(100);
        system("~/Desktop/\"Rhythm Game\"");
        changeMenu(1);
    }
}

void interface::unloadGameSelectMenu() {
    backButton.hide();
    CYOA.hide();
    rhythmGame.hide();
}


//game running notif
void interface::loadGameRunningNotif() {
    setBackground(100, 100, 100);
    GRnotif.show();
}

void interface::tickGameRunningNotif() {

}

void interface::unloadGameRunningNotif() {
    GRnotif.hide();
}



bool interface::isOpen() {
    return !programClosed;
}

int interface::getSleepTime() {
    return 20;
}