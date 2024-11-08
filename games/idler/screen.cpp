#include "screen.h"

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <time.h>

#include <chrono>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <thread>

using namespace std;

// most code with anything related to initializing the xlib window is copied from:
// https://handmade.network/forums/articles/t/2834-tutorial_a_tour_through_xlib_and_related_technologies
// mostly, it is just the screen class' functions:
// initDisplay
// matchVisualInfo
// setWindowAttrs
// finalize
// setSizeHint
// setupInput
// beforeDraw
// finishDraw
// adjustSize
// toggleMaximize
// and handleEvents.
// A few other xlib related functions were inspired from other websites, but I think that
// I changed them enough for them to not necessarily need be credited
// (I also just forgot where they came from).

// constructors
screen::screen() {
    myWidth = 600;
    myHeight = 400;
    initDefaults();
}

screen::screen(int width, int height) {
    myWidth = width;
    myHeight = height;
    initDefaults();
}

void screen::initDefaults() {
    XInitThreads();

    for (int i = 0; i < 256; i++) {
        pressedKeys[i] = false;
        keycodeMap[i] = 0;
    }

    leftArrowPressed = false;
    rightArrowPressed = false;
    upArrowPressed = false;
    downArrowPressed = false;

    doFill = true;
    fillR = 0;
    fillG = 0;
    fillB = 0;

    strokeWeight = 0;
    doStroke = false;
    strokeR = 0;
    strokeG = 0;
    strokeB = 0;

    doBackground = true;
    BGR = 0;
    BGG = 0;
    BGB = 0;

    myHeldElems = 0;
    myElements = vector<screenElement*>();

    myKnownFontCount = getAvailFonts(myFontFileNames);

    myFonts = new textHandler*[myKnownFontCount];
    myFontNames = new string[myKnownFontCount];
    for(int i = 0; i < myKnownFontCount; i++) {
        myFonts[i] = new textHandler("fonts/" + myFontFileNames[i]);
        myFontNames[i] = myFonts[i]->getFontName();
    }

    selectedInput = nullptr;

    setFont("basic 10x10");
}

// getters
int screen::getWidth() const {
    return myWidth;
}
int screen::getHeight() const {
    return myHeight;
}

bool screen::isOpen() {
    return windowOpen;
}

int screen::getFontSize() const {
    return myFontSize;
}

bool screen::getMousePressed() const {
    return mouseClicked;
}

bool screen::getMousePressedFirstFrame() const {
    return firstFrameMouseClicked;
}

// setups
void screen::initDisplay() {
    myDisplay = XOpenDisplay(0);

    if (!myDisplay) {
        throw runtime_error("No display available");
    }

    myRoot = DefaultRootWindow(myDisplay);
    myDefaultScreen = DefaultScreen(myDisplay);
}

void screen::matchVisualInfo() {
    myScreenBitDepth = 24;
    myVisinfo = {};
    if (!XMatchVisualInfo(myDisplay, myDefaultScreen, myScreenBitDepth,
                          TrueColor, &myVisinfo)) {
        throw runtime_error("No matching visual info\n");
    }
}

void screen::setWindowAttrs() {
    myWindowAttr;
    myWindowAttr.bit_gravity = StaticGravity;
    myWindowAttr.background_pixel = 0xFFFFFF;
    myWindowAttr.colormap =
        XCreateColormap(myDisplay, myRoot, myVisinfo.visual, AllocNone);
    myWindowAttr.event_mask =
        StructureNotifyMask | KeyPressMask | KeyReleaseMask;

    unsigned long attributeMask =
        CWBitGravity | CWBackPixel | CWColormap | CWEventMask;

    myWindow = XCreateWindow(myDisplay, myRoot, 0, 0, myWidth, myHeight, 0,
                             myVisinfo.depth, InputOutput, myVisinfo.visual,
                             attributeMask, &myWindowAttr);
    

    if (!myWindow) {
        throw runtime_error("Window wasn't created properly\n");
    }

}

void screen::finalize(string name) {
    XStoreName(myDisplay, myWindow, name.c_str());

    setSizeHint(400, 300, 0, 0);
    setupInput();

    XMapWindow(myDisplay, myWindow);
    XFlush(myDisplay);

    myPixelBits = 32;
    myPixelBytes = myPixelBits / 8;
    myWindowBufferSize = myWidth * myHeight * myPixelBytes;
    myMem = (char*)malloc(myWindowBufferSize);

    myXWindowBuffer =
        XCreateImage(myDisplay, myVisinfo.visual, myVisinfo.depth, ZPixmap, 0,
                     myMem, myWidth, myHeight, myPixelBits, 0);
    myDefaultGC = DefaultGC(myDisplay, myDefaultScreen);
    myActiveGC = DefaultGC(myDisplay, myDefaultScreen);

    myConnectionNumber = ConnectionNumber(myDisplay);


    myWM_DELETE_WINDOW = XInternAtom(myDisplay, "WM_DELETE_WINDOW", False);
    if (!XSetWMProtocols(myDisplay, myWindow, &myWM_DELETE_WINDOW, 1)) {
        throw runtime_error("Couldn't register WM_DELETE_WINDOW property\n");
    }


    sizeChange = false;
    windowOpen = true;
}

void screen::setupInput() {
    myXInputMethod = XOpenIM(myDisplay, 0, 0, 0);
    if (!myXInputMethod) {
        throw runtime_error("Input Method could not be opened\n");
    }

    myStyles = 0;
    if (XGetIMValues(myXInputMethod, XNQueryInputStyle, &myStyles, NULL) ||
        !myStyles) {
        throw runtime_error("Input Styles could not be retrieved\n");
    }

    myBestMatchStyle = 0;
    for (int i = 0; i < myStyles->count_styles; i++) {
        XIMStyle thisStyle = myStyles->supported_styles[i];
        if (thisStyle == (XIMPreeditNothing | XIMStatusNothing)) {
            myBestMatchStyle = thisStyle;
            break;
        }
    }
    XFree(myStyles);

    if (!myBestMatchStyle) {
        throw runtime_error("No matching input style could be determined\n");
    }

    myXInputContext =
        XCreateIC(myXInputMethod, XNInputStyle, myBestMatchStyle,
                  XNClientWindow, myWindow, XNFocusWindow, myWindow, NULL);
    if (!myXInputContext) {
        throw runtime_error("Input Context could not be created\n");
    }
}

void screen::setSizeHint(int minWidth, int minHeight, int maxWidth,
                         int maxHeight) {
    XSizeHints hints = {};
    if (minWidth > 0 && minHeight > 0) {
        hints.flags |= PMinSize;
    }
    if (maxWidth > 0 && maxHeight > 0) {
        hints.flags |= PMaxSize;
    }

    hints.min_width = minWidth;
    hints.min_height = minHeight;
    hints.max_width = maxWidth;
    hints.max_height = maxHeight;

    XSetWMNormalHints(myDisplay, myWindow, &hints);
}

void screen::close() {
    windowOpen = false;
}

Status screen::toggleMaximize() {  
    XClientMessageEvent ev = {};
    Atom wmState = XInternAtom(myDisplay, "_NET_WM_STATE", false);
    Atom maxH  =  XInternAtom(myDisplay, "_NET_WM_STATE_MAXIMIZED_HORZ", false);
    Atom maxV  =  XInternAtom(myDisplay, "_NET_WM_STATE_MAXIMIZED_VERT", false);

    if(wmState == None) return 0;

    ev.type = ClientMessage;
    ev.format = 32;
    ev.window = myWindow;
    ev.message_type = wmState;
    ev.data.l[0] = 2; // _NET_WM_STATE_TOGGLE 2 according to spec; Not defined in my headers
    ev.data.l[1] = maxH;
    ev.data.l[2] = maxV;
    ev.data.l[3] = 1;

    return XSendEvent(myDisplay, myRoot, false,
                      SubstructureNotifyMask,
                      (XEvent *)&ev);

}

void screen::handleEvents() {
    XEvent ev = {};
    while (XPending(myDisplay) > 0) {
        XNextEvent(myDisplay, &ev);
        switch (ev.type) {
            case DestroyNotify: {
                XDestroyWindowEvent* e = (XDestroyWindowEvent*)&ev;
                if (e->window == myWindow) {
                    windowOpen = false;
                }
            } break;

            case ClientMessage: {
                XClientMessageEvent* e = (XClientMessageEvent*)&ev;
                if ((Atom)e->data.l[0] == myWM_DELETE_WINDOW) {
                    XDestroyWindow(myDisplay, myWindow);
                    close();
                }
            } break;

            case ConfigureNotify: {
                XConfigureEvent* e = (XConfigureEvent*)&ev;
                myWidth = e->width;
                myHeight = e->height;
                sizeChange = true;
            } break;

            case KeyPress: {
                XKeyPressedEvent* e = (XKeyPressedEvent*)&ev;

                int symbol = 0;
                Status status = 0;
                Xutf8LookupString(myXInputContext, e, (char*)&symbol, 4, 0,
                                  &status);

                if (status == XBufferOverflow) {
                    // Should not happen since there are no utf-8 characters
                    // larger than 24bits
                    //But something to be aware of when used
                    // to directly write to a string buffer
                    throw runtime_error(
                        "Buffer overflow when trying to create keyboard symbol "
                        "map\n");
                } else if (status == XLookupChars) {
                    pressedKeys[symbol] = true;
                    newKeyUpdates.push(symbol);
                    keycodeMap[e->keycode] = symbol;
                } else if (e->state == 16) {
                    //arrow keys
                    switch (e->keycode) {
                        case 111: upArrowPressed = true; break;
                        case 113: leftArrowPressed = true; break;
                        case 114: rightArrowPressed = true; break;
                        case 116: downArrowPressed = true; break;
                    }
                }

                if(selectedInput != nullptr) {
                    inputBox *theInput = (inputBox*)selectedInput;
                    if(status == XLookupChars) {
                        if(!theInput->isShowing()) {break;}
                        if(isPrintable(symbol)) {
                            theInput->sendChar((char)symbol);
                        } else if (symbol == 13) { //enter, return, etc
                            theInput->submit();
                        } else if (symbol == 8 || symbol == 127)  { //backspace or delete
                            theInput->backSpace();
                        }
                    } else if(e->state == 16) {
                        switch (e->keycode) {
                            case 111: theInput->moveCursorUp(); break;
                            case 113: theInput->moveCursorLeft(); break;
                            case 114: theInput->moveCursorRight(); break;
                            case 116: theInput->moveCursorDown(); break;
                        }
                    }
                }
            } break;

            case KeyRelease: {
                // there is no way to get characters from key release events, so
                // I store the ascii representations mapped to the returned keycodes
                XKeyPressedEvent* e = (XKeyPressedEvent*)&ev;

                int reference = keycodeMap[e->keycode];
                if (reference > 0) {
                    pressedKeys[reference] = false;
                    newKeyUpdates.push(reference | 0b10000000);
                }

                if (e->state == 16) {
                    //arrow keys
                    switch (e->keycode) {
                        case 111: upArrowPressed = false; break;
                        case 113: leftArrowPressed = false; break;
                        case 114: rightArrowPressed = false; break;
                        case 116: downArrowPressed = false; break;
                    }
                }

            } break;
        }
    }
}

bool screen::isPrintable(int symbol) {
    if(symbol >= 32 && symbol <= 126) {
        return true;
    }
    return false;
}

void screen::updateMouseStatus() {
    if (!isOpen()) {
        return;
    }

    unsigned int mouseStatus;
    Window throwawayWin;
    Window throwawayChildWin;
    int throwawayX;
    int throwawayY;

    XQueryPointer(myDisplay, myWindow, &throwawayWin, &throwawayChildWin,
                      &throwawayX, &throwawayY, &mouseX, &mouseY,
                      &mouseStatus);
    
    mouseInScreen = (mouseX >= 0) && (mouseY >= 0) && (mouseX < myWidth) && (mouseY < myHeight);
    if (mouseInScreen) {
        // for (int i = 31; i >= 0; i--) {
        //     cout << ((mouseStatus >> i) & 1);
        // }
        // cout << endl;
        bool mdown = (mouseStatus & 0b100000000) > 0;
        firstFrameMouseClicked = mdown && !mouseClicked;
        mouseClicked = mdown;

        if(mouseStatus & 0b100000000000) {
            newScrollUpdates.push(true);
        }
        if(mouseStatus & 0b1000000000000) {
            newScrollUpdates.push(false);
        }

        if(firstFrameMouseClicked) {
            newMouseUpdates.push(mouseX);
            newMouseUpdates.push(mouseY);
            checkElementClicked();
        }
    }
}

void screen::checkElementClicked() {
    selectedInput = nullptr;
    for (int i = 0; i < myHeldElems; i++) {
        string theType = myElements[i]->myType;
        if (theType == "button") {
            button* tempElem = (button*)myElements[i];
            bool success = tempElem->checkClicked(mouseX, mouseY);
            if(success) {break;}
        } else if (theType == "inputBox") {
            inputBox* tempElem = (inputBox*)myElements[i];
            bool success = tempElem->checkClicked(mouseX, mouseY);
            if(success) {
                selectedInput = myElements[i];
                break;
            }
        }
    }
}

void screen::adjustSize() {
    if (sizeChange) {
        sizeChange = false;
        XDestroyImage(myXWindowBuffer);  // Frees the memory we malloced;
        myWindowBufferSize = myWidth * myHeight * myPixelBytes;
        myMem = (char*)malloc(myWindowBufferSize);

        myXWindowBuffer =
            XCreateImage(myDisplay, myVisinfo.visual, myVisinfo.depth, ZPixmap,
                         0, myMem, myWidth, myHeight, myPixelBits, 0);
    }
}

void screen::beforeDraw() {
    handleEvents();
    updateMouseStatus();
    adjustSize();
}

void screen::finishDraw() {
    if(!windowOpen) {return;}
    
    XSync(myDisplay, false);
    XFlush(myDisplay);
    XPutImage(myDisplay, myWindow, myDefaultGC, myXWindowBuffer, 0, 0, 0, 0,
              myWidth, myHeight);
}

bool screen::getKeyPressed(int keyCode) {
    return pressedKeys[keyCode];
} 

char screen::getKeyEvent() {
    char retVal = newKeyUpdates.front();
    newKeyUpdates.pop();
    return retVal;
}

void screen::getMouseEvent(int &outX, int &outY) {
    outX = newMouseUpdates.front();
    newMouseUpdates.pop();
    outY = newMouseUpdates.front();
    newMouseUpdates.pop();
}

bool screen::getScrollEvent() {
    bool retVal = newScrollUpdates.front();
    newScrollUpdates.pop();
    return retVal;
}

void screen::setStroke(int r, int g, int b) {
    doStroke = true;
    strokeR = r;
    strokeG = g;
    strokeB = b;
}

void screen::setStroke(int color) {
    doStroke = true;
    strokeR = color>>16;
    strokeG = (color>>8)%256;
    strokeB = color%256;
}

void screen::setStrokeWeight(int weight) {
    strokeWeight = weight;
}

void screen::noStroke() {
    doStroke = false;
}

void screen::setBGcolor(int color) {
    doBackground = true;
    BGR = color>>16;
    BGG = (color>>8)%256;
    BGB = color%256;
}

void screen::setBGcolor(int r, int g, int b) {
    doBackground = true;
    BGR = r;
    BGG = g;
    BGB = b;
}

void screen::noBG() {
    doBackground = false;
}

void screen::setFontSize(int fontSize) {
    myFontSize = fontSize;
}

void screen::refreshFontNames() {
    for(int i = 0; i < myKnownFontCount; i++) {
        myFontNames[i] = myFonts[i]->getFontName();
    }
}

void screen::setFont(string fontName) {
    refreshFontNames();
    for(int i = 0; i < myKnownFontCount; i++) {
        if(fontName == myFontNames[i]) {
            myTextHandler = myFonts[i];
            return;
        }
    }
    throw runtime_error("Error: couldn't recognize font name (" + fontName + ")");
}

textHandler *screen::getTextHandler(string fontName) {
    for(int i = 0; i < myKnownFontCount; i++) {
        if(fontName == myFontNames[i]) {
            return myFonts[i];
        }
    }
    throw runtime_error("Error: couldn't recognize font name (" + fontName + ")");
}

string *screen::getListOfFonts(int &retFontCount) {
    retFontCount = myKnownFontCount;
    return myFontNames;
}

void screen::setFill(int r, int g, int b) {
    doFill = true;
    fillR = r;
    fillG = g;
    fillB = b;
}

void screen::setFill(int color) {
    doFill = true;
    fillR = color >> 16;
    fillG = (color >> 8) % 256;
    fillB = color % 256;
}

void screen::noFill() {
    doFill = false;
}

void screen::drawBackground(int r, int g, int b) {
    drawBackground((r<<16)+(g<<8)+b);
}

void screen::drawBackground(int myColor) {
    if(!doBackground) {
        return;
    }
    setFill(BGR, BGG, BGB);
    noStroke();
    drawRect(0, 0, myWidth, myHeight);
}

void screen::drawBackground() {
    drawBackground(BGR, BGG, BGB);
}

void screen::drawRect(int x, int y, int width, int height) {
    if (width <= 0 || height <= 0 || x >= myWidth || y >= myHeight || !isOpen()) {
        return;
    }

    int realX = x;
    int realY = y;
    int realWidth = realX + width < myWidth ? width : myWidth - realX;
    int realHeight = realY + height < myHeight ? height : myHeight - realY;

    if(realX < 0) {
        realWidth += realX;
        realX = 0;
    }
    if(realY < 0) {
        realHeight += realY;
        realY = 0;
    }

    if(doFill) {
        unsigned int myColor = (fillR<<16) + (fillG<<8) + fillB;
        int pitch = myWidth*myPixelBytes;
        for(int i = realY; i < realY + realHeight; i++) {
            char* rowStart = myMem + (i*pitch);
            for(int j = realX; j < realX + realWidth; j++) {
                unsigned int* p = (unsigned int*) (rowStart + (j*myPixelBytes));
                *p = myColor;
            }
        }
    }

    if(doStroke) {
        int strokeOffset = (strokeWeight-1)/2;
        doStroke = false;
        int currR = fillR;
        int currG = fillG;
        int currB = fillB;
        setFill(strokeR, strokeG, strokeB);
        drawRect(realX-strokeOffset, realY-strokeOffset, realWidth+strokeWeight, strokeWeight);
        drawRect(realX-strokeOffset, realY+realHeight-strokeOffset, realWidth+strokeWeight, strokeWeight);
        drawRect(realX-strokeOffset, realY-strokeOffset, strokeWeight, realHeight+strokeWeight);
        drawRect(realX+realWidth-strokeOffset, realY-strokeOffset, strokeWeight, realHeight+strokeWeight);
        doStroke = true;
        fillR = currR;
        fillG = currG;
        fillB = currB;
    }
    //XFlush(myDisplay);
}

void screen::drawText(string text, int x, int y) {
    unsigned int myColor = (fillR<<16) + (fillG<<8) + fillB;
    int pitch = myWidth*myPixelBytes;
    int FONTDETAIL = myTextHandler->getFontDetail();
    int spacing = (myTextHandler->getCharSpacing() * myFontSize)/FONTDETAIL;


    int currX = x;
    int currY = y;
    for(int i = 0; i < text.length(); i++) {
        char currChar = text[i];
        if(currChar == '\n') {
            currY += spacing + myFontSize;
            currX = x;
            continue;
        } else if(currChar == ' ') {
            currX += (myTextHandler->getSpacewidth() * myFontSize / FONTDETAIL) + spacing;
        }

        if(!myTextHandler->hasChar(currChar)) {continue;}
        bool **charBitmap = myTextHandler->getChar(currChar);
        int negXOffset = myTextHandler->getCharLeadWhitespace(currChar);
        int widthOffset = myTextHandler->getCharEndWhitespace(currChar);

        int startX = currX - ((negXOffset*myFontSize)/FONTDETAIL);
        int endX = startX + myFontSize - ((myFontSize*widthOffset)/FONTDETAIL);
        int charFootprint = FONTDETAIL - negXOffset - widthOffset;

        for(int cx = startX; cx < endX; cx++) {
            if(cx < 0 || cx >= myWidth) {continue;}
            for(int cy = currY; cy < currY + myFontSize; cy++) {
                if(cy < 0 || cy >= myHeight) {continue;}
                int bitmapPosX = (cx-startX)*FONTDETAIL / myFontSize;
                int bitmapPosY = (cy-currY)*FONTDETAIL / myFontSize;
                if(charBitmap[bitmapPosY][bitmapPosX]) {
                    unsigned int* p = (unsigned int*) (myMem + (cy*pitch) + (cx*myPixelBytes));
                    *p = myColor;
                }
            }
        }

        currX += (charFootprint*myFontSize/FONTDETAIL) + spacing;
    }
}

int screen::getTextWidth(string text) {
    int FONTDETAIL = myTextHandler->getFontDetail();
    int longestLine = 0;
    int newLinePos = text.find("\n");
    while(newLinePos != string::npos) {
        string currLine = text.substr(0, newLinePos);
        int lineWidth = getTextLineWidth(currLine);
        if(lineWidth > longestLine) {longestLine = lineWidth;}
        text = text.substr(newLinePos+1);
        newLinePos = text.find("\n");
    }
    int lineWidth = getTextLineWidth(text);
    if(lineWidth > longestLine) {longestLine = lineWidth;}

    return longestLine;
}

int screen::getTextLineWidth(string text) {
    int FONTDETAIL = myTextHandler->getFontDetail();
    int spacing = (myTextHandler->getCharSpacing() * myFontSize) / FONTDETAIL; // gap size between characters
    int spaceWidth = (myTextHandler->getSpacewidth() * myFontSize / FONTDETAIL) + spacing; // spacebar gap size

    int length = 0;
    for(int i = 0; i < text.length(); i++) {
        char currChar = text[i];
        if(currChar == ' ') {length += spaceWidth; continue;}
        if(!myTextHandler->hasChar(currChar)) {continue;}

        int charXOffset = myTextHandler->getCharLeadWhitespace(currChar);
        int charWidthOffset = myTextHandler->getCharEndWhitespace(currChar);
        int charCols = FONTDETAIL - charXOffset - charWidthOffset;
        int charFootprint = charCols*myFontSize/FONTDETAIL;
        length += charFootprint;
        length += spacing;
    }
    if(length > 0) {
        length -= spacing;
    }
    return length;
}

int screen::getTextHeight(string text) {
    int FONTDETAIL = myTextHandler->getFontDetail();
    int newLineCount = 0;
    int newLinePos = text.find("\n");
    while(newLinePos != string::npos) {
        string currLine = text.substr(0, newLinePos);
        newLineCount++;
        text = text.substr(newLinePos+1);
        newLinePos = text.find("\n");
    }

    int height = getNewlineYDiff()*newLineCount + myFontSize;
    return height;
}

int screen::getNewlineYDiff() {
    int FONTDETAIL = myTextHandler->getFontDetail();
    int spacing = (myTextHandler->getCharSpacing() * myFontSize)/FONTDETAIL;
    return myFontSize + spacing;
}

void screen::addElement(screenElement* newElem) {
    // if (myHeldElems >= myElemCap) {
    //     screenElement* newElemList[myElemCap * 2];
    //     myElemCap *= 2;
    //     for (int i = 0; i < myHeldElems; i++) {
    //         newElemList[i] = myElements[i];
    //     }
    //     delete [] myElements;
    //     myElements = newElemList;
    // }
    myElements.push_back(newElem);
    myHeldElems++;
}

void screen::remElement(screenElement *elem) {
    for(vector<screenElement*>::iterator it = myElements.begin(); it != myElements.end(); it++) {
        if(*it == elem) {
            myElements.erase(it);
            myHeldElems--;
            return;
        }
    }
}

bool screen::hasElement(screenElement *elem) {
    for(int i = 0; i < myHeldElems; i++) {
        if(myElements[i] == elem) {
            return true;
        }
    }
    return false;
}

void screen::drawElements() {
    if(selectedInput != nullptr) {
        ((inputBox*)selectedInput)->incCursorFlashTimer();
    }
    for (int i = 0; i < myHeldElems; i++) {
        string theType = myElements[i]->myType;
        if (theType == "rectangle") {
            rectangle* tempElem = (rectangle*)myElements[i];
            tempElem->draw();
        }
        else if (theType == "textBox") {
            textBox* tempElem = (textBox*)myElements[i];
            tempElem->draw();
        }
        else if (theType == "button") {
            button* tempElem = (button*)myElements[i];
            tempElem->draw();
        }
        else if (theType == "inputBox") {
            inputBox* tempElem = (inputBox*)myElements[i];
            tempElem->draw();
        }
    }
}


void screen::updateStatus() {
    if(!windowOpen) {return;}

    XWindowAttributes myAttrs;
    Status stat = XGetWindowAttributes(myDisplay, myWindow, &myAttrs);

    if(stat) {
        myWidth = myAttrs.width;
        myHeight = myAttrs.height;
    }
}





























// helper classes/functions

// thanks HolyBlackCat from stack overflow for this framerate limiter
std::chrono::system_clock::time_point a = std::chrono::system_clock::now();
std::chrono::system_clock::time_point b = std::chrono::system_clock::now();

void waitForFrame(float frameMS) {
    a = std::chrono::system_clock::now();
    std::chrono::duration<double, std::milli> work_time = a - b;

    if (work_time.count() < frameMS) {
        std::chrono::duration<double, std::milli> delta_ms(frameMS -
                                                           work_time.count());
        auto delta_ms_duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
        std::this_thread::sleep_for(
            std::chrono::milliseconds(delta_ms_duration.count()));
    }

    b = std::chrono::system_clock::now();
    std::chrono::duration<double, std::milli> sleep_time = b - a;
}


screen globalScreen;
bool defScreen = false;
thread drawThread;

void doFrames() {
    int frames = 0;

    waitForFrame(20);

    while (globalScreen.isOpen()) {
        waitForFrame(20);
        
        globalScreen.beforeDraw();
        globalScreen.drawBackground();
        globalScreen.drawElements();
        globalScreen.finishDraw();

        globalScreen.updateStatus();
    }
}


screen* makeScreen() {
    if (defScreen) {
        return nullptr;
    }

    globalScreen = screen();
    globalScreen.initDisplay();
    globalScreen.matchVisualInfo();
    globalScreen.setWindowAttrs();
    globalScreen.finalize("My Screen");
    defScreen = true;

    drawThread = thread(doFrames);
    drawThread.detach();

    return &globalScreen;
}

screen* makeScreen(string name) {
    if (defScreen) {
        return nullptr;
    }

    globalScreen = screen();
    globalScreen.initDisplay();
    globalScreen.matchVisualInfo();
    globalScreen.setWindowAttrs();
    globalScreen.finalize(name);
    defScreen = true;

    drawThread = thread(doFrames);
    drawThread.detach();

    return &globalScreen;
}

screen* makeScreen(int width, int height) {
    if (defScreen) {
        return nullptr;
    }

    globalScreen = screen(width, height);
    globalScreen.initDisplay();
    globalScreen.matchVisualInfo();
    globalScreen.setWindowAttrs();
    globalScreen.finalize("My Screen");
    defScreen = true;

    drawThread = thread(doFrames);
    drawThread.detach();

    return &globalScreen;
}

screen* makeScreen(string name, int width, int height) {
    if (defScreen) {
        return nullptr;
    }

    globalScreen = screen(width, height);
    globalScreen.initDisplay();
    globalScreen.matchVisualInfo();
    globalScreen.setWindowAttrs();
    globalScreen.finalize(name);
    defScreen = true;

    drawThread = thread(doFrames);
    drawThread.detach();

    return &globalScreen;
}

void maximizeScreen() {
    globalScreen.toggleMaximize();
}

void setBackground(int color) {
    globalScreen.setBGcolor(color);
}

void setBackground(int r, int g, int b) {
    globalScreen.setBGcolor(r, g, b);
}

void noBackground() {
    globalScreen.noBG();
}


//---------------------------screenElement---------------------------
string screenElement::getType() {
    return myType;
}

void screenElement::draw() {}

bool screenElement::checkClick(int x, int y) {
    return false;
}

void screenElement::beClicked() {}



//---------------------------rectangle---------------------------

// constructors
// default
rectangle::rectangle() {
    myType = "rectangle";
    myX = 0;
    myY = 0;
    myWidth = 50;
    myHeight = 50;
    showing = true;
    myColor = 0;
    globalScreen.addElement(this);
}

// explicit value
rectangle::rectangle(int x, int y, int width, int height) {
    myType = "rectangle";
    myX = x;
    myY = y;
    myWidth = width;
    myHeight = height;
    showing = true;
    myColor = 0;
    globalScreen.addElement(this);
}


rectangle::~rectangle() {
    globalScreen.remElement(this);
}


void rectangle::redefine(int x, int y, int width, int height) {
    myX = x;
    myY = y;
    myWidth = width;
    myHeight = height;
    if(!globalScreen.hasElement(this)) {globalScreen.addElement(this);}
}

string rectangle::getType() {
    return myType;
}

// getters
int rectangle::getX() {
    return myX;
}

int rectangle::getY() {
    return myY;
}

int rectangle::getWidth() {
    return myWidth;
}

int rectangle::getHeight() {
    return myHeight;
}

int rectangle::getColor() {
    return myColor;
}

int rectangle::getRed() {
    return myColor >> 16;
}

int rectangle::getGreen() {
    return (myColor >> 8) % 256;
}

int rectangle::getBlue() {
    return myColor % 256;
}

bool rectangle::isShowing() {
    return showing;
}

// setters
void rectangle::setX(int x) {
    myX = x;
}

void rectangle::setY(int y) {
    myY = y;
}

void rectangle::setPos(int x, int y) {
    myX = x;
    myY = y;
}

void rectangle::setWidth(int width) {
    myWidth = width;
}

void rectangle::setHeight(int height) {
    myHeight = height;
}

void rectangle::setColor(int color) {
    myColor = color;
}

void rectangle::setColor(int r, int g, int b) {
    myColor = (r << 16) + (g << 8) + b;
}

void rectangle::setRed(int r) {
    myColor = (r << 16) + (myColor % (256 * 256));
}

void rectangle::setGreen(int g) {
    myColor = ((myColor >> 16) << 16) + (g << 8) + (myColor % 256);
}

void rectangle::setBlue(int b) {
    myColor = ((myColor >> 8) << 8) + b;
}

void rectangle::show() {
    showing = true;
}

void rectangle::hide() {
    showing = false;
}

void rectangle::draw() {
    if(!showing) {return;}
    globalScreen.setFill(myColor);
    globalScreen.noStroke();
    globalScreen.drawRect(myX, myY, myWidth, myHeight);
}





//---------------------------textBox---------------------------

// constructors
// default
textBox::textBox() {
    myType = "textBox";
    myX = 0;
    myY = 0;
    myTextColor = 0;
    myAlignment = "left";
    myFontName = "basic 10x10";
    myFontSize = 20;
    myText = "";
    showing = true;
    globalScreen.addElement(this);
}

// explicit value
textBox::textBox(string text) {
    myType = "textBox";
    myX = 0;
    myY = 0;
    myTextColor = 0;
    myAlignment = "left";
    myFontName = "basic 10x10";
    myFontSize = 20;
    myText = text;
    showing = true;
    globalScreen.addElement(this);
}

textBox::textBox(int x, int y) {
    myType = "textBox";
    myX = x;
    myY = y;
    myTextColor = 0;
    myAlignment = "left";
    myFontName = "basic 10x10";
    myFontSize = 20;
    myText = "";
    showing = true;
    globalScreen.addElement(this);
}

textBox::textBox(int x, int y, string text) {
    myType = "textBox";
    myX = x;
    myY = y;
    myTextColor = 0;
    myAlignment = "left";
    myFontName = "basic 10x10";
    myFontSize = 20;
    myText = text;
    showing = true;
    globalScreen.addElement(this);
}



textBox::~textBox() {
    globalScreen.remElement(this);
}


void textBox::redefine(int x, int y) {
    myX = x;
    myY = y;
    if(!globalScreen.hasElement(this)) {globalScreen.addElement(this);}
}

void textBox::redefine(int x, int y, string text) {
    myX = x;
    myY = y;
    myText = text;
    if(!globalScreen.hasElement(this)) {globalScreen.addElement(this);}
}

string textBox::getType() {
    return myType;
}


//getters
int textBox::getX() {
    return myX;
}

int textBox::getY() {
    return myY;
}

int textBox::getTextColor() {
    return myTextColor;
}

int textBox::getTextRed() {
    return myTextColor >> 16;
}

int textBox::getTextGreen() {
    return (myTextColor >> 8) % 256;
}

int textBox::getTextBlue() {
    return myTextColor % 256;
}

bool textBox::isShowing() {
    return showing;
}

string textBox::getAlignment() {
    return myAlignment;
}

string textBox::getText() {
    return myText;
}

int textBox::getFontSize() {
    return myFontSize;
}

string textBox::getFont() {
    return myFontName;
}


//setters
void textBox::setX(int x) {
    myX = x;
}

void textBox::setY(int y) {
    myY = y;
}

void textBox::setTextColor(int color) {
    myTextColor = color;
}

void textBox::setTextColor(int r, int g, int b) {
    myTextColor = (r << 16) + (g << 8) + b;
}

void textBox::setTextRed(int r) {
    myTextColor = (r << 16) + (myTextColor % (256 * 256));
}

void textBox::setTextGreen(int g) {
    myTextColor = ((myTextColor >> 16) << 16) + (g << 8) + (myTextColor % 256);
}

void textBox::setTextBlue(int b) {
    myTextColor = ((myTextColor >> 8) << 8) + b;
}

void textBox::setAlignment(string text) {
    if(text == "left" || text == "right" || text == "center") {
        myAlignment = text;
    } else {
        cout << "Caution! alignment given is not recognised (" << text << ")\n";
    }
}

void textBox::setText(string text) {
    myText = text;
}

void textBox::setFontSize(int fontSize) {
    myFontSize = fontSize;
}

void textBox::setFont(string fontName) {
    myFontName = fontName;
}

void textBox::show() {
    showing = true;
}

void textBox::hide() {
    showing = false;
}


void textBox::draw() {
    if(!showing) {return;}
    globalScreen.setFill(myTextColor);
    globalScreen.setFontSize(myFontSize);
    globalScreen.setFont(myFontName);

    string text = myText;
    int lineNum = 0;

    while(text.length() > 0) {
        string currLine;
        int newlinePos = text.find("\n");
        if(newlinePos == string::npos) {
            currLine = text;
            text = "";
        } else {
            currLine = text.substr(0, newlinePos);
            text = text.substr(newlinePos+1);
        }
        int xPos = myX;
        if(myAlignment == "center") {
            xPos -= globalScreen.getTextWidth(currLine)/2;
        } else if(myAlignment == "right") {
            xPos -= globalScreen.getTextWidth(currLine);
        }
        globalScreen.drawText(currLine, xPos, myY + (lineNum*globalScreen.getNewlineYDiff()));
        lineNum++;
    }
}




//---------------------------button---------------------------


//constructors
//default
button::button() {
    myType = "button";
    myX = 0;
    myY = 0;
    myWidth = 50;
    myHeight = 50;
    myColor = 256^3 - 1; // white
    myStrokeColor = 0;
    myStrokeWeight = 2;
    doingStroke = true;
    myTextColor = 0;
    myFontSize = 20;
    myFontName = "basic 10x10";
    myLabel = "";
    showing = true;
    beenClicked = false;
    globalScreen.addElement(this);
}

//explicit value
button::button(string label) {
    myType = "button";
    myX = 0;
    myY = 0;
    myWidth = 50;
    myHeight = 50;
    myColor = 256^3 - 1; // white
    myStrokeColor = 0;
    myStrokeWeight = 2;
    doingStroke = true;
    myTextColor = 0;
    myFontSize = 20;
    myFontName = "basic 10x10";
    myLabel = label;
    showing = true;
    beenClicked = false;
    globalScreen.addElement(this);
}

button::button(int x, int y, int width, int height) {
    myType = "button";
    myX = x;
    myY = y;
    myWidth = width;
    myHeight = height;
    myColor = 256^3 - 1; // white
    myStrokeColor = 0;
    myStrokeWeight = 2;
    doingStroke = true;
    myTextColor = 0;
    myFontSize = 20;
    myFontName = "basic 10x10";
    myLabel = "";
    showing = true;
    beenClicked = false;
    globalScreen.addElement(this);
}

button::button(int x, int y, int width, int height, string label) {
    myType = "button";
    myX = x;
    myY = y;
    myWidth = width;
    myHeight = height;
    myColor = 256^3 - 1; // white
    myStrokeColor = 0;
    myStrokeWeight = 2;
    doingStroke = true;
    myTextColor = 0;
    myFontSize = 20;
    myFontName = "basic 10x10";
    myLabel = label;
    showing = true;
    beenClicked = false;
    globalScreen.addElement(this);
}



button::~button() {
    globalScreen.remElement(this);
}


void button::redefine(int x, int y, int width, int height) {
    myX = x;
    myY = y;
    myWidth = width;
    myHeight = height;
    if(!globalScreen.hasElement(this)) {globalScreen.addElement(this);}
}

void button::redefine(int x, int y, int width, int height, string label) {
    myX = x;
    myY = y;
    myWidth = width;
    myHeight = height;
    myLabel = label;
    if(!globalScreen.hasElement(this)) {globalScreen.addElement(this);}
}

string button::getType() {
    return myType;
}

//getters
int button::getX() {
    return myX;
}

int button::getY() {
    return myY;
}

int button::getWidth() {
    return myWidth;
}

int button::getHeight() {
    return myHeight;
}

int button::getColor() {
    return myColor;
}

int button::getRed() {
    return myColor >> 16;
}

int button::getGreen() {
    return (myColor >> 8) % 256;
}

int button::getBlue() {
    return myColor % 256;
}

int button::getStrokeColor() {
    return myStrokeColor;
}

int button::getStrokeRed() {
    return myStrokeColor >> 16;
}

int button::getStrokeGreen() {
    return (myStrokeColor >> 8) % 256;
}

int button::getStrokeBlue() {
    return myStrokeColor % 256;
}

int button::getStrokeWeight() {
    return myStrokeWeight;
}

bool button::getDoingStroke() {
    return doingStroke;
}

bool button::isShowing() {
    return showing;
}

int button::getTextColor() {
    return myTextColor;
}

int button::getTextRed() {
    return myTextColor >> 16;
}

int button::getTextGreen() {
    return (myTextColor >> 8) % 256;
}

int button::getTextBlue() {
    return myTextColor % 256;
}

string button::getText() {
    return myLabel;
}

int button::getFontSize() {
    return myFontSize;
}

string button::getFont() {
    return myFontName;
}
    
bool button::getClickedState() {
    bool retVal = beenClicked;
    beenClicked = false;
    cout.flush(); //for some reason, it only works when some interrupt is thrown in.
    return retVal;
}

//setters
void button::setX(int x) {
    myX = x;
}

void button::setY(int y) {
    myY = y;
}

void button::setWidth(int width) {
    myWidth = width;
}

void button::setHeight(int height) {
    myHeight = height;
}

void button::setColor(int color) {
    myColor = color;
}

void button::setColor(int r, int g, int b) {
    myColor = (r << 16) + (g << 8) + b;
}

void button::setRed(int r) {
    myColor = (r << 16) + (myColor % (256 * 256));
}

void button::setGreen(int g) {
    myColor = ((myColor >> 16) << 16) + (g << 8) + (myColor % 256);
}

void button::setBlue(int b) {
    myColor = ((myColor >> 8) << 8) + b;
}

void button::setStrokeColor(int color) {
    myStrokeColor = color;
}

void button::setStrokeColor(int r, int g, int b) {
    myStrokeColor = (r << 16) + (g << 8) + b;
}

void button::setStrokeRed(int r) {
    myStrokeColor = (r << 16) + (myStrokeColor % (256 * 256));
}

void button::setStrokeGreen(int g) {
    myStrokeColor = ((myStrokeColor >> 16) << 16) + (g << 8) + (myStrokeColor % 256);
}

void button::setStrokeBlue(int b) {
    myStrokeColor = ((myStrokeColor >> 8) << 8) + b;
}

void button::setStrokeWeight(int weight) {
    myStrokeWeight = weight;
    doingStroke = true;
}

void button::noStroke() {
    doingStroke = false;
}

void button::setTextColor(int color) {
    myTextColor = color;
}

void button::setTextColor(int r, int g, int b) {
    myTextColor = (r << 16) + (g << 8) + b;
}

void button::setTextRed(int r) {
    myTextColor = (r << 16) + (myTextColor % (256 * 256));
}

void button::setTextGreen(int g) {
    myTextColor = ((myTextColor >> 16) << 16) + (g << 8) + (myTextColor % 256);
}

void button::setTextBlue(int b) {
    myTextColor = ((myTextColor >> 8) << 8) + b;
}

void button::setText(string text) {
    myLabel = text;
}

void button::setFontSize(int fontSize) {
    myFontSize = fontSize;
}

void button::setFont(string fontName) {
    myFontName = fontName;
}

int button::checkClicked(int mouseX, int mouseY) {
    if(!showing) {return false;}
    bool inX = mouseX >= myX && mouseX <= myX + myWidth;
    bool inY = mouseY >= myY && mouseY <= myY + myHeight;
    bool success = inX && inY;
    if(success) {
        beenClicked = true;
        cout.flush(); //for some reason, it only works if some interrupt is thrown in
    }
    return success;
}

void button::show() {
    showing = true;
}

void button::hide() {
    showing = false;
}

void button::draw() {
    if(!showing) {return;}
    globalScreen.setFill(myColor);
    if(doingStroke) {
        globalScreen.setStrokeWeight(myStrokeWeight);
        globalScreen.setStroke(myStrokeColor);
    } else {
        globalScreen.noStroke();
    }
    globalScreen.drawRect(myX, myY, myWidth, myHeight);

    globalScreen.setFill(myTextColor);
    globalScreen.setFontSize(myFontSize);
    globalScreen.setFont(myFontName);
    int xPos = myX + (myWidth/2) - globalScreen.getTextWidth(myLabel)/2;
    int yPos = myY + (myHeight/2) - globalScreen.getTextHeight(myLabel)/2;
    globalScreen.drawText(myLabel, xPos, yPos);
}










//---------------------------input box---------------------------



//constructors
//default
inputBox::inputBox() {
    myType = "inputBox";
    myX = 0;
    myY = 0;
    myWidth = 200;
    myHeight = 50;
    myHintColor = (200)*(256*256) + (200)*(256) + 200; //light gray
    myColor = 256^3 - 1; //white
    myTextColor = 0; //black
    myFontSize = 20;
    myAlignment = "left";
    myFontName = "basic 10x10";
    isMultiLine = false;
    showing = true;
    isSubmitted = false;
    isSelected = false;
    mySubmittedText = "";
    myHint = "";
    myText = "";
    submitClearsText = false;
    cursorPos = 0;
    globalScreen.addElement(this);
}

//explicit value
inputBox::inputBox(string textHint) {
    myType = "inputBox";
    myX = 0;
    myY = 0;
    myWidth = 200;
    myHeight = 50;
    myHintColor = (200)*(256*256) + (200)*(256) + 200; //light gray
    myColor = 256^3 - 1; //white
    myTextColor = 0; //black
    myFontSize = 20;
    myAlignment = "left";
    myFontName = "basic 10x10";
    isMultiLine = false;
    showing = true;
    isSubmitted = false;
    isSelected = false;
    mySubmittedText = "";
    myHint = textHint;
    myText = "";
    submitClearsText = false;
    cursorPos = 0;
    globalScreen.addElement(this);
}

inputBox::inputBox(int x, int y, int width, int height) {
    myType = "inputBox";
    myX = x;
    myY = y;
    myWidth = width;
    myHeight = height;
    myHintColor = (200)*(256*256) + (200)*(256) + 200; //light gray
    myColor = 256^3 - 1; //white
    myTextColor = 0; //black
    myFontSize = 20;
    myAlignment = "left";
    myFontName = "basic 10x10";
    isMultiLine = false;
    showing = true;
    isSubmitted = false;
    isSelected = false;
    mySubmittedText = "";
    myHint = "";
    myText = "";
    submitClearsText = false;
    cursorPos = 0;
    globalScreen.addElement(this);
}

inputBox::inputBox(int x, int y, int width, int height, string textHint) {
    myType = "inputBox";
    myX = x;
    myY = y;
    myWidth = width;
    myHeight = height;
    myHintColor = (200)*(256*256) + (200)*(256) + 200; //light gray
    myColor = 256^3 - 1; //white
    myTextColor = 0; //black
    myFontSize = 20;
    myAlignment = "left";
    myFontName = "basic 10x10";
    isMultiLine = false;
    showing = true;
    isSubmitted = false;
    isSelected = false;
    mySubmittedText = "";
    myHint = textHint;
    myText = "";
    submitClearsText = false;
    cursorPos = 0;
    globalScreen.addElement(this);
}



inputBox::~inputBox() {
    globalScreen.remElement(this);
}


void inputBox::redefine(int x, int y, int width, int height) {
    myX = x;
    myY = y;
    myWidth = width;
    myHeight = height;
    if(!globalScreen.hasElement(this)) {globalScreen.addElement(this);}
}

void inputBox::redefine(int x, int y, int width, int height, string textHint) {
    myX = x;
    myY = y;
    myWidth = width;
    myHeight = height;
    myHint = textHint;
    if(!globalScreen.hasElement(this)) {globalScreen.addElement(this);}
}

string inputBox::getType() {
    return myType;
}

//getters
int inputBox::getX() {
    return myX;
}

int inputBox::getY() {
    return myY;
}

int inputBox::getWidth() {
    return myWidth;
}

int inputBox::getHeight() {
    return myHeight;
}

int inputBox::getColor() {
    return myColor;
}

int inputBox::getRed() {
    return myColor >> 16;
}

int inputBox::getGreen() {
    return (myColor >> 8) % 256;
}

int inputBox::getBlue() {
    return myColor % 256;
}

int inputBox::getTextColor() {
    return myTextColor;
}

int inputBox::getTextRed() {
    return myTextColor >> 16;
}

int inputBox::getTextGreen() {
    return (myTextColor >> 8) % 256;
}

int inputBox::getTextBlue() {
    return myTextColor % 256;
}

int inputBox::getHintColor() {
    return myTextColor;
}

int inputBox::getHintRed() {
    return myTextColor >> 16;
}

int inputBox::getHintGreen() {
    return (myTextColor >> 8) % 256;
}

int inputBox::getHintBlue() {
    return myTextColor % 256;
}

bool inputBox::isShowing() {
    return showing;
}

string inputBox::getTypedText() {
    return myText;
}

string inputBox::getSubmittedText() {
    isSubmitted = false;
    return mySubmittedText;
}

bool inputBox::hasBeenSubmitted() {
    return isSubmitted;
}

string inputBox::getHint() {
    return myHint;
}

int inputBox::getFontSize() {
    return myFontSize;
}

string inputBox::getFont() {
    return myFontName;
}

string inputBox::getAlignment() {
    return myAlignment;
}


//setters
void inputBox::setX(int x) {
    myX = x;
}

void inputBox::setY(int y) {
    myY = y;
}

void inputBox::setWidth(int width) {
    myWidth = width;
}

void inputBox::setHeight(int height) {
    myHeight = height;
}

void inputBox::setColor(int color) {
    myColor = color;
}

void inputBox::setColor(int r, int g, int b) {
    myColor = (r << 16) + (g << 8) + b;
}
void inputBox::setRed(int r) {
    myColor = (r << 16) + (myColor % (256 * 256));
}

void inputBox::setGreen(int g) {
    myColor = ((myColor >> 16) << 16) + (g << 8) + (myColor % 256);
}

void inputBox::setBlue(int b) {
    myColor = ((myColor >> 8) << 8) + b;
}

void inputBox::setTextColor(int color) {
    myTextColor = color;
}

void inputBox::setTextColor(int r, int g, int b) {
    myTextColor = (r << 16) + (g << 8) + b;
}

void inputBox::setTextRed(int r) {
    myTextColor = (r << 16) + (myTextColor % (256 * 256));
}

void inputBox::setTextGreen(int g) {
    myTextColor = ((myTextColor >> 16) << 16) + (g << 8) + (myTextColor % 256);
}

void inputBox::setTextBlue(int b) {
    myTextColor = ((myTextColor >> 8) << 8) + b;
}

void inputBox::setHintColor(int color) {
    myHintColor = color;
}

void inputBox::setHintColor(int r, int g, int b) {
    myHintColor = (r << 16) + (g << 8) + b;
}

void inputBox::setHintRed(int r) {
    myHintColor = (r << 16) + (myHintColor % (256 * 256));
}

void inputBox::setHintGreen(int g) {
    myHintColor = ((myHintColor >> 16) << 16) + (g << 8) + (myHintColor % 256);
}

void inputBox::setHintBlue(int b) {
    myHintColor = ((myHintColor >> 8) << 8) + b;
}

void inputBox::setHint(string newHint) {
    myHint = newHint;
}

void inputBox::setFontSize(int fontSize) {
    myFontSize = fontSize;
}

void inputBox::setFont(string fontName) {
    myFontName = fontName;
}

void inputBox::setAlignment(string alignment) {
    myAlignment = alignment;
}

void inputBox::setText(string text) {
    myText = text;
}

void inputBox::clearTextOnSubmit(bool clears) {
    submitClearsText = clears;
}

void inputBox::multiLine(bool multiLined) {
    isMultiLine = multiLined;
}

void inputBox::show() {
    showing = true;
}

void inputBox::hide() {
    showing = false;
}

bool inputBox::checkClicked(int mouseX, int mouseY) {
    if(!showing) {deselect(); return false;}
    bool inX = mouseX >= myX && mouseX <= myX + myWidth;
    bool inY = mouseY >= myY && mouseY <= myY + myHeight;
    bool success = inX && inY;
    if(success) {
        select();
    } else {
        deselect();
    }
    return success;
}

void inputBox::backSpace() {
    if (cursorPos > 0) {
        myText = myText.substr(0, cursorPos-1) + myText.substr(cursorPos);
        cursorPos--;
    }
    cursorFlashTimer = 0;
}

void inputBox::submit() {
    mySubmittedText = myText;
    if(submitClearsText) {myText = ""; cursorPos = 0;}
    else if (isMultiLine) {sendChar('\n');} //if is a multiline box and submitting doesnt's clear text, enter should add a newline.
    isSubmitted = true;
}

void inputBox::sendChar(char c) {
    myText = myText.substr(0, cursorPos) + c + myText.substr(cursorPos);
    cursorFlashTimer = 0;
    cursorPos++;
}

void inputBox::clearInput() {
    myText = "";
    cursorPos = 0;
}

int inputBox::find2PrevNewline() {
    int searchPos = cursorPos;
    int remLines = 2;
    while(searchPos > 0) {
        searchPos--;
        if(myText[searchPos] == '\n') {
            remLines--;
            if(remLines <= 0) return searchPos;
        }
    }
    return -1;
}

int inputBox::findPrevNewline() {
    int searchPos = cursorPos;
    while(searchPos > 0) {
        searchPos--;
        if(myText[searchPos] == '\n') {
            return searchPos;
        }
    }
    return -1;
}

int inputBox::findNextNewline() {
    int searchPos = cursorPos;
    while(searchPos < myText.length() - 1) {
        searchPos++;
        if(myText[searchPos] == '\n') {
            return searchPos;
        }
    }
    return -1;
}

int inputBox::findNext2Newline() {
    int searchPos = cursorPos;
    int remaining = 2;
    while(searchPos < myText.length() - 1) {
        searchPos++;
        if(myText[searchPos] == '\n') {
            remaining--;
            if(remaining <= 0) {return searchPos;}
        }
    }
    return -1;
}

int inputBox::findPosInLine() {
    int searchPos = cursorPos;
    while(searchPos > 0) {
        searchPos--;
        if(myText[searchPos] == '\n') {
            break;
        }
    }
    if(myText[searchPos] != '\n') {return cursorPos;}
    return cursorPos - searchPos;
}

void inputBox::moveCursorUp() {
    int prevNewlinePos = find2PrevNewline();
    int maxNewPos = findPrevNewline();
    int linePos = findPosInLine();
    bool secondLine = (linePos != cursorPos) && (prevNewlinePos == -1);
    if(secondLine) {
        prevNewlinePos = 0;
    }
    if(cursorPos - prevNewlinePos > (myWidth / myFontSize) || linePos == cursorPos && prevNewlinePos >= 0) {
        cursorPos -= (myWidth / myFontSize);
    } else if (prevNewlinePos == -1) {
        cursorPos = 0;
    } else {
        cursorPos = min(prevNewlinePos + linePos, maxNewPos-1);
    }
    cursorPos = max(cursorPos, 0);
    cursorFlashTimer = 0;
}

void inputBox::moveCursorDown() {
    int nextNewlinePos = findNextNewline();
    int maxNewPos = findNext2Newline();
    int linePos = findPosInLine();
    if(maxNewPos == -1) {
        maxNewPos = myText.length();
    }
    if(nextNewlinePos == -1) {
        cursorPos = myText.length();
    } else if(nextNewlinePos - cursorPos > (myWidth / myFontSize)) {
        cursorPos += (myWidth / myFontSize);
    } else {
        cursorPos = min(nextNewlinePos + linePos, maxNewPos-1);
    }
    cursorPos = min(cursorPos, (int) myText.length());
    cursorFlashTimer = 0;
}

void inputBox::moveCursorLeft() {
    cursorPos--;
    cursorPos = max(cursorPos, 0);
    cursorFlashTimer = 0;
}

void inputBox::moveCursorRight() {
    cursorPos++;
    cursorPos = min(cursorPos, (int) myText.length());
    cursorFlashTimer = 0;
}

void inputBox::select() {
    isSelected = true;
    cursorFlashTimer = 0;
    cursorPos = myText.length();
}

void inputBox::deselect() {
    isSelected = false;
}

void inputBox::incCursorFlashTimer() {
    cursorFlashTimer++;
}

void inputBox::draw() {
    if(!showing) {return;}
    //box
    globalScreen.setFill(myColor);
    globalScreen.setStroke(0, 0, 0);
    globalScreen.setStrokeWeight(4);
    globalScreen.drawRect(myX, myY, myWidth, myHeight);

    //text
    if(isMultiLine) {
        drawMultiLineText();
    } else {
        drawSingleLineText();
    }
}

void inputBox::drawSingleLineText() {
    globalScreen.setFontSize(myFontSize);
    globalScreen.setFont(myFontName);
    string displayText;
    //choose user's input or hint
    if(myText.length() > 0 || isSelected) {
        globalScreen.setFill(myTextColor);
        displayText = myText;
        //add flashing cursor if selected
        if(isSelected) {
            displayText = displayText.substr(0, cursorPos) + "_" + displayText.substr(cursorPos);
        }
    } else {
        globalScreen.setFill(myHintColor);
        displayText = myHint;
    }
    //limit text length to box size
    while (globalScreen.getTextWidth(displayText) >= myWidth - 16) {
        displayText = displayText.substr(1);
    }
    //calculate alignment offset
    int xPos = myX + 8;
    if(myAlignment == "center") {
        xPos = (myWidth - globalScreen.getTextWidth(displayText))/2;
    } else if(myAlignment == "right") {
        xPos = myWidth - globalScreen.getTextWidth(displayText) - 8;
    }
    //make the cursor flash
    if(cursorFlashTimer % 40 > 20 && isSelected) {
        displayText = displayText.substr(0, cursorPos) + " " + displayText.substr(cursorPos + 1);
    }

    int yPos = myY + (myHeight/2) - globalScreen.getTextHeight(displayText)/2;
    globalScreen.drawText(displayText, xPos, yPos);
}


void inputBox::drawMultiLineText() {
    globalScreen.setFontSize(myFontSize);
    globalScreen.setFont(myFontName);
    vector<string> displayLines;
    string remainingText;
    //choose user's input or hint
    if(myText.length() > 0 || isSelected) {
        globalScreen.setFill(myTextColor);
        remainingText = myText;
        //add flashing cursor if selected
        if(isSelected) {
            remainingText = remainingText.substr(0, cursorPos) + "_" + remainingText.substr(cursorPos);
        }
    } else {
        globalScreen.setFill(myHintColor);
        remainingText = myHint;
    }

    //limit text length to box size and split into lines
    while (remainingText.length() > 0) {
        string currLine = "";
        int nextWordPos = min(remainingText.find(" "), remainingText.find("\n"));
        bool hasNextWord = nextWordPos != string::npos;
        string currWord = hasNextWord ? remainingText.substr(0, nextWordPos) : remainingText;

        while(globalScreen.getTextWidth(currLine + " " + currWord) < myWidth - 16 && remainingText.length() > 0) {
            currLine += currWord + " ";
            char splitChar = remainingText[nextWordPos];
            remainingText = hasNextWord ? remainingText.substr(nextWordPos+1) : "";
            if(splitChar == '\n') {
                currLine += "\n";
                break; //newline found, don't put any more on this line
            }
            nextWordPos = min(remainingText.find(" "), remainingText.find("\n"));
            hasNextWord = nextWordPos != string::npos;
            currWord = hasNextWord ? remainingText.substr(0, nextWordPos) : remainingText;
        }

        //remove extra space
        if(currLine.length() > 0) {
            currLine = currLine.substr(0, currLine.length() - 1);
        }

        //word is too long to fit on one line, so take what can fit on this line
        if(currLine == "" && remainingText != "") {
            currLine = remainingText.substr(0, nextWordPos);
            while(globalScreen.getTextWidth(currLine) > myWidth - 16) {
                currLine = currLine.substr(0, currLine.length() - 1);
            }
            remainingText = remainingText.substr(currLine.length());
        }

        displayLines.push_back(currLine);
    }

    //find what line cursor is on
    int cursorLine = -1;
    int cursorSubPos;
    int remainingCharsToCursor = cursorPos;
    if(isSelected) {
        while(remainingCharsToCursor >= 0) {
            cursorLine++;
            remainingCharsToCursor -= displayLines[cursorLine].length();
        }
        cursorSubPos = displayLines[cursorLine].length() + remainingCharsToCursor;
    }

    //limit text height to box size
    string totalText = "";
    for(int i = 0; i < displayLines.size(); i++) {
        totalText += displayLines[i] + "\n";
    }
    if(displayLines.size() >= 1) {
        totalText = totalText.substr(0, totalText.length() - 1); //remove extra newline
    }
    //remove lines above cursor
    while(globalScreen.getTextHeight(totalText) > myHeight - 16 && totalText.find("\n") != string::npos && cursorLine > 0) {
        totalText = totalText.substr(totalText.find("\n") + 1);
        displayLines.erase(displayLines.begin());
        cursorLine--;
    }
    //remove lines below cursor
    while(globalScreen.getTextHeight(totalText) > myHeight - 16 && totalText.find("\n") != string::npos) {
        totalText = totalText.substr(0, totalText.rfind("\n"));
        displayLines.pop_back();
    }

    //draw remaining lines
    int currDrawYPos = myY + 8;
    for(int i = 0; i < displayLines.size(); i++) {
        string currLine = displayLines[i];

        //calculate alignment offset
        int xPos = myX + 8;
        if(myAlignment == "center") {
            xPos = (myWidth - globalScreen.getTextWidth(currLine))/2;
        } else if(myAlignment == "right") {
            xPos = myWidth - globalScreen.getTextWidth(currLine) - 8;
        }

        //make the cursor flash
        if(i == cursorLine && cursorFlashTimer % 40 > 20 && isSelected) {
            int firstHalfLength = globalScreen.getTextWidth(currLine.substr(0, cursorSubPos+1));
            textHandler *currHandler = globalScreen.getTextHandler(myFontName);
            int spacing = currHandler->getCharSpacing()*myFontSize / currHandler->getFontDetail();
            int secondHalfX = xPos + firstHalfLength + spacing;
            globalScreen.drawText(currLine.substr(cursorSubPos+1), secondHalfX, currDrawYPos);
            currLine = currLine.substr(0, cursorSubPos);
        }

        globalScreen.drawText(currLine, xPos, currDrawYPos);

        currDrawYPos += globalScreen.getNewlineYDiff();
    }
}

