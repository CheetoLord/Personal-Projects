
#include "menu.h"


//constructor
interface::interface() {
    myMenuID = 0;
    defaultFont = "basic 10x10";

    readCharNames();

    setupScreen();
    
    makeFontSelectMenuElements();
    makeFontEditMenuElements();
    makeMetadataEditMenuElements();
    makeCharacterSelectMenuElements();
    makeCharacterEditorElements();
    makeFontPreviewElements();

    changeMenu(0);

}

//setups
void interface::readCharNames() {
    characterNames = new string[maxChar - minChar + 1];

    ifstream charNamesFile("characterNames.txt");
    assert(charNamesFile.is_open());

    string line;
    while(getline(charNamesFile, line)) {
        char theChar = line[0];
        int listPos = (int)theChar - minChar;
        characterNames[listPos] = line.substr(2);
    }
}

void interface::makeFontSelectMenuElements() {

    string *fontNames = theScreen->getListOfFonts(fontCount);

    fontSelectButtons = new button[fontCount];
    normalFontNames = new textBox[fontCount];
    
    for(int i = 0; i < fontCount; i++) {
        string fontName = fontNames[i];
        switch(i%4) {
            case 0: fontSelectButtons[i].redefine(50, 200, 230, 100, fontName); break;
            case 1: fontSelectButtons[i].redefine(320, 200, 230, 100, fontName); break;
            case 2: fontSelectButtons[i].redefine(50, 400, 230, 100, fontName); break;
            case 3: fontSelectButtons[i].redefine(320, 400, 230, 100, fontName); break;
        }
        fontSelectButtons[i].setFont(fontName);
        fontSelectButtons[i].setFontSize(20);
        fontSelectButtons[i].setColor(255, 0, 255);
        fontSelectButtons[i].setTextColor(255, 255, 255);
        fontSelectButtons[i].hide();

        switch(i%4) {
            case 0: normalFontNames[i].redefine(165, 170, fontName); break;
            case 1: normalFontNames[i].redefine(435, 170, fontName); break;
            case 2: normalFontNames[i].redefine(165, 370, fontName); break;
            case 3: normalFontNames[i].redefine(435, 370, fontName); break;
        }
        normalFontNames[i].setFont(defaultFont);
        normalFontNames[i].setFontSize(10);
        normalFontNames[i].setAlignment("center");
        normalFontNames[i].setTextColor(255, 255, 255);
        normalFontNames[i].hide();
    }

    FSMleft.redefine(10, screenHeight/2 + 20, 30, 30, "<");
    FSMleft.setFont(defaultFont);
    FSMleft.setTextColor(0, 0, 0);
    FSMleft.setColor(220, 220, 220);
    FSMleft.setFontSize(20);
    FSMleft.hide();

    FSMright.redefine(screenWidth - 40, screenHeight/2 + 20, 30, 30, ">");
    FSMright.setFont(defaultFont);
    FSMright.setTextColor(0, 0, 0);
    FSMright.setColor(220, 220, 220);
    FSMright.setFontSize(20);
    FSMright.hide();

    FSMtitle.redefine(screenWidth/2, 20, "Select a\nfont to edit");
    FSMtitle.setFont(defaultFont);
    FSMtitle.setTextColor(255, 255, 255);
    FSMtitle.setFontSize(30);
    FSMtitle.setAlignment("center");
    FSMtitle.hide();

}

void interface::makeFontEditMenuElements() {

    editFontMetadata.redefine(screenWidth/2 - 200, 250, 400, 50, "Edit Metadata");
    editFontMetadata.setFont(defaultFont);
    editFontMetadata.setTextColor(0, 0, 0);
    editFontMetadata.setColor(0, 220, 220);
    editFontMetadata.setFontSize(25);
    editFontMetadata.hide();

    editCharacters.redefine(screenWidth/2 - 200, 310, 400, 50, "Edit Characters");
    editCharacters.setFont(defaultFont);
    editCharacters.setTextColor(0, 0, 0);
    editCharacters.setColor(255, 100, 0);
    editCharacters.setFontSize(25);
    editCharacters.hide();

    seePreview.redefine(screenWidth/2 - 200, 370, 400, 50, "See Preview");
    seePreview.setFont(defaultFont);
    seePreview.setTextColor(0, 0, 0);
    seePreview.setColor(255, 100, 255);
    seePreview.setFontSize(25);
    seePreview.hide();

    saveFont.redefine(screenWidth/2 - 100, 470, 200, 40, "Save Font");
    saveFont.setFont(defaultFont);
    saveFont.setTextColor(0, 0, 0);
    saveFont.setColor(0, 220, 0);
    saveFont.setFontSize(20);
    saveFont.hide();

    discardChanges.redefine(screenWidth/2 - 180, 520, 360, 40, "Discard Changes");
    discardChanges.setFont(defaultFont);
    discardChanges.setTextColor(0, 0, 0);
    discardChanges.setColor(80, 80, 80);
    discardChanges.setFontSize(20);
    discardChanges.hide();

    editingFontName.redefine(screenWidth/2, 40, "Font Name Here");
    editingFontName.setFont(defaultFont);
    editingFontName.setAlignment("center");
    editingFontName.setTextColor(255, 255, 255);
    editingFontName.setFontSize(20);
    editingFontName.hide();

    editingFontNameInFont.redefine(screenWidth/2, 100, "Font Name Here");
    editingFontNameInFont.setAlignment("center");
    editingFontNameInFont.setTextColor(255, 255, 255);
    editingFontNameInFont.setFontSize(30);
    editingFontNameInFont.hide();
}

void interface::makeMetadataEditMenuElements() {

    fontCharSpacingInput.redefine(screenWidth/2 - 150, 170, 300, 40);
    fontCharSpacingInput.setAlignment("left");
    fontCharSpacingInput.setFontSize(20);
    fontCharSpacingInput.setFont(defaultFont);
    fontCharSpacingInput.setTextColor(0, 0, 0);
    fontCharSpacingInput.setColor(255, 255, 255);
    fontCharSpacingInput.clearTextOnSubmit(false);
    fontCharSpacingInput.hide();

    fontCharSpacingInputLabel.redefine(screenWidth/2, 140, "Font character spacing (currently: ):");
    fontCharSpacingInputLabel.setFont(defaultFont);
    fontCharSpacingInputLabel.setFontSize(15);
    fontCharSpacingInputLabel.setTextColor(0, 0, 0);
    fontCharSpacingInputLabel.setAlignment("center");
    fontCharSpacingInputLabel.hide();

    fontSpaceWidthInput.redefine(screenWidth/2 - 150, 260, 300, 40);
    fontSpaceWidthInput.setAlignment("left");
    fontSpaceWidthInput.setFontSize(20);
    fontSpaceWidthInput.setFont(defaultFont);
    fontSpaceWidthInput.setTextColor(0, 0, 0);
    fontSpaceWidthInput.setColor(255, 255, 255);
    fontSpaceWidthInput.clearTextOnSubmit(false);
    fontSpaceWidthInput.hide();

    fontSpaceWidthInputLabel.redefine(screenWidth/2, 230, "Font space width (currently: ):");
    fontSpaceWidthInputLabel.setFont(defaultFont);
    fontSpaceWidthInputLabel.setFontSize(15);
    fontSpaceWidthInputLabel.setTextColor(0, 0, 0);
    fontSpaceWidthInputLabel.setAlignment("center");
    fontSpaceWidthInputLabel.hide();

    fontNameInput.redefine(screenWidth/2 - 150, 350, 300, 40);
    fontNameInput.setAlignment("left");
    fontNameInput.setFontSize(20);
    fontNameInput.setFont(defaultFont);
    fontNameInput.setTextColor(0, 0, 0);
    fontNameInput.setColor(255, 255, 255);
    fontNameInput.clearTextOnSubmit(false);
    fontNameInput.hide();

    fontNameInputLabel.redefine(screenWidth/2, 320, "Font name:");
    fontNameInputLabel.setFont(defaultFont);
    fontNameInputLabel.setFontSize(20);
    fontNameInputLabel.setAlignment("center");
    fontNameInputLabel.setTextColor(0, 0, 0);
    fontNameInputLabel.hide();

    MDBackButton.redefine(screenWidth/2 - 80, 400, 160, 50, "Back");
    MDBackButton.setColor(160, 160, 160);
    MDBackButton.setTextColor(0, 0, 0);
    MDBackButton.setFont(defaultFont);
    MDBackButton.setFontSize(30);
    MDBackButton.hide();

}

void interface::makeCharacterSelectMenuElements() {

    characterSelectButtons = new button[maxChar - minChar + 1];
    characterSelectButtonLabels = new textBox[maxChar - minChar + 1];
    for(int i = 0; i <= maxChar-minChar; i++) {
        int x = (i % CScols)*75 + 75;
        int y = ((i / CScols) % CSrows)*80 + 200;
        characterSelectButtons[i].redefine(x, y, 50, 40, string(1, (char)(i+minChar)));
        characterSelectButtonLabels[i].redefine(x+25, y-25, string(1, (char)(i+minChar)));

        characterSelectButtons[i].setFontSize(30);
        characterSelectButtons[i].setTextColor(0, 0, 0);
        characterSelectButtons[i].setColor(255, 255, 255);
        characterSelectButtons[i].hide();

        characterSelectButtonLabels[i].setFont(defaultFont);
        characterSelectButtonLabels[i].setFontSize(20);
        characterSelectButtonLabels[i].setTextColor(0, 0, 0);
        characterSelectButtonLabels[i].setAlignment("center");
        characterSelectButtonLabels[i].hide();
    }

    CSMleft.redefine(10, screenHeight/2 + 20, 30, 30, "<");
    CSMleft.setFont(defaultFont);
    CSMleft.setTextColor(0, 0, 0);
    CSMleft.setColor(220, 220, 220);
    CSMleft.setFontSize(20);
    CSMleft.hide();

    CSMright.redefine(screenWidth - 40, screenHeight/2 + 20, 30, 30, ">");
    CSMright.setFont(defaultFont);
    CSMright.setTextColor(0, 0, 0);
    CSMright.setColor(220, 220, 220);
    CSMright.setFontSize(20);
    CSMright.hide();

    CSMBackButton.redefine(screenWidth/2 - 50, screenHeight - 40, 100, 30, "Back");
    CSMBackButton.setFont(defaultFont);
    CSMBackButton.setFontSize(20);
    CSMBackButton.setColor(120, 120, 120);
    CSMBackButton.setTextColor(0, 0, 0);
    CSMBackButton.hide();

    CSMtitle.redefine(screenWidth/2, 20, "Select a\ncharacter\nto edit");
    CSMtitle.setFont(defaultFont);
    CSMtitle.setTextColor(0, 0, 0);
    CSMtitle.setFontSize(30);
    CSMtitle.setAlignment("center");
    CSMtitle.hide();

}

void interface::makeCharacterEditorElements() {

    charName.redefine(screenWidth/2, 30, "Character name goes here");
    charName.setTextColor(0, 0, 0);
    charName.setFont(defaultFont);
    charName.setFontSize(20);
    charName.setAlignment("center");
    charName.hide();

    saveCharChanges.redefine(20, 110, 180, 60, "Save\nChanges");
    saveCharChanges.setColor(50, 255, 50);
    saveCharChanges.setFont(defaultFont);
    saveCharChanges.setFontSize(20);
    saveCharChanges.setTextColor(0, 0, 0);
    saveCharChanges.hide();

    discardCharChanges.redefine(screenWidth - 200, 110, 180, 60, "Discard\nChanges");
    discardCharChanges.setColor(150, 150, 150);
    discardCharChanges.setFont(defaultFont);
    discardCharChanges.setFontSize(20);
    discardCharChanges.setTextColor(0, 0, 0);
    discardCharChanges.hide();

    charPreview.redefine(screenWidth/2, 120, "char goes here");
    charPreview.setTextColor(0, 0, 0);
    charPreview.setFontSize(50);
    charPreview.setAlignment("center");
    charPreview.hide();

    clearChar.redefine(10, screenHeight - 40, 80, 30, "Clear");
    clearChar.setColor(80, 80, 80);
    clearChar.setTextColor(0, 0, 0);
    clearChar.setFont(defaultFont);
    clearChar.setFontSize(15);
    clearChar.hide();

}

void interface::makeFontPreviewElements() {

    previewSectionName.redefine(screenWidth/2, 10, "Section name goes here");
    previewSectionName.setFont(defaultFont);
    previewSectionName.setAlignment("center");
    previewSectionName.setFontSize(40);
    previewSectionName.setTextColor(0, 0, 0);
    previewSectionName.hide();
    
    string QBFtextLowerBig = "the quick, brown\nfox jumped over\nthe lazy dog.";
    string QBFtextUpperBig = "THE QUICK, BROWN\nFOX JUMPED OVER\nTHE LAZY DOG.";

    string QBFtextLower = "the quick, brown fox\njumped over the lazy dog.";
    string QBFtextUpper = "THE QUICK, BROWN FOX\nJUMPED OVER THE LAZY DOG.";

    previewQuickBrownFoxUpper.redefine(screenWidth/2, 330, QBFtextUpper);
    previewQuickBrownFoxUpper.setAlignment("center");
    previewQuickBrownFoxUpper.setFontSize(20);
    previewQuickBrownFoxUpper.setTextColor(255, 255, 255);
    previewQuickBrownFoxUpper.hide();

    previewQuickBrownFoxLower.redefine(screenWidth/2, 390, QBFtextLower);
    previewQuickBrownFoxLower.setAlignment("center");
    previewQuickBrownFoxLower.setFontSize(20);
    previewQuickBrownFoxLower.setTextColor(255, 255, 255);
    previewQuickBrownFoxLower.hide();

    previewQuickBrownFoxBigUpper.redefine(screenWidth/2, 90, QBFtextUpperBig);
    previewQuickBrownFoxBigUpper.setAlignment("center");
    previewQuickBrownFoxBigUpper.setFontSize(30);
    previewQuickBrownFoxBigUpper.setTextColor(255, 255, 255);
    previewQuickBrownFoxBigUpper.hide();

    previewQuickBrownFoxBigLower.redefine(screenWidth/2, 210, QBFtextLowerBig);
    previewQuickBrownFoxBigLower.setAlignment("center");
    previewQuickBrownFoxBigLower.setFontSize(30);
    previewQuickBrownFoxBigLower.setTextColor(255, 255, 255);
    previewQuickBrownFoxBigLower.hide();

    previewQuickBrownFoxLittleUpper.redefine(screenWidth/2, 460, QBFtextUpper);
    previewQuickBrownFoxLittleUpper.setAlignment("center");
    previewQuickBrownFoxLittleUpper.setFontSize(10);
    previewQuickBrownFoxLittleUpper.setTextColor(255, 255, 255);
    previewQuickBrownFoxLittleUpper.hide();

    previewQuickBrownFoxLittleLower.redefine(screenWidth/2, 500, QBFtextLower);
    previewQuickBrownFoxLittleLower.setAlignment("center");
    previewQuickBrownFoxLittleLower.setFontSize(10);
    previewQuickBrownFoxLittleLower.setTextColor(255, 255, 255);
    previewQuickBrownFoxLittleLower.hide();

}


//screen setup
void interface::setupScreen() {

    border = 100;

    screenWidth = 400 + 2*border;
    screenHeight = 400 + 2*border;
    theScreen = makeScreen("Font Editor", screenWidth, screenHeight);
    setBackground(220, 220, 220);
}


// loading/ticking/unloading menus
void interface::changeMenu(int menuID) {
    switch(myMenuID) {
        case 0: unloadFontSelectMenu(); break;
        case 1: unloadFontEditMenu(); break;
        case 2: unloadMetadataEditMenu(); break;
        case 3: unloadCharacterSelectMenu(); break;
        case 4: unloadCharacterEditMenu(); break;
        case 5: unloadPreviewMenu(); break;
    }

    myMenuID = menuID;

    switch(myMenuID) {
        case 0: loadFontSelectMenu(); break;
        case 1: loadFontEditMenu(); break;
        case 2: loadMetadataEditMenu(); break;
        case 3: loadCharacterSelectMenu(); break;
        case 4: loadCharacterEditMenu(); break;
        case 5: loadPreviewMenu(); break;
    }
}

void interface::tickMenu() {
    switch(myMenuID) {
        case 0: tickFontSelectMenu(); break;
        case 1: tickFontEditMenu(); break;
        case 2: tickMetadataEditMenu(); break;
        case 3: tickCharacterSelectMenu(); break;
        case 4: tickCharacterEditMenu(); break;
        case 5: tickPreviewMenu(); break;
    }

    programClosed = !theScreen->isOpen();
}


// font select menu
void interface::loadFontSelectMenu() {
    setBackground(50, 150, 200);
    screenNum = 0;
    loadFSMscreen();
    
    if(fontCount > 4) {
        FSMright.show();
        FSMleft.show();
    }

    FSMtitle.show();
}

void interface::loadFSMscreen() {
    for (int i = 0; i < fontCount; i++) {
        fontSelectButtons[i].hide();
        normalFontNames[i].hide();
    }

    for (int i = screenNum*4; i < (screenNum+1)*4 && i < fontCount; i++) {
        fontSelectButtons[i].show();
        normalFontNames[i].show();
    }
}

void interface::tickFontSelectMenu() {
    if(FSMleft.getClickedState()) {
        screenNum--;
        if(screenNum < 0) {
            screenNum = (fontCount-1) / 4;
        }
        loadFSMscreen();
    } else if(FSMright.getClickedState()) {
        screenNum++;
        if(screenNum > (fontCount-1) / 4) {
            screenNum = 0;
        }
        loadFSMscreen();
    }

    for (int i = 0; i < fontCount; i++) {
        if(fontSelectButtons[i].getClickedState()) {
            chosenFont = fontSelectButtons[i].getFont();
            fontNum = i;
            changeMenu(1);
        }
    }
}

void interface::unloadFontSelectMenu() {
    FSMright.hide();
    FSMleft.hide();
    FSMtitle.hide();

    for(int i = 0; i < fontCount; i++) {
        fontSelectButtons[i].hide();
        normalFontNames[i].hide();
    }
}

//font edit menu
void interface::loadFontEditMenu() {
    setBackground(120, 10, 10);

    editingFontHandler = theScreen->getTextHandler(chosenFont);
    chosenFontDetail = editingFontHandler->getFontDetail();

    editFontMetadata.show();
    editCharacters.show();
    seePreview.show();
    saveFont.show();
    discardChanges.show();

    editingFontName.show();
    editingFontName.setFont(defaultFont);
    editingFontName.setText(chosenFont);

    editingFontNameInFont.show();
    editingFontNameInFont.setFont(chosenFont);
    editingFontNameInFont.setText(chosenFont);
}

void interface::tickFontEditMenu() {
    if(editFontMetadata.getClickedState()) {
        changeMenu(2);
    } else if(editCharacters.getClickedState()) {
        changeMenu(3);
        screenNum = 0;
    } else if(seePreview.getClickedState()) {
        changeMenu(5);
    } else if(saveFont.getClickedState()) {
        editingFontHandler->saveFontChangesToFile();
        changeMenu(0);
    } else if(discardChanges.getClickedState()) {
        editingFontHandler->reloadFromFile();
        changeMenu(0);
    }
}

void interface::unloadFontEditMenu() {
    editFontMetadata.hide();
    editCharacters.hide();
    seePreview.hide();
    saveFont.hide();
    discardChanges.hide();
    editingFontName.hide();
    editingFontNameInFont.hide();
}

//metadata edit menu
void interface::loadMetadataEditMenu() {
    setBackground(50, 50, 255);

    editingFontName.show();
    editingFontNameInFont.show();

    fontNameInput.setText(editingFontHandler->getFontName());
    fontNameInput.getSubmittedText();
    fontNameInput.show();

    fontNameInputLabel.show();

    fontSpaceWidthInput.setText(to_string(editingFontHandler->getSpacewidth()));
    fontSpaceWidthInput.getSubmittedText();
    fontSpaceWidthInput.show();

    fontSpaceWidthInputLabel.setText("Font space width (currently " + to_string(editingFontHandler->getSpacewidth()) + "):");
    fontSpaceWidthInputLabel.show();

    fontCharSpacingInput.setText(to_string(editingFontHandler->getCharSpacing()));
    fontCharSpacingInput.getSubmittedText();
    fontCharSpacingInput.show();

    fontCharSpacingInputLabel.setText("Font character spacing (currently " + to_string(editingFontHandler->getCharSpacing()) + "):");
    fontCharSpacingInputLabel.show();

    MDBackButton.show();

}

void interface::tickMetadataEditMenu() {
    if(fontNameInput.hasBeenSubmitted()) {
        string newFontName = fontNameInput.getSubmittedText();
        editingFontHandler->setFontName(newFontName);
        editingFontName.setText(newFontName);
        editingFontNameInFont.setText(newFontName);
        editingFontNameInFont.setFont(newFontName);
        fontSelectButtons[fontNum].setFont(newFontName);
        fontSelectButtons[fontNum].setText(newFontName);
        normalFontNames[fontNum].setText(newFontName);
        chosenFont = newFontName;
    } 
    else if(fontSpaceWidthInput.hasBeenSubmitted()) {
        string newSpaceWidth = fontSpaceWidthInput.getSubmittedText();
        bool isNum = true;
        for(int i = 0; i < newSpaceWidth.length(); i++) {
            if(!isdigit(newSpaceWidth[i])) {
                isNum = false;
                break;
            }
        }
        if(isNum) {
            editingFontHandler->setSpacewidth(stoi(newSpaceWidth));
            fontSpaceWidthInputLabel.setText("Font space width (currently " + newSpaceWidth + "):");
        }
    }
    else if(fontCharSpacingInput.hasBeenSubmitted()) {
        string newCharSpacing = fontCharSpacingInput.getSubmittedText();
        bool isNum = true;
        for(int i = 0; i < newCharSpacing.length(); i++) {
            if(!isdigit(newCharSpacing[i])) {
                isNum = false;
                break;
            }
        }
        if(isNum) {
            editingFontHandler->setCharSpacing(stoi(newCharSpacing));
            fontCharSpacingInputLabel.setText("Font character spacing (currently " + newCharSpacing + "):");
        }
    }
    else if(MDBackButton.getClickedState()) {
        changeMenu(1);
    }
}

void interface::unloadMetadataEditMenu() {
    editingFontName.hide();
    editingFontNameInFont.hide();
    fontNameInput.hide();
    fontNameInputLabel.hide();
    fontCharSpacingInput.hide();
    fontCharSpacingInputLabel.hide();
    fontSpaceWidthInput.hide();
    fontSpaceWidthInputLabel.hide();
    MDBackButton.hide();
}

//character select menu
void interface::loadCharacterSelectMenu() {
    setBackground(200, 200, 200);

    loadCharSelectScreen(screenNum);
    for(int i = 0; i <= maxChar-minChar; i++) {
        characterSelectButtons[i].setFont(chosenFont);
    }

    CSMleft.show();
    CSMright.show();
    CSMBackButton.show();
    CSMtitle.show();
}

void interface::loadCharSelectScreen(int n) {
    for(int i = 0; i <= maxChar-minChar; i++) {
        characterSelectButtons[i].hide();
        characterSelectButtonLabels[i].hide();
    }

    int buttonsPerScreen = CSrows*CScols;
    for(int i = buttonsPerScreen*n; i < buttonsPerScreen*(n+1) && i <= maxChar-minChar; i++) {
        characterSelectButtons[i].show();
        characterSelectButtonLabels[i].show();
    }
}

void interface::tickCharacterSelectMenu() {
    for(int i = 0; i <= maxChar-minChar; i++) {
        if(characterSelectButtons[i].getClickedState()) {
            chosenChar = (char)(i+minChar);
            changeMenu(4);
        }
    }

    if(CSMleft.getClickedState()) {
        screenNum--;
        if(screenNum < 0) {
            screenNum = (maxChar-minChar)/(CSrows*CScols);
        }
        loadCharSelectScreen(screenNum);
    } else if(CSMright.getClickedState()) {
        screenNum++;
        if(screenNum > (maxChar-minChar)/(CSrows*CScols)) {
            screenNum = 0;
        }
        loadCharSelectScreen(screenNum);
    } else if(CSMBackButton.getClickedState()) {
        changeMenu(1);
    }
}

void interface::unloadCharacterSelectMenu() {
    for(int i = 0; i <= maxChar-minChar; i++) {
        characterSelectButtons[i].hide();
        characterSelectButtonLabels[i].hide();
    }

    CSMleft.hide();
    CSMright.hide();
    CSMBackButton.hide();
    CSMtitle.hide();
}

//character edit menu
void interface::loadCharacterEditMenu() {
    setBackground(100, 100, 200);

    loadCharacterEditorButtons();

    string currCharName = characterNames[(int)chosenChar - minChar];
    charName.setText(currCharName + " (" + chosenChar + ")");
    charName.show();

    charPreview.setText(string(1, chosenChar));
    charPreview.setFont(chosenFont);
    charPreview.show();

    saveCharChanges.show();
    discardCharChanges.show();
    clearChar.show();
}

void interface::loadCharacterEditorButtons() {
    charHandler = editingFontHandler->getCharacterClass(chosenChar);
    bool **bitmap = charHandler->getBitmap();
    currCharBitmap = new bool*[chosenFontDetail];
    originalBitmap = bitmap;

    int availNoteWidth = screenWidth - 2*border;
    int availNoteHeight = screenHeight - 2*border;
    int buffer = 2;
    int buttonWidth = availNoteWidth/chosenFontDetail - buffer*2;
    int buttonHeight = availNoteHeight/chosenFontDetail - buffer*2;
    editorButtons = new button*[chosenFontDetail];

    int startX = buffer + 2*border - 20;
    int startY = buffer + border;
    for(int i = 0; i < chosenFontDetail; i++) {
        editorButtons[i] = new button[chosenFontDetail];
        currCharBitmap[i] = new bool[chosenFontDetail];
        for(int j = 0; j < chosenFontDetail; j++) {
            currCharBitmap[i][j] = bitmap[i][j];
            
            int y = i*buttonWidth + i*2*buffer + startX;
            int x = j*buttonHeight + j*2*buffer + startY;
            editorButtons[i][j].redefine(x, y, buttonWidth, buttonHeight);
            if(!currCharBitmap[i][j]) {editorButtons[i][j].setColor(220, 220, 220);}
            else {editorButtons[i][j].setColor(0, 0, 0);}
            editorButtons[i][j].noStroke();
            editorButtons[i][j].show();
        }
    }


}

void interface::updateCharacterEditorButton(int i, int j) {
    if(!currCharBitmap[i][j]) {editorButtons[i][j].setColor(220, 220, 220);}
    else {editorButtons[i][j].setColor(0, 0, 0);}
    charHandler->updateBitmap(currCharBitmap);
}

void interface::tickCharacterEditMenu() {
    for(int i = 0; i < chosenFontDetail; i++) {
        for(int j = 0; j < chosenFontDetail; j++) {
            if(editorButtons[i][j].getClickedState()) {
                currCharBitmap[i][j] = !currCharBitmap[i][j];
                updateCharacterEditorButton(i, j);
            }
        }
    }

    if(clearChar.getClickedState()) {
        for(int i = 0; i < chosenFontDetail; i++) {
            for(int j = 0; j < chosenFontDetail; j++) {
                currCharBitmap[i][j] = false;
                updateCharacterEditorButton(i, j);
            }
        }
    } else if(saveCharChanges.getClickedState()) {
        changeMenu(3);
    } else if(discardCharChanges.getClickedState()) {
        charHandler->updateBitmap(originalBitmap);
        changeMenu(3);
    }
}

void interface::unloadCharacterEditMenu() {
    unloadCharacterEditorButtons();

    charName.hide();
    charPreview.hide();
    saveCharChanges.hide();
    discardCharChanges.hide();
    clearChar.hide();
}


//font preview menu
void interface::loadPreviewMenu() {
    screenNum = 0;
    previewQuickBrownFoxBigLower.setFont(chosenFont);
    previewQuickBrownFoxBigUpper.setFont(chosenFont);
    previewQuickBrownFoxLittleLower.setFont(chosenFont);
    previewQuickBrownFoxLittleUpper.setFont(chosenFont);
    previewQuickBrownFoxLower.setFont(chosenFont);
    previewQuickBrownFoxUpper.setFont(chosenFont);
    loadPreviewSection(0);
}

void interface::loadPreviewSection(int n) {

    setBackground(30, 150, 30);

    unloadPreviewMenu();

    CSMBackButton.show();
    CSMleft.show();
    CSMright.show();
    previewSectionName.show();

    if(n == 0) {
        previewSectionName.setText("Letters");
        previewQuickBrownFoxBigLower.show();
        previewQuickBrownFoxBigUpper.show();
        previewQuickBrownFoxLittleLower.show();
        previewQuickBrownFoxLittleUpper.show();
        previewQuickBrownFoxLower.show();
        previewQuickBrownFoxUpper.show();
    }
    else if(n == 1) {
        //dunno yet
    }

}

void interface::tickPreviewMenu() {
    if(CSMBackButton.getClickedState()) {
        changeMenu(1);
    }
    else if(CSMleft.getClickedState()) {
        screenNum--;
        if(screenNum < 0) {
            screenNum = previewScreens-1;
        }
        loadPreviewSection(screenNum);
    }
    else if(CSMright.getClickedState()) {
        screenNum++;
        if(screenNum >= previewScreens) {
            screenNum = 0;
        }
        loadPreviewSection(screenNum);
    }
}

void interface::unloadPreviewMenu() {

    CSMBackButton.hide();
    CSMleft.hide();
    CSMright.hide();
    previewSectionName.hide();
    previewSectionName.hide();

    previewQuickBrownFoxBigLower.hide();
    previewQuickBrownFoxBigUpper.hide();
    previewQuickBrownFoxLittleLower.hide();
    previewQuickBrownFoxLittleUpper.hide();
    previewQuickBrownFoxLower.hide();
    previewQuickBrownFoxUpper.hide();

}


void interface::unloadCharacterEditorButtons() {
    for(int i = 0; i < chosenFontDetail; i++) {
        for(int j = 0; j < chosenFontDetail; j++) {
            theScreen->remElement((screenElement*)(&editorButtons[i][j]));
        }
    }
}

bool interface::isOpen() {
    return !programClosed;
}