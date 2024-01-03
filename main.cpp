#include <filesystem>
#include <typeinfo>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include "json.hpp"
using namespace std;
using json = nlohmann::json;
using namespace std::filesystem;

class Room
{
public:
    string id;
    string description;
    unordered_map<string, string> exits;
};

class Object
{
public:
    string id;
    string description;
    string initialRoom;
};

class Player
{
public:
    string currentRoomId = "default"; // Will be set to "initialroom" in main when parsing map data
    vector<string> inventory;
    string objectiveType;
    vector<string> what; // Objective details.

    void checkObjective(const string &argument, json &mapData, Player &player)
    {
        /*checkObjective matches on of three objective types: collect,room and kill.
        i) collect: iterates through objective details and checks through player inventory to ensure all items are collected.
        ii)room: simply checks if objective room matches player's currentRoom.
        iii)kill: Operates on same logic as "collect". iterates through what vector and checks the specified enemy is dead.

        Potential improvements: better use of auto iterator to avoid looping to improve efficiency within "collect" and "kill".
        */

        if (player.objectiveType == "collect")
        {
            bool collectedItems = true;
            // match item to collect with player inventory.
            for (auto &i : player.what)
            { // search through objecitve details.

                collectedItems = false;
                for (auto &j : mapData["objects"])
                {
                    string objectID = j["id"];
                    // match objective element with object id
                    if (i == objectID)

                    {
                        string objectInitialRoom = j["initialroom"];
                        if (objectInitialRoom == "inventory") // check room was set to "inventory"
                        {
                            collectedItems = true;
                            break;
                        }
                    }
                    else
                    {
                        collectedItems = false;
                    }
                }
                if (!collectedItems)
                { // not all objects have been collected. Continue game.
                    // "Objective Check reached. Player hasn't collected all required objects yet. "
                    return;
                }
            }
            cout << "Success You completed the mission, which was to collect : ";
            bool firstItemFlag = true;
            for (auto &it : player.what)
            {
                if (firstItemFlag) // as long as not end of vector, append comma.
                {
                    cout << it;
                    firstItemFlag = false;
                }
                else
                {
                    cout << ", " << it;
                }
            }
            cout << endl;
            cout << "You Won!" << endl;
            exit(0);
        }

        else if (player.objectiveType == "room")
        {
            if (player.currentRoomId == player.what[0])
            {
                // success obtained, player's current room is the same as the desired room.
                cout << "Success! You completed the mission, which was to enter: " << player.what[0] << endl
                     << "You Won!" << endl;
                exit(0);
            }
            else
            {
                return;
            }
        }

        else if (player.objectiveType == "kill")
        {
            bool killedEnemies = true;
            // match enemy weakness with player inventory.
            for (auto &i : player.what)
            { // search through objecitve details.

                killedEnemies = false;
                for (auto &j : mapData["enemies"])
                {
                    string enemyID = j["id"];
                    // match objective element with enemy id
                    if (i == enemyID)

                    {
                        string enemyInitialRoom = j["initialroom"];
                        if (enemyInitialRoom == "dead") // check room was set to "dead"
                        {
                            killedEnemies = true;
                            break;
                        }
                    }
                    else
                    {
                        killedEnemies = false;
                    }
                }
                if (!killedEnemies)
                { // not all enemies have been killed. Continue game.
                    // "Objective Check reached. Player hasn't killed all required enemies yet."

                    return;
                }
            }
            if (killedEnemies)
            {
                cout << "Success You completed the mission, which was to kill : ";
                bool firstItemFlag = true;
                for (auto &it : player.what)
                {
                    if (firstItemFlag) // as long as not end of vector, append comma.
                    {
                        cout << it;
                        firstItemFlag = false;
                    }
                    else
                    {
                        cout << ", " << it;
                    }
                }
                cout << endl;
                cout << "You Won!" << endl;
                exit(0);
            }
        }
        else
        {
            cout << "No Objective Type found." << endl;
        }
    }

    // Look around the room and show any objects/enemies in current room.
    void
    lookAround(const string &argument, json &mapData, Player &player)
    {
        /* Look fulfils one of three conditions:
        i) look inside player inventory and return description of "item" (object within inv).
        ii) "look at object" when object is in same room as player.
        iii) "look at enemy" when enemy is in same room as player.
        iv) "General Loook function which returns room description and any enemies/objects in the room"
        called with: "look", "look around", "look [item/enemy/object]"
        */

        checkObjective(argument, mapData, player);

        string roomDescString, enemyDescString = "", objectDescString, enemyName, objectName;

        // Handles the look object scenario where object is in player inventory.
        bool isItemFlag = false;
        for (const auto &item : player.inventory)
        { // iterate through inventory, search for arg. return desc of "id".
            if (item == argument)
            {
                isItemFlag = true;
                for (const auto &object : mapData["objects"])
                {
                    if (object["id"] == item)
                    {
                        objectDescString = object["desc"];
                        cout << objectDescString << endl;

                        return;
                    }
                }
            }
        }

        // Special conditions, Look "arg" returns description, if "arg" == object || enemy.
        // Handles the look object scenario where object or enemy is in room.
        bool isObjectFlag = false;
        for (const auto &object : mapData["objects"])
        {
            if (object["id"] == argument)
            {
                isObjectFlag = true;
                if (object["initialroom"] == player.currentRoomId)
                {
                    objectDescString = object["desc"];
                    cout << objectDescString << endl; // You should only be able to see desc if you type "look gun" for example
                    return;
                }
                else
                {
                    // handle if object exists but not in room.
                    cout << "Object " << argument << " is not in this room" << endl;
                    return;
                }
            }
        }

        // enemy version of the above.
        bool isEnemyFlag = false;
        for (const auto &enemy : mapData["enemies"])
        {
            if (enemy["id"] == argument)
            {
                isEnemyFlag = true;
                if (enemy["initialroom"] == player.currentRoomId)
                {
                    enemyDescString = enemy["desc"];
                    cout << enemyDescString << endl;
                    return;
                }
                else
                {
                    // handle if object exists but not in room.
                    cout << "Enemy " << argument << " is not in this room" << endl;
                    return;
                }
            }
        }

        // check if item is; not in inventory, not an object and not an enemy.
        if (!isItemFlag && !isObjectFlag && !isEnemyFlag && argument != "")
        { // logic: should only be accessed if look arg is a non-empty field and not any of; object, enemy or item in inventory.
            cout << "I don't understand that." << endl;
            return;
        }

        // Finally, if none of those conditions were met, look into room and return; (i) room desc, (ii) objets desc and (iii) enemy name (OR try intro_msg too)
        for (const auto &room : mapData["rooms"])
        {
            if (room["id"] == currentRoomId)
            {
                auto roomDescIterator = room.find("desc");

                if (roomDescIterator != room.end())
                {
                    roomDescString = roomDescIterator->get<string>();
                }
                else
                { // if somehow player's room can't be found, execute this.
                    cout << "Unexpected Occurrence: Player's room desc not found." << endl;
                    roomDescString = "You are in an unkown room.";
                }

                cout << roomDescString << endl;
                break;
            }
        }

        for (const auto &object : mapData["objects"])
        {
            if (object["initialroom"] == currentRoomId)
            {
                objectName = object["id"];
                cout << "There is a " << objectName << endl;
            }
        }
        // Finally, list enemies in the room, try to find intro message.
        for (const auto &enemy : mapData["enemies"])
        {

            if (enemy["initialroom"] == player.currentRoomId)
            {
                string introMsg;
                auto introMsgIterator = enemy.find("intro_msg");

                string enemyName;
                if (introMsgIterator != enemy.end())
                {
                    introMsg = introMsgIterator->get<string>();
                }
                else
                {
                    enemyName = enemy.value("id", "strange enemy");
                    introMsg = "There is a " + enemyName + ".";
                }

                cout << introMsg << endl;

                return;
            }
        }
    }

    void move(const string &argument, json &mapData, Player &player)
    {
        /*move searches through map and updates player's current room to the desired exit value from the key-value pair.
        will first check if there is an enemy in the player's room. if enemy exists, random chance based on enemy aggression that player will suddenly die and end game.
        Called with: "move" and "go"
        */

        // check if enemy in room. if so, there is a chance the enemy will kill you.
        for (auto &enemy : mapData["enemies"])
        {
            if (enemy["initialroom"] == player.currentRoomId)
            {
                string failedEscapeMessage;
                int randint = rand() % 100;
                if (randint <= enemy["aggressiveness"])
                {
                    try
                    {
                        failedEscapeMessage = enemy["unsuccessful_escape_msg"].get<string>();
                    }
                    catch (const json::exception &e)
                    {
                        string enemyName = enemy["id"];
                        failedEscapeMessage = "You died trying to escape the " + enemyName + ".";
                    }
                    cout << failedEscapeMessage << endl
                         << "GAME OVER." << endl;
                    exit(0);
                }
            }
        }

        bool roomFound = false, exitFound = false;
        for (const auto &room : mapData["rooms"])
        { // match room and current room.
            if (room["id"] == currentRoomId)
            {
                roomFound = true;
                // Tried out iterator here, seems good but I'm not going back to rewrite all my old loops
                auto exits = room["exits"];
                auto exitIt = exits.find(argument);
                if (exitIt != exits.end())
                {
                    exitFound = true;
                    currentRoomId = exits[argument];
                    lookAround("", mapData, player);
                    return;
                }
                break;
            }
        }
        if (!roomFound || !exitFound)
        {
            cout << "I don't understand move " << argument << endl;
        }
    }

    void take(const string &argument, json &mapData, Player &player)
    {
        /*
        take searches for object, if within room, adds to player's inventory
        called with: "take", "pick", "grab"*/

        // Iterate through objects
        for (auto &object : mapData["objects"])
        {
            // Check if the object is in the player's current room and matches the argument
            if (object["id"] == argument)
            {
                if (object["initialroom"] == currentRoomId)
                {
                    // Push into player inventory
                    player.inventory.push_back(argument);

                    // Remove object by setting value to something unreachable.
                    object["initialroom"] = "inventory";

                    cout << "You took the " << argument << endl;
                    checkObjective(argument, mapData, player);
                    return;
                }
                else
                {
                    cout << argument << " not in this room. It can't be taken." << endl;

                    return;
                }
            }
        }
        cout << "I don't understand take " << argument << endl;
    }

    void listItems(const string &argument, json &mapData, Player &player)
    {
        /*prints items within player's inventory in format: [item, item, item].
        called with: "list items", "look inventory", "list inventory".
        */

        string strInventory;
        if (player.inventory.empty())
        {
            cout << "Inventory is empty." << endl;
            return;
        }
        else
        {
            bool firstItemFlag = true;
            for (const string &item : player.inventory)
            {
                if (firstItemFlag) // as long as not end of vector, append comma.
                {
                    strInventory += item;
                    firstItemFlag = false;
                }
                else
                { // Since initially set as true, skip to else, set as false, should never access this again.
                    strInventory += ", " + item;
                    // this literally only exists becuase I couldn't remember a neat way to handle vector items < end.
                }
            }
            cout << "Player Inventory: [" << strInventory << "]" << endl;

            return;
        }
    }

    void kill(const string &argument, json &mapData, Player &player)
    {
        /*
        check enemy is in room, check enemy weaknesses are matched with the player's inventory items. if so successful kill, if not, kill player.
        called with: "kill", "fight", "attack".
         */

        // Check enemy
        for (auto &enemyInstance : mapData["enemies"])
        {                                        // iterate through inventory
            if (enemyInstance["id"] == argument) // Check if arg exists
            {
                if (enemyInstance["initialroom"] == currentRoomId)
                {
                    bool winFightFlag = true;
                    // match enemy weakness with player inventory.
                    for (auto &i : enemyInstance["killedby"])
                    { // search through player inventory.
                        winFightFlag = false;
                        for (auto &j : player.inventory)
                        {
                            if (i == j)
                            {
                                winFightFlag = true;
                                break;
                            }
                            else
                            {
                                winFightFlag = false;
                            }
                        }
                        if (!winFightFlag)
                        {
                            string killMsg;
                            try
                            {
                                killMsg = enemyInstance["unsuccessful_kill_msg"].get<string>();
                            }
                            catch (const json::exception &e)
                            {
                                killMsg = "You failed to kill the " + argument + ". The " + argument + " rips you apart.";
                            }

                            cout << killMsg << endl;
                            cout << " GAME OVER." << endl;
                            exit(0);
                        }
                    }
                    // Remove enemyInstance by setting initialroom to "dead".
                    if (winFightFlag)
                    {
                        string killMsg;
                        try
                        {
                            killMsg = enemyInstance["successful_kill_msg"].get<string>();
                        }
                        catch (const json::exception &e)
                        {
                            killMsg = "You won your fight! You have slain the " + enemyInstance["id"].get<string>();
                        }
                        enemyInstance["initialroom"] = "dead";
                        cout << killMsg << endl;

                        checkObjective(argument, mapData, player);
                        return;
                    }
                }
                else
                {
                    cout << "This enemy is not in the same room as you." << endl;
                    return;
                }
            }
        }
        cout << "I don't understand " << argument << endl;
    }

    void failedInput()
    {
        cout << "Command not found. Try Again" << endl;
    }
};

class Enemy // I think all enemies need to be stored in a vector?
{
public:
    string id;
    string desc;
    int aggressiveness;
    string initialRoom;
    vector<string> weakness;
};

class Map // Contains all information from JSON Object.
{
public:
    // Will be set to initial room in main when parsing map data
    json jsonData;
    vector<Room> rooms;
    vector<Object> objects;
    vector<Enemy> enemyList;
    Player gamePlayer;
};

using CommandType = void (Player::*)(const string &, json &, Player &); // For my sanity when scrolling through the code

// main program for playing the game. It's essentially passing you from one while loop to another to get the neccessary data at each stage of gameplay.
int main()
{
    // playableMaps is a vector string storing all available .json files within the same folder.
    vector<string> playableMaps;
    int x = 1; // used for listing playable maps.
    cout << "Text Adventure Game" << endl;

    path directorypath = ".";
    // Get any available json file and return it as a numbered option
    string mapName;
    try
    {
        if (exists(directorypath) && is_directory(directorypath))
        {
            for (const auto &entry : directory_iterator(directorypath))
            {
                if (entry.path().extension() == ".json") // one last check to ensure only JSON files
                {
                    mapName = entry.path().string();
                    // Store filename in playableMaps
                    playableMaps.push_back(mapName);
                }
            }
        }

        else
        {
            // Handle the case where the directory doesn't exist
            cerr << "Directory not found." << endl;
        }
    }
    catch (const exception &e) // didn't specify if I'm using json exception or std...
    {
        cerr << e.what() << endl;
    }

    // Print avaliable map files.
    for (const auto &map : playableMaps)
    {
        cout << x << ": " << map.substr(2, map.size() - 7) << endl; //.substr is a tedious way to show the file to user in a clean way e.g "map1" instead of "./map1.json".
        x++;
    }

    json j; // object that represents the json data

    while (true)
    {
        cout << "Select one of the maps below using the numbers: ";
        int i;
        cin >> i;

        try
        {
            string selectedMap;
            selectedMap = playableMaps.at(i - 1);
            cout << "Your chosen map is: " << selectedMap.substr(2, selectedMap.size() - 7) << endl;
            selectedMap = selectedMap.substr(2);
            //.substr is a tedious way to show the file to user in a clean way "map1" instead of "./map1.json".
            ifstream fin(selectedMap);
            fin >> j; // pass selectedMap into json object.
            break;
        }
        catch (const out_of_range &e)
        {
            // Handle out-of-range exception
            cout << "Not a valid index, try again." << endl;
            // Clear input buffer.
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // added this to stop an issue I got when entering str would give endless loop.
        }
    }

    // Initialise all the neccessary entities.
    Map GameMap;

    GameMap.jsonData = j; //

    // Pass JSON rooms into c++ class

    for (const auto &jsonRoom : j["rooms"])
    {
        Room room;
        room.id = jsonRoom["id"];
        room.description = jsonRoom["desc"];

        for (const auto &exit : j["exits"].items())
        {
            room.exits[exit.key()] = exit.value();
        }
        GameMap.rooms.push_back(room);
    }

    // Pass JSON Objects into c++ class
    for (const auto &jsonObj : j["objects"])
    {
        Object object;
        object.id = jsonObj["id"];
        object.description = jsonObj["desc"];
        object.initialRoom = jsonObj["initialroom"];
        GameMap.objects.push_back(object);
    }

    // Pass JSON player into c++ class

    Player player;

    json::iterator it = j["player"].find("initialroom");

    if (it != j["player"].end())
    {
        // Found the "initialroom" key in the JSON data
        player.currentRoomId = it.value();
    }
    else
    {
        // "initialroom" key not found
        cerr << "Error: Player's initial room not found in player data." << endl;
    }

    // Nothing needed to be passed into inventory yet.

    // Pass in objective.
    player.objectiveType = j["objective"]["type"];
    for (auto &i : j["objective"]["what"])
    {
        player.what.push_back(i);
    }

    // Pass JSON enemies into c++ class
    for (const auto &EnemyInstance : j["enemies"])
    {
        Enemy enemy;
        enemy.id = EnemyInstance["id"];
        enemy.desc = EnemyInstance["desc"];
        enemy.aggressiveness = EnemyInstance["aggressiveness"];
        enemy.initialRoom = EnemyInstance["initialroom"];

        for (const auto &killedBy : EnemyInstance["killedby"])
        {
            enemy.weakness.push_back(killedBy);
        }

        GameMap.enemyList.push_back(enemy);
    }

    // "Game start"
    cout << endl;
    player.lookAround("", j, player);
    cout << endl
         << "> ";

    // Unorderded Map "commands"
    unordered_map<string, CommandType>
        commands = {
            // Multi-word commands are special commands which don't take in arguments once they're called.
            // Single word command is typically expected, followed with arg,
            {"take", &Player::take},
            {"pick", &Player::take},
            {"grab", &Player::take},
            {"look", &Player::lookAround},
            {"look around", &Player::lookAround},
            {"kill", &Player::kill},
            {"fight", &Player::kill},
            {"attack", &Player::kill},
            {"move", &Player::move},
            {"go", &Player::move},
            {"list items", &Player::listItems},
            {"look inventory", &Player::listItems},
            {"list inventory", &Player::listItems}};

    // Player Text Input Loop
    while (true)
    {

        string input, playercmd, argument;

        getline(cin, input);

        // ignore empty input
        if (input.empty())
        {
            continue;
        }

        // Check for multi-word commands
        if (commands.find(input) != commands.end())
        {
            playercmd = input;
        }
        else
        {
            stringstream ss(input);
            // format example: cmd: "Move" arg:"Left" ==> Move Left.
            ss >> playercmd;
            for (auto x : playercmd)
            {
                x = (char)tolower(x);
            }

            getline(ss >> ws, argument);
        }

        // Hard coded because a function isn't neccessary for this.
        if (playercmd == "quit" || playercmd == "exit")
        {
            cout << "Quitting game, goodbye." << endl;
            exit(0);
        }

        auto it = commands.find(playercmd);
        if (it != commands.end())
        {
            cout << endl;
            (player.*(it->second))(argument, j, player);
            cout << endl
                 << "> ";
        }
        else
        {
            cout << endl;
            // Unexpected input. Try again
            player.failedInput();
            cout << endl
                 << "> ";
        }
    }
}