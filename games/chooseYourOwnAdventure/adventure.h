
#include <iostream>
#include <fstream>
#include <map>
#include <vector>

using namespace std;


void trim(string &s) {
    s.erase(s.find_last_not_of(" \n\r\t")+1);
}


struct block {
    string name;
    string data;
};

vector<block> readBlocks(string data, string *blockNames, int blockTypes) {
    vector<block> retBlocks;

    string currBlock = "none";
    int nextNewlinePos;
    while (data.length() > 0) {
        //read next line
        nextNewlinePos = data.find("\n");
        if(nextNewlinePos == string::npos) {
            nextNewlinePos = data.length();
        }
        string nextLine = data.substr(0, nextNewlinePos);
        data = (data.length() != nextNewlinePos) ? data.substr(nextNewlinePos + 1) : "";

        //check for labels
        bool foundLabel = false;
        for(int i = 0; i < blockTypes; i++) {
            if(nextLine == blockNames[i]) {
                block newBlock;
                newBlock.name = blockNames[i];
                retBlocks.push_back(newBlock);
                foundLabel = true;
                break;
            }
        }
        if(foundLabel) {continue;}

        //add line to current block if it exists
        if(retBlocks.size() == 0) {continue;}
        retBlocks.back().data += nextLine + "\n";
    }

    //remove extra newlines
    for(int i = 0; i < retBlocks.size(); i++) {
        trim(retBlocks[i].data);
    }

    return retBlocks;
}


struct choice {
    choice(string data) {
        string labels[2] = {"----body----", "----leads----"};
        vector<block> blocks = readBlocks(data, labels, 2);

        for(int i = 0; i < blocks.size(); i++) {
            string type = blocks[i].name;
            string data = blocks[i].data;
            if(type == "----body----") {
                name += data + "\n";
            } else if(type == "----leads----") {
                destEvent += data + "\n";
            }
        }

        trim(name);
        trim(destEvent);
    };
    choice() {};
    string name;
    string destEvent;
};

struct event {
    event(string data, bool debug = false) {
        string labels[5] = {"----name----", "----story----", "----choice----", "----auto-leads----", "----wipe-screen----"};
        vector<block> blocks = readBlocks(data, labels, 5);

        for(int i = 0; i < blocks.size(); i++) {
            string type = blocks[i].name;
            string data = blocks[i].data;
            if(type == "----name----") {
                name += data + "\n";
            } else if(type == "----story----") {
                story += data + "\n";
            } else if(type == "----choice----") {
                choices.push_back(choice(data));
            } else if(type == "----auto-leads----") {
                forceDest += data + "\n";
            } else if(type == "----wipe-screen----") {
                clearsScreen = true;
            }
        }

        trim(name);
        trim(story);
        trim(forceDest);

        if(debug) {
            if(name != "") {
                cout << "\033[1;36mEvent found with name: \"\033[35m" << name << "\033[36m\"\033[0m\n";
            } else {
                cout << "\033[1;36mEvent found with missing/incorrect name block\033[0m\n";
            }
        }
    };
    event() {};
    string name;
    string story;
    string forceDest;
    bool clearsScreen = false;
    vector<choice> choices;
};





struct adventure {
    adventure(string fileName, bool debug = false) {
        //read data
        string data = "";
        ifstream adventureData("adventures/" + fileName);
        string line;
        while(getline(adventureData, line)) {
            data += line + "\n";
        }
        trim(data);
        
        //interpret data
        string labels[2] = {"----title----", "----event----"};
        vector<block> blocks = readBlocks(data, labels, 2);

        for(int i = 0; i < blocks.size(); i++) {
            string type = blocks[i].name;
            string data = blocks[i].data;
            if(type == "----title----") {
                name += data + "\n";
            } else if(type == "----event----") {
                event newEvent(data, debug);
                events[newEvent.name] = newEvent;
            }
        }

        trim(name);
    }
    event getEvent(string eventName, bool defaultToStart) {
        if(events.count(eventName) == 1) {
            return events[eventName];
        } else if(!defaultToStart) {
            cout << "\n\033[1;31mError: \"\033[31m" + eventName + "\033[31m\" does not exist.\nAsk the author to finish this adventure\033[0m\n\n";
            throw runtime_error("Event does not exist (" + eventName + ")");
        } else {
            if (events.count("error") == 1) {
                return events["error"];
            } else {
                cout << "\n\033[1;33mWarning: program running with noCrash enabled, but story\nfile does not have an \"error\" event.\nDefaulting to event \"start\"\033[0m\n\n";
                return events["start"];
            }
        }
    }
    map<string, event> events;
    string name;
};