
#include <iostream>
#include "menu.h"
#include "screen.h" //for waitForFrame()

using namespace std;

int main() {
    interface myInterface;

    waitForFrame(20);

    myInterface.changeMenu(0);

    while(myInterface.isOpen()) {
        waitForFrame(myInterface.getSleepTime());
        myInterface.tickMenu();
    }

    return 0;
}