
#include "screen.h"
#include "adventure.h"
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <functional>

using namespace std;



string limitLines(string text, int maxLines) {
    vector<string> lines;

    int nextNewline = text.find("\n");
    while(nextNewline != string::npos) {
        lines.push_back(text.substr(0, nextNewline));
        text = text.substr(nextNewline+1);
        nextNewline = text.find("\n");
    }
    lines.push_back(text);

    while(lines.size() > maxLines) {
        lines.erase(lines.begin());
    }

    string remText = lines[0];
    for(int i = 1; i < lines.size(); i++) {
        remText += "\n" + lines[i];
    }
    return remText;
}



void unmakeChoiceButtons(button *buttons, int buttonCount, screen *theScreen) {
    if(!buttons) {return;}
    for(int i = 0; i < buttonCount; i++) {
        theScreen->remElement((screenElement*) &buttons[i]);
    }
    delete [] buttons;
}

//each pair of numbers represents 1 position
//first number is horizontal. positions are 0 = left, 2 = center, 4 = right, 1 and 3 are halfway in between 0 and 2, and 2 and 4 respecively
//second number is vertical, 0 = top, 1 = middle, 2 = bottom.
int *getButtonPositions(int buttonCount) {
    switch(buttonCount) {
        case 0: return nullptr;
        case 1: return new int[2] {2, 1};                                                   //1 in center
        case 2: return new int[4] {1, 1, 3, 1};                                             //2 in middle 
        case 3: return new int[6] {0, 1, 2, 1, 4, 1};                                       //3 in middle
        case 4: return new int[8] {1, 1, 3, 1, 1, 2, 3, 2};                                 //2x2 square
        case 5: return new int[10] {1, 1, 3, 1, 0, 2, 2, 2, 4, 2};                          //2 in middle row, 3 in bottom row
        case 6: return new int[12] {1, 0, 3, 0, 1, 1, 3, 1, 1, 2, 3, 2};                    //2 columns of buttons
        case 7: return new int[14] {1, 0, 3, 0, 0, 1, 2, 1, 4, 1, 1, 2, 3, 2};              //2 in top and botom rows, 3 in middle row
        case 8: return new int[16] {1, 0, 3, 0, 0, 1, 2, 1, 4, 1, 0, 2, 2, 2, 4, 2};        //2 in top row, 3 in other rows
        case 9: return new int[18] {0, 0, 2, 0, 4, 0, 0, 1, 2, 1, 4, 1, 0, 2, 2, 2, 4, 2};  //3x3 square
    }

    throw runtime_error("Button count (" + to_string(buttonCount) + ") is out of range (0-9)");
    return nullptr;
}

button *makeChoiceButtons(int buttonCount, screen *theScreen) {
    button *buttons = new button[buttonCount];
    int *positionList = getButtonPositions(buttonCount);

    int buttonWidth = theScreen->getWidth()/4;
    int buttonHeight = theScreen->getHeight()/10;

    int screenMiddle = theScreen->getWidth()/2;
    int screenBottom = theScreen->getHeight();

    for(int i = 0; i < buttonCount; i++) {
        int x = screenMiddle + ((positionList[i*2] - 3)*buttonWidth/2);
        int y = screenBottom - ((3 - positionList[i*2 + 1]) * buttonHeight) - buttonHeight/2;
        buttons[i].redefine(x, y, buttonWidth, buttonHeight, "Choice goes here");
        buttons[i].setFontSize(20);
        buttons[i].setColor(0, 0, 0);
        buttons[i].setTextColor(255, 255, 255);
        buttons[i].setStrokeColor(255, 255, 255);
        buttons[i].setStrokeWeight(4);
    }

    return buttons;
}






string chooseAdventure(screen *theScreen) {
    textBox choicePrompt(theScreen->getWidth()/2, 20, "Choose An Adventure To Take");
    choicePrompt.setTextColor(255, 255, 255);
    choicePrompt.setFontSize(50);
    choicePrompt.setAlignment("center");
    
    button choiceSpicy(theScreen->getWidth()/2 - 250, theScreen->getHeight()/2 - 50, 500, 100, "Spicy Adventure");
    choiceSpicy.setColor(0, 0, 0);
    choiceSpicy.setStrokeColor(255, 255, 255);
    choiceSpicy.setTextColor(255, 255, 255);
    choiceSpicy.setFontSize(30);

    string choice = "";

    while(choice == "" && theScreen->isOpen()) {
        waitForFrame(100);
        if(choiceSpicy.getClickedState()) {
            choice = "Spicy.txt";
        }
    }

    theScreen->remElement((screenElement*) &choicePrompt);
    theScreen->remElement((screenElement*) &choiceSpicy);

    return choice;
}





int main(int argc, char *argv[]) {
    bool debug = false;
    bool noCrash = false;
    for(int i = 0; i < argc; i++) {
        if(string(argv[i]) == "debug") {
            debug = true;
        } else if(string(argv[i]) == "nocrash") {
            noCrash = true;
        }
    }

    if(debug) {
        string adventureTitle = "Spicy.txt";
        if(argc > 2) {
            adventureTitle = argv[2];
        }
        adventure myAdventure(adventureTitle, true);
        return 0;
    }

    screen *theScreen = makeScreen("Choose Your Own Adventure", 1, 1);
    setBackground(0, 0, 0);
    maximizeScreen();

    //wait for screen maximize to go through
    while(theScreen->getWidth() == 1) {
        waitForFrame(100);
    }

    string chosenAdventure = chooseAdventure(theScreen);
    if(!theScreen->isOpen()) {
        return 0;
    }

    adventure myAdventure(chosenAdventure);
    event currEvent;

    int choiceCount = 0;
    button *choices = nullptr;

    textBox storyDisplay(theScreen->getWidth()/2, theScreen->getHeight()*11/20, "");
    storyDisplay.setTextColor(255, 255, 255);
    storyDisplay.setFontSize(20);
    storyDisplay.setAlignment("center");
    storyDisplay.setVertAlignment("bottom");

    rectangle dividingRectangle(0, theScreen->getHeight()*3/5, theScreen->getWidth(), 5);
    dividingRectangle.setColor(255, 255, 255);

    function<void(string, bool)> goToEvent = [&](string eventName, bool forced) mutable {
        currEvent = myAdventure.getEvent(eventName, noCrash);
        unmakeChoiceButtons(choices, choiceCount, theScreen);

        choiceCount = currEvent.choices.size();
        choices = makeChoiceButtons(choiceCount, theScreen);
        for(int i = 0; i < choiceCount; i++) {
            choices[i].setText(currEvent.choices[i].name);
        }

        if(!currEvent.clearsScreen || forced) {
            string seperator = forced ? "\n\n" : "\n\n--------------------------------\n\n";
            string newStory = storyDisplay.getText() + seperator + currEvent.story;
            trim(newStory);
            newStory = limitLines(newStory, 30);
            storyDisplay.setText(newStory);
        } else {
            string newStory = currEvent.story;
            trim(newStory);
            newStory = limitLines(newStory, 30);
            storyDisplay.setText(newStory);
        }

        if(currEvent.forceDest != "") {
            goToEvent(currEvent.forceDest, true);
        }
    };

    goToEvent("start", false);

    while(theScreen->isOpen()) {
        waitForFrame(100);

        for(int i = 0; i < choiceCount; i++) {
            if(choices[i].getClickedState()) {
                goToEvent(currEvent.choices[i].destEvent, false);
                break;
            }
        }

        if(choiceCount == 0) {
            waitForFrame(2000);
            break;
        }
    }

    return 0;
}