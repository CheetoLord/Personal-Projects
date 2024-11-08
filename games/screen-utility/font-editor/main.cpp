
#include <iostream>
#include "screen.h"
#include "menu.h"

using namespace std;

int main() {

    interface myMenu;

    while(myMenu.isOpen()) {
        waitForFrame(20);
        myMenu.tickMenu();
    }

    return 0;
}