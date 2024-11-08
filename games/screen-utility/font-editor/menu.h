


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
    void readCharNames();
    void makeFontSelectMenuElements();
    void makeFontEditMenuElements();
    void makeMetadataEditMenuElements();
    void makeCharacterSelectMenuElements();
    void makeCharacterEditorElements();
    void makeFontPreviewElements();

    void setupScreen();

    void changeMenu(int menuID);
    void tickMenu();

    void loadFontSelectMenu();
    void loadFSMscreen();
    void tickFontSelectMenu();
    void unloadFontSelectMenu();

    void loadFontEditMenu();
    void tickFontEditMenu();
    void unloadFontEditMenu();

    void loadMetadataEditMenu();
    void tickMetadataEditMenu();
    void unloadMetadataEditMenu();

    void loadCharacterSelectMenu();
    void loadCharSelectScreen(int n);
    void tickCharacterSelectMenu();
    void unloadCharacterSelectMenu();

    void loadCharacterEditMenu();
    void loadCharacterEditorButtons();
    void updateCharacterEditorButton(int i, int j);
    void tickCharacterEditMenu();
    void unloadCharacterEditMenu();
    void unloadCharacterEditorButtons();

    void loadPreviewMenu();
    void loadPreviewSection(int n);
    void tickPreviewMenu();
    void unloadPreviewMenu();

    bool isOpen();

private:
    //id key:
    //0 = font select screen
    //1 = font edit screen
    //2 = font metadata edit screen
    //3 = character select menu
    //4 = character edit menu
    //5 = font preview menu
    int myMenuID;

    int border;
    int screenWidth;
    int screenHeight;
    screen *theScreen;


    //font select menu
    int fontCount;
    int screenNum; //also used in charater select menu
    button *fontSelectButtons;
    button FSMleft;
    button FSMright;
    textBox *normalFontNames;
    textBox FSMtitle;

    //font edit menu
    int fontNum;
    string chosenFont;
    string chosenFontFile;
    int chosenFontDetail;
    button editFontMetadata;
    button editCharacters;
    button seePreview;
    button saveFont;
    button discardChanges;
    textBox editingFontName;
    textBox editingFontNameInFont;
    textHandler *editingFontHandler;

    //font metadata edit menu
    textBox fontCharSpacingInputLabel;
    inputBox fontCharSpacingInput;
    textBox fontSpaceWidthInputLabel;
    inputBox fontSpaceWidthInput;
    textBox fontNameInputLabel;
    inputBox fontNameInput;
    button MDBackButton;

    //character select menu
    int minChar = 33;
    int maxChar = 126;
    int CScols = 6;
    int CSrows = 4;
    button *characterSelectButtons;
    textBox *characterSelectButtonLabels;
    button CSMleft;
    button CSMright;
    button CSMBackButton;
    textBox CSMtitle;

    //character edit menu
    string *characterNames;
    char chosenChar;
    bool **currCharBitmap;
    character *charHandler;
    button **editorButtons;
    textBox charName;
    button saveCharChanges;
    button discardCharChanges;
    bool **originalBitmap;
    textBox charPreview;
    button clearChar;

    //font preview menu
    //gonna reuse CSMBackButton
    //gonna also reuse CSMleft and CSMright
    //and screenNum
    int previewScreens = 2;
    textBox previewSectionName;
    textBox previewQuickBrownFoxUpper;
    textBox previewQuickBrownFoxLower;
    textBox previewQuickBrownFoxBigUpper;
    textBox previewQuickBrownFoxBigLower;
    textBox previewQuickBrownFoxLittleUpper;
    textBox previewQuickBrownFoxLittleLower;
    

    bool programClosed;
    string defaultFont;
};



#endif