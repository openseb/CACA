// caca.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <windows.h>  // Add this for Windows API functions
#include <map>
#include <algorithm>

// Define ItemType as constants
const int WEAPON = 0;
const int ARMOR = 1;
const int POTION = 2;

// Update the Item struct
struct Item {
    char name[50];
    int type;
    int value;
};

// Define a maximum number of items in the inventory
#define MAX_INVENTORY 10

// Update the Player struct
struct Player {
    char name[50];
    int health;
    int maxHealth;
    int attack;
    int level;
    int exp;
    int gold;
    Item equippedWeapon;
    Item equippedArmor;
    Item inventory[MAX_INVENTORY];
    int inventoryCount;
};

struct Position {
    int x;
    int y;
    Position() : x(0), y(0) {}
    Position(int _x, int _y) : x(_x), y(_y) {}
};

struct Enemy {
    Position pos;
};

// Color constants
const int COLOR_DEFAULT = 7;
const int COLOR_PLAYER = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
const int COLOR_TREE = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
const int COLOR_ROCK = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
const int COLOR_TELEPORT = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
const int COLOR_ENEMY = FOREGROUND_RED | FOREGROUND_INTENSITY;
const int COLOR_GROUND = FOREGROUND_INTENSITY; // Very dark gray

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void resetColor() {
    setColor(COLOR_DEFAULT);
}

class Map {
private:
    std::vector<std::vector<char> > grid; // Note the space between '>
    int width;
    int height;
    Position teleportA;
    Position teleportB;
    std::vector<Enemy> enemies;

public:
    Map(int w, int h) : width(w), height(h) {
        // Initialize grid
        grid.resize(height);
        for (int i = 0; i < height; ++i) {
            grid[i].resize(width, '.');
        }
        generateMap();
    }

    void generateMap() {
        int i, x, y;

        // Add trees
        for (i = 0; i < width * height / 10; ++i) {
            x = rand() % width;
            y = rand() % height;
            grid[y][x] = 'T';
        }

        // Add rocks
        for (i = 0; i < width * height / 20; ++i) {
            x = rand() % width;
            y = rand() % height;
            grid[y][x] = 'R';
        }

        // Add teleport cells
        teleportA.x = rand() % width;
        teleportA.y = rand() % height;
        teleportB.x = rand() % width;
        teleportB.y = rand() % height;
        grid[teleportA.y][teleportA.x] = 'A';
        grid[teleportB.y][teleportB.x] = 'B';

        // Add enemies
        for (i = 0; i < 3; ++i) {
            Enemy enemy;
            do {
                enemy.pos.x = rand() % width;
                enemy.pos.y = rand() % height;
            } while (grid[enemy.pos.y][enemy.pos.x] != '.');
            enemies.push_back(enemy);
        }
    }

    void display(const Position& playerPos) {
        int x, y, i;
        std::vector<std::vector<char> > displayGrid = grid;

        // Place enemies
        for (i = 0; i < enemies.size(); ++i) {
            displayGrid[enemies[i].pos.y][enemies[i].pos.x] = 'E';
        }

        for (y = 0; y < height; ++y) {
            for (x = 0; x < width; ++x) {
                if (x == playerPos.x && y == playerPos.y) {
                    setColor(COLOR_PLAYER);
                    std::cout << '@';
                } else {
                    switch (displayGrid[y][x]) {
                        case 'T':
                            setColor(COLOR_TREE);
                            break;
                        case 'R':
                            setColor(COLOR_ROCK);
                            break;
                        case 'A':
                        case 'B':
                            setColor(COLOR_TELEPORT);
                            break;
                        case 'E':
                            setColor(COLOR_ENEMY);
                            break;
                        default:
                            resetColor();
                            break;
                    }
                    std::cout << displayGrid[y][x];
                }
                resetColor();
            }
            std::cout << std::endl;
        }
    }

    bool isValidMove(const Position& pos) {
        return pos.x >= 0 && pos.x < width && pos.y >= 0 && pos.y < height
               && grid[pos.y][pos.x] != 'T' && grid[pos.y][pos.x] != 'R';
    }

    Position getTeleportDestination(const Position& pos) {
        if (pos.x == teleportA.x && pos.y == teleportA.y) return teleportB;
        if (pos.x == teleportB.x && pos.y == teleportB.y) return teleportA;
        return pos;
    }

    void moveEnemies() {
        int i, dx, dy;
        for (i = 0; i < enemies.size(); ++i) {
            do {
                dx = (rand() % 3) - 1;  // -1, 0, or 1
                dy = (rand() % 3) - 1;  // -1, 0, or 1
            } while (!isValidMove(Position(enemies[i].pos.x + dx, enemies[i].pos.y + dy)));

            enemies[i].pos.x += dx;
            enemies[i].pos.y += dy;
        }
    }

    bool isEnemyAt(const Position& pos) {
        int i;
        for (i = 0; i < enemies.size(); ++i) {
            if (enemies[i].pos.x == pos.x && enemies[i].pos.y == pos.y) {
                return true;
            }
        }
        return false;
    }

    void removeEnemy(const Position& pos) {
        for (int i = 0; i < enemies.size(); ++i) {
            if (enemies[i].pos.x == pos.x && enemies[i].pos.y == pos.y) {
                enemies.erase(enemies.begin() + i);
                break;
            }
        }
    }
};

void battle(Player& player, Map& map, const Position& enemyPos) {
    std::cout << "A wild monster appears!\n";
    int monsterHealth = 50;
    int monsterAttack = 5;

    while (player.health > 0 && monsterHealth > 0) {
        std::cout << "Your health: " << player.health << " Monster health: " << monsterHealth << "\n";
        std::cout << "Do you want to attack (a) or run (r)? ";
        char choice;
        std::cin >> choice;

        if (choice == 'a') {
            monsterHealth -= player.attack;
            std::cout << "You deal " << player.attack << " damage!\n";
            if (monsterHealth > 0) {
                player.health -= monsterAttack;
                std::cout << "Monster deals " << monsterAttack << " damage!\n";
            }
        } else if (choice == 'r') {
            std::cout << "You run away!\n";
            return;
        }
    }

    if (player.health <= 0) {
        std::cout << "Game Over! You have been defeated.\n";
    } else {
        int expGained = rand() % 20 + 10; // Random exp between 10 and 29
        int goldGained = rand() % 15 + 5; // Random gold between 5 and 19
        player.exp += expGained;
        player.gold += goldGained;
        std::cout << "You have defeated the monster!\n";
        std::cout << "You gained " << expGained << " exp and " << goldGained << " gold!\n";
        map.removeEnemy(enemyPos);

        // Simple leveling system
        if (player.exp >= player.level * 100) {
            player.level++;
            player.exp -= (player.level - 1) * 100;
            player.attack += 2;
            player.health += 10;
            std::cout << "Congratulations! You've reached level " << player.level << "!\n";
            std::cout << "Your attack increased by 2 and your max health increased by 10.\n";
        }
    }
    std::cout << "Press Enter to continue...";
    std::cin.ignore();
    std::cin.get();
}

// Add these functions after the Map class

void initializeItems(Item* items, int& itemCount) {
    strcpy(items[itemCount].name, "Sword");
    items[itemCount].type = WEAPON;
    items[itemCount].value = 5;
    itemCount++;

    strcpy(items[itemCount].name, "Shield");
    items[itemCount].type = ARMOR;
    items[itemCount].value = 3;
    itemCount++;

    strcpy(items[itemCount].name, "Health Potion");
    items[itemCount].type = POTION;
    items[itemCount].value = 20;
    itemCount++;
}

void displayInventory(const Player& player, const Item* items, int itemCount) {
    std::cout << "Equipped Weapon: " << player.equippedWeapon.name << " (Attack: " << player.equippedWeapon.value << ")\n";
    std::cout << "Equipped Armor: " << player.equippedArmor.name << " (Defense: " << player.equippedArmor.value << ")\n";
    std::cout << "Gold: " << player.gold << "\n";
    std::cout << "Inventory:\n";
    for (int i = 0; i < player.inventoryCount; i++) {
        std::cout << player.inventory[i].name << "\n";
    }
}

void useItem(Player& player, Item* items, int itemCount, const char* itemName) {
    for (int i = 0; i < player.inventoryCount; i++) {
        if (strcmp(player.inventory[i].name, itemName) == 0) {
            if (player.inventory[i].type == POTION) {
                // Replace std::min with a simple if statement
                if (player.health + player.inventory[i].value > player.maxHealth) {
                    player.health = player.maxHealth;
                } else {
                    player.health += player.inventory[i].value;
                }
                std::cout << "You used a " << itemName << " and restored " << player.inventory[i].value << " health.\n";
                // Remove the item from inventory
                for (int j = i; j < player.inventoryCount - 1; j++) {
                    player.inventory[j] = player.inventory[j + 1];
                }
                player.inventoryCount--;
                return;
            } else {
                std::cout << "You can't use this item directly.\n";
                return;
            }
        }
    }
    std::cout << "You don't have any " << itemName << " to use.\n";
}

void dropItem(Player& player, const char* itemName) {
    // Since we don't have an inventory system, we'll just print a message
    std::cout << "You don't have an inventory to drop items from.\n";
}

void equipItem(Player& player, Item* items, int itemCount, const char* itemName) {
    for (int i = 0; i < itemCount; i++) {
        if (strcmp(items[i].name, itemName) == 0) {
            if (items[i].type == WEAPON) {
                player.equippedWeapon = items[i];
                std::cout << "You equipped " << itemName << ".\n";
            } else if (items[i].type == ARMOR) {
                player.equippedArmor = items[i];
                std::cout << "You equipped " << itemName << ".\n";
            } else {
                std::cout << "You can't equip this item.\n";
            }
            return;
        }
    }
    std::cout << "You don't have any " << itemName << " to equip.\n";
}

void manageInventory(Player& player, Item* items, int itemCount) {
    char command[20];
    while (true) {
        displayInventory(player, items, itemCount);
        std::cout << "Enter command (use/drop/equip/back): ";
        std::cin >> command;

        if (strcmp(command, "back") == 0 || strcmp(command, "b") == 0) break;

        char itemName[50];
        std::cout << "Enter item name: ";
        std::cin.ignore();
        std::cin.getline(itemName, 50);

        if (strcmp(command, "use") == 0) {
            useItem(player, items, itemCount, itemName);
        } else if (strcmp(command, "drop") == 0) {
            dropItem(player, itemName);
        } else if (strcmp(command, "equip") == 0) {
            equipItem(player, items, itemCount, itemName);
        } else {
            std::cout << "Invalid command.\n";
        }
    }
}

// Update the buyItem function
void buyItem(Player& player, Item* items, int itemCount, const char* itemName) {
    for (int i = 0; i < itemCount; i++) {
        if (strcmp(items[i].name, itemName) == 0) {
            int price = items[i].value * 2; // Simple pricing
            if (player.gold >= price) {
                if (player.inventoryCount < MAX_INVENTORY) {
                    player.gold -= price;
                    player.inventory[player.inventoryCount] = items[i];
                    player.inventoryCount++;
                    if (items[i].type == WEAPON) {
                        player.equippedWeapon = items[i];
                        std::cout << "You bought and equipped " << itemName << ".\n";
                    } else if (items[i].type == ARMOR) {
                        player.equippedArmor = items[i];
                        std::cout << "You bought and equipped " << itemName << ".\n";
                    } else {
                        std::cout << "You bought " << itemName << ".\n";
                    }
                } else {
                    std::cout << "Your inventory is full.\n";
                }
            } else {
                std::cout << "You don't have enough gold to buy " << itemName << ".\n";
            }
            return;
        }
    }
    std::cout << "Item not found.\n";
}

void exploreMap(Player& player, Map& map, Item* items, int itemCount) {
    Position playerPos(0, 0);
    char move;
    int turnCount = 0;

    while (true) {
        system("cls"); // Clear screen (Windows-specific)
        map.display(playerPos);
        setColor(COLOR_PLAYER);
        std::cout << player.name << " - Level: " << player.level << " Exp: " << player.exp << "/" << player.level * 100 << std::endl;
        std::cout << "Health: " << player.health << " Attack: " << player.attack << " Gold: " << player.gold << std::endl;
        resetColor();
        std::cout << "Move (w/a/s/d), open inventory (i), or quit (q): ";
        std::cin >> move;

        if (move == 'i') {
            manageInventory(player, items, itemCount);
            continue;
        }

        Position newPos = playerPos;
        switch (move) {
            case 'w': newPos.y--; break;
            case 's': newPos.y++; break;
            case 'a': newPos.x--; break;
            case 'd': newPos.x++; break;
            case 'q': return;
            default: continue;
        }

        if (map.isValidMove(newPos)) {
            playerPos = map.getTeleportDestination(newPos);
            if (map.isEnemyAt(playerPos)) {
                battle(player, map, playerPos);
                if (player.health <= 0) return;
            }
        }

        turnCount++;
        if (turnCount % 3 == 0) {
            map.moveEnemies();
        }
    }
}

int main(int argc, char* argv[])
{
    srand(static_cast<unsigned int>(time(0)));

    Player player;
    std::cout << "Welcome to Simple RPG!\n";
    std::cout << "Enter your character's name: ";
    std::cin >> player.name;
    player.health = 100;
    player.maxHealth = 100;
    player.attack = 10;
    player.level = 1;
    player.exp = 0;
    player.gold = 0;
    strcpy(player.equippedWeapon.name, "Fists");
    player.equippedWeapon.type = WEAPON;
    player.equippedWeapon.value = 0;
    strcpy(player.equippedArmor.name, "Clothes");
    player.equippedArmor.type = ARMOR;
    player.equippedArmor.value = 0;
    player.inventoryCount = 0;

    Item items[MAX_INVENTORY];
    int itemCount = 0;
    initializeItems(items, itemCount);

    // Give the player some starting items
    strcpy(player.inventory[player.inventoryCount].name, "Health Potion");
    player.inventory[player.inventoryCount].type = POTION;
    player.inventory[player.inventoryCount].value = 20;
    player.inventoryCount++;

    strcpy(player.inventory[player.inventoryCount].name, "Sword");
    player.inventory[player.inventoryCount].type = WEAPON;
    player.inventory[player.inventoryCount].value = 5;
    player.inventoryCount++;

    std::cout << "Hello, " << player.name << "! Your adventure begins.\n";
    
    Map gameMap(50, 20); // Create a 20x10 map
    exploreMap(player, gameMap, items, itemCount);

    std::cout << "Thanks for playing!\n";
    return 0;
}
