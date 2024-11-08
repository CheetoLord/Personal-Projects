
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <queue>
#include <vector>
#include <string>
#include "textHandler.h"

using namespace std;

#ifndef _SCREEN_H_
#define _SCREEN_H_


class screenElement {
public:
    void draw();
    bool checkClick(int x, int y);
    void beClicked();
    string getType();
    string myType;
};




class screen {
public:
    //constructors
    screen();
    screen(int width, int height);
    void initDefaults();


    //getters
    int getWidth() const;
    int getHeight() const;
    int getFontSize() const;
    bool getMousePressed() const;
    bool getMousePressedFirstFrame() const;

    //setups
    void initDisplay();
    void matchVisualInfo();
    void setWindowAttrs();
    void finalize(string name);
    void setSizeHint(int minWidth, int minHeight, int maxWidth, int maxHeight);
    void setupInput();

    bool isOpen();
    void close();

    Status toggleMaximize();

    void handleEvents();
    void updateMouseStatus();
    void checkSelectedInputs();
    bool isPrintable(int symbol);
    void adjustSize();
    void beforeDraw();
    void finishDraw();

    bool getKeyPressed(int keyCode);
    bool newKeyEvents() {return !newKeyUpdates.empty();};
    char getKeyEvent();
    int getMouseX() {return (int) mouseX;};
    int getMouseY() {return (int) mouseY;};
    bool newMouseEvents() {return !newMouseUpdates.empty();};
    void getMouseEvent(int &outX, int &outY);
    bool newScrollEvents() {return !newScrollUpdates.empty();};
    bool getScrollEvent();

    void setStroke(int r, int g, int b);
    void setStroke(int color);
    void setStrokeWeight(int weight);
    void noStroke();

    void setFill(int r, int g, int b);
    void setFill(int color);
    void noFill();

    void setBGcolor(int color);
    void setBGcolor(int r, int g, int b);
    void noBG();

    void setFontSize(int fontSize);
    void refreshFontNames();
    void setFont(string fontName);

    string *getListOfFonts(int &retFontCount);
    textHandler *getTextHandler(string fontName);

    void drawBackground(int r, int g, int b);
    void drawBackground(int myColor);
    void drawBackground();
    void drawRect(int x, int y, int width, int height);
    void drawText(string text, int x, int y);
    int getTextWidth(string text);
    int getTextLineWidth(string text);
    int getTextHeight(string text);
    int getNewlineYDiff();

    void checkElementClicked();

    void addElement(screenElement* newElem);
    void remElement(screenElement *elem);
    bool hasElement(screenElement *elem);
    void drawElements();


private:
    int myWidth;
    int myHeight;

    Display* myDisplay;
    int myConnectionNumber;

    int myRoot;
    int myDefaultScreen;

    int myScreenBitDepth;
    XVisualInfo myVisinfo;

    XSetWindowAttributes myWindowAttr;
    Window myWindow;
    Atom myWM_DELETE_WINDOW;

    int myPixelBits;
    int myPixelBytes;
    int myWindowBufferSize;
    char* myMem;
    XImage* myXWindowBuffer;
    GC myDefaultGC;
    GC myActiveGC;

    bool windowOpen;
    int sizeChange;

    bool pressedKeys[256];
    queue<char> newKeyUpdates;
    queue<int> newMouseUpdates;
    queue<int> newScrollUpdates;
    int keycodeMap[256];
    XIM myXInputMethod;
    XIMStyles* myStyles;
    XIMStyle myBestMatchStyle;
    XIC myXInputContext;

    int mouseX;
    int mouseY;
    bool mouseInScreen;
    bool mouseClicked;
    bool mouseScrollUp;
    bool mouseScrollDown;
    bool firstFrameMouseClicked;

    int myFontSize;

    bool doFill;
    int fillR;
    int fillG;
    int fillB;

    bool doStroke;
    int strokeR;
    int strokeG;
    int strokeB;
    int strokeWeight;

    bool doBackground;
    int BGR;
    int BGG;
    int BGB;

    textHandler *myTextHandler;
    textHandler **myFonts;
    string *myFontNames;
    string *myFontFileNames;
    int myKnownFontCount;

    screenElement *selectedInput;

    vector<screenElement*> myElements;
    int myHeldElems;

};



















// helper classes/functions

void waitForFrame(float frameMS);

screen* makeScreen();
screen* makeScreen(string name);
screen* makeScreen(int width, int height);
screen* makeScreen(string name, int width, int height);

void maximizeScreen();

void setBackground(int color);
void setBackground(int r, int g, int b);
void noBackground();

int calcTextWidth(string text, int size);




class rectangle : screenElement {
public:
    //constructors
    rectangle();
    rectangle(int x, int y, int width, int height);

    void redefine(int x, int y, int width, int height);

    string getType();

    //getters
    int getX();
    int getY();
    int getWidth();
    int getHeight();
    int getColor();
    int getRed();
    int getGreen();
    int getBlue();
    bool isShowing();

    //setters
    void setX(int x);
    void setY(int y);
    void setPos(int x, int y);
    void setWidth(int width);
    void setHeight(int height);
    void setColor(int color);
    void setColor(int r, int g, int b);
    void setRed(int r);
    void setGreen(int g);
    void setBlue(int b);

    void show();
    void hide();



    void draw();

private:
    int myX;
    int myY;
    int myWidth;
    int myHeight;
    int myColor;
    bool showing;
};



class textBox : screenElement {
public:
    //constructors
    textBox();
    textBox(string text);
    textBox(int x, int y);
    textBox(int x, int y, string text);

    void redefine(int x, int y);
    void redefine(int x, int y, string text);

    string getType();

    //getters
    int getX();
    int getY();
    int getTextColor();
    int getTextRed();
    int getTextGreen();
    int getTextBlue();
    bool isShowing();
    string getAlignment();
    string getText();
    int getFontSize();
    string getFont();

    //setters
    void setX(int x);
    void setY(int y);
    void setTextColor(int color);
    void setTextColor(int r, int g, int b);
    void setTextRed(int r);
    void setTextGreen(int g);
    void setTextBlue(int b);
    void setAlignment(string alignment);
    void setText(string text);
    void setFontSize(int fontSize);
    void setFont(string fontName);

    void show();
    void hide();

    void draw();

private:
    int myX;
    int myY;
    int myTextColor;
    bool showing;
    string myText;
    string myAlignment;
    int myFontSize;
    string myFontName;
};



class button : screenElement {
public:
    //constructors
    button();
    button(string label);
    button(int x, int y, int width, int height);
    button(int x, int y, int width, int height, string label);

    void redefine(int x, int y, int width, int height);
    void redefine(int x, int y, int width, int height, string label);

    string getType();

    //getters
    int getX();
    int getY();
    int getWidth();
    int getHeight();
    int getColor();
    int getRed();
    int getGreen();
    int getBlue();
    int getStrokeColor();
    int getStrokeRed();
    int getStrokeGreen();
    int getStrokeBlue();
    int getStrokeWeight();
    bool getDoingStroke();
    int getTextColor();
    int getTextRed();
    int getTextGreen();
    int getTextBlue();
    bool isShowing();
    string getText();
    int getFontSize();
    string getFont();
    bool getClickedState();

    //setters
    void setX(int x);
    void setY(int y);
    void setWidth(int width);
    void setHeight(int height);
    void setColor(int color);
    void setColor(int r, int g, int b);
    void setRed(int r);
    void setGreen(int g);
    void setBlue(int b);
    void setStrokeColor(int color);
    void setStrokeColor(int r, int g, int b);
    void setStrokeRed(int r);
    void setStrokeGreen(int g);
    void setStrokeBlue(int b);
    void setStrokeWeight(int weight);
    void noStroke();
    void setTextColor(int color);
    void setTextColor(int r, int g, int b);
    void setTextRed(int r);
    void setTextGreen(int g);
    void setTextBlue(int b);
    void setText(string text);
    void setFont(string fontName);
    void setFontSize(int fontSize);

    int checkClicked(int mouseX, int mouseY);

    void show();
    void hide();

    void draw();

public:
    int myX;
    int myY;
    int myWidth;
    int myHeight;
    int myColor;
    int myStrokeColor;
    int myTextColor;
    bool doingStroke;
    int myStrokeWeight;
    bool showing;
    string myLabel;
    int myFontSize;
    string myFontName;
    bool beenClicked;
};





class inputBox : screenElement {
public:
    //constructors
    inputBox();
    inputBox(string textHint);
    inputBox(int x, int y, int width, int height);
    inputBox(int x, int y, int width, int height, string textHint);

    void redefine(int x, int y, int width, int height);
    void redefine(int x, int y, int width, int height, string textHint);

    string getType();

    //getters
    int getX();
    int getY();
    int getWidth();
    int getHeight();

    int getColor();
    int getRed();
    int getGreen();
    int getBlue();

    int getTextColor();
    int getTextRed();
    int getTextGreen();
    int getTextBlue();

    int getHintColor();
    int getHintRed();
    int getHintGreen();
    int getHintBlue();

    bool isShowing();

    string getAlignment();
    int getFontSize();
    string getFont();
    string getHint();

    string getSubmittedText();
    bool hasBeenSubmitted();

    //setters
    void setX(int x);
    void setY(int y);
    void setWidth(int width);
    void setHeight(int height);

    void setColor(int color);
    void setColor(int r, int g, int b);
    void setRed(int r);
    void setGreen(int g);
    void setBlue(int b);

    void setTextColor(int color);
    void setTextColor(int r, int g, int b);
    void setTextRed(int r);
    void setTextGreen(int g);
    void setTextBlue(int b);

    void setHintColor(int color);
    void setHintColor(int r, int g, int b);
    void setHintRed(int r);
    void setHintGreen(int g);
    void setHintBlue(int b);

    void setAlignment(string alignment);
    void setText(string text);
    void setFontSize(int fontSize);
    void setFont(string fontName);

    void backSpace();
    void submit();
    void sendChar(char c);
    void clearInput();

    void select();
    void deselect();

    void clearTextOnSubmit(bool clears);

    bool checkClicked(int mouseX, int mouseY);
    void incCursorFlashTimer();

    void show();
    void hide();

    void draw();

private:
    int myX;
    int myY;
    int myHeight;
    int myWidth;
    int myTextColor;
    int myColor;
    int myHintColor;
    bool showing;
    string myAlignment;
    int myFontSize;
    string myFontName;
    string myText;
    string myHint;
    string mySubmittedText;
    bool isSubmitted;
    bool isSelected;
    bool submitClearsText;
    int cursorFlashTimer;
};




#endif