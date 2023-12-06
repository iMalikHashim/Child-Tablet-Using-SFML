#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstring>
#include <cmath>
#include <vector>
#include <sstream>
#include <iterator>
#include <fstream>

int counter=0;
using namespace std;

sf::RenderWindow window(sf::VideoMode(700, 700), "Tablet", sf::Style::Close);
sf::ConvexShape cursor;
sf::Font font;
sf::Text commandText;
sf::Text inputText;

const int MAX_COMMANDS = 100;
const int MAX_COMMAND_LENGTH = 256;
char commands[MAX_COMMANDS][MAX_COMMAND_LENGTH] = {0};
int commandCount = 0;
vector<sf::RectangleShape> lines;
sf::Text commandTexts[MAX_COMMANDS];
sf::Vector2f commandTextPosition(700, 680);
const int commandTextSpacing = 20;

char inputString[256] = {0};
float cursorAngle = 0.0f;
bool penDown = true;
sf::Color penColor = sf::Color::Black;
int penWidth = 5;
struct Circle {
    sf::Vector2f position;
    int radius;
    sf::Color color;
    int outlineThickness;
};

vector <Circle> circles;
// vector<sf::Vertex> lines;

// void moveCursor(char* command);
void moveCursor(const char command[]);
// void turnCursor(char* command, bool rightTurn);
void turnCursor(const char command[], bool rightTurn);
void executeCommand(const char* cmd);
// void drawLine(const sf::Vector2f& from, const sf::Vector2f& to) {
//     lines.push_back(sf::Vertex(from, penColor));
//     lines.push_back(sf::Vertex(to, penColor));
// }

void drawLine(const sf::Vector2f& from, const sf::Vector2f& to) {
    sf::RectangleShape line;
    sf::Vector2f direction = to - from;
    sf::Vector2f unitDirection = direction / std::sqrt(direction.x * direction.x + direction.y * direction.y);
    sf::Vector2f normal(-unitDirection.y, unitDirection.x);

    float thickness = static_cast<float>(penWidth);
    sf::Vector2f size(std::sqrt(direction.x * direction.x + direction.y * direction.y), thickness);

    line.setSize(size);
    line.setOrigin(0, thickness / 2);
    line.setPosition(from);
    line.setRotation(atan2(direction.y, direction.x) * 180 / 3.14159f);
    line.setFillColor(penColor);

    lines.push_back(line);
}


void saveCommands(const char* filename) {
    ofstream file(filename);
    if (file.is_open()) {
        for (int i = 0; i < commandCount; ++i) {
            file << commands[i] << endl;
        }
        file.close();
    } else {
        cerr << "Unable to open file for saving" << endl;
    }
}

void loadCommands(const char* filename) {
    ifstream file(filename);
    string line;
    if (file.is_open()) {
        while (getline(file, line) && commandCount < MAX_COMMANDS) {
            
            if (line.rfind("save", 0) == 0 || line.rfind("load", 0) == 0) {
                cout << "Skipping command: " << line << endl;
                continue; 
            }

            strncpy(commands[commandCount], line.c_str(), MAX_COMMAND_LENGTH);
            commands[commandCount][MAX_COMMAND_LENGTH - 1] = '\0';

            
            cout << "Executing command: " << commands[commandCount] << endl;

            executeCommand(commands[commandCount]);
            commandCount++;
        }
        file.close();
    } else {
        cerr << "Unable to open file for loading: " << filename << endl;
    }
}



void drawCircle(int radius) {
    Circle circle;
    circle.position = cursor.getPosition();
    circle.radius = radius;
    circle.color = penColor;
    circle.outlineThickness = penWidth;

    circles.push_back(circle);
}

// void changePenColor(const string& colorName) {
//     map<string, sf::Color> colorMap = {
//         {"BLACK", sf::Color::Black}, {"WHITE", sf::Color::White},
//         {"RED", sf::Color::Red}, {"GREEN", sf::Color::Green},
//         {"BLUE", sf::Color::Blue}, {"YELLOW", sf::Color::Yellow},
//         {"MAGENTA", sf::Color::Magenta}, {"CYAN", sf::Color::Cyan},
//         {"ORANGE", sf::Color(255, 165, 0)}, {"PURPLE", sf::Color(128, 0, 128)}
//     };
//     auto it = colorMap.find(colorName);
//     if (it != colorMap.end()) {
//         penColor = it->second;
//     }
// }

void changePenColor(const string& colorName) {
    if (colorName == "BLACK") {
        penColor = sf::Color::Black;
    } else if (colorName == "WHITE") {
        penColor = sf::Color::White;
    } else if (colorName == "RED") {
        penColor = sf::Color::Red;
    } else if (colorName == "GREEN") {
        penColor = sf::Color::Green;
    } else if (colorName == "BLUE") {
        penColor = sf::Color::Blue;
    } else if (colorName == "YELLOW") {
        penColor = sf::Color::Yellow;
    } else if (colorName == "MAGENTA") {
        penColor = sf::Color::Magenta;
    } else if (colorName == "CYAN") {
        penColor = sf::Color::Cyan;
    } else if (colorName == "ORANGE") {
        penColor = sf::Color(255, 165, 0);
    } else if (colorName == "PURPLE") {
        penColor = sf::Color(128, 0, 128);
    } else {
        cerr << "Unknown color: " << colorName << endl;
    }
}

void renderCommandHistory() {
    const int maxDisplayCommands = 10;
    int startCommand = max(0, commandCount - maxDisplayCommands);
    float lineHeight = 24;
    float startY = window.getSize().y - lineHeight;

    for (int i = startCommand; i < commandCount; ++i) {
        sf::Text command;
        command.setFont(font);
        command.setCharacterSize(24);
        command.setFillColor(sf::Color::Black);
        command.setString(commands[i]);
        command.setPosition(window.getSize().x - command.getLocalBounds().width - 10, startY);

        window.draw(command);
        startY -= lineHeight;
    }
}

void repeatCommand(const char* cmd) {
    int times = atoi(cmd + 7);
    const char* start = strchr(cmd, '[');
    const char* end = strrchr(cmd, ']');

    if (!start || !end || start > end) {
        cerr << "Invalid repeat command format." << endl;
        return;
    }

    string commands = string(start + 1, end);
    istringstream iss(commands);
    vector<string> commandList((istream_iterator<string>(iss)), istream_iterator<string>());

    for (int i = 0; i < times; ++i) {
        for (const auto& singleCmd : commandList) {
            executeCommand(singleCmd.c_str());
        }
    }
}

void setPenState(bool down) {
    penDown = down;
}

void changePenWidth(int width) {
    penWidth = width;
}

void clearScreen() {
    lines.clear();
    circles.clear(); 
}


void moveCursor(const char command[]) {
    char cmd[256];
    strcpy(cmd, command); // Copy command into a local array
    int distance = atoi(cmd + 2);
    
    float radianAngle = (cursorAngle+90.0f) * 3.14159f / 180.0f;
    float dx = cos(radianAngle) * distance;
    float dy = sin(radianAngle) * distance;

    if (command[0] == 'b') { // Check for backward command
        dx = -dx;
        dy = -dy;
    }

    
    sf::Vector2f startPosition = cursor.getPosition();
    sf::Vector2f endPosition(startPosition.x + dx, startPosition.y - dy);

    if (penDown) {
        drawLine(startPosition, endPosition);
    }

    cursor.setPosition(endPosition);
}

// void turnCursor(char* command, bool rightTurn) {
//     int angle = atoi(command + 2);
//     float cuAngle;
//     if (angle % 45 != 0 && angle % 30 != 0) {
//         cerr << "Invalid angle. Please enter an angle that is a multiple of 45 or 30 degrees." << endl;
//         return; 
//     }
//     if (rightTurn) {
//         cout << "Cursor Angle: " << cursorAngle << endl;
        
//         // Check if the angle is a multiple of 30 degrees
//         if (angle % 30 == 0) {
//             // Calculate the number of 30-degree steps to turn
//             int steps = angle / 30;
            
//             // Adjust the cursor angle in 30-degree steps
//             cursorAngle -= steps * 30;
//         }
//         // Check if the angle is a multiple of 45 degrees
//         else if (angle % 45 == 0) {
//             // Calculate the number of 45-degree steps to turn
//             int steps = angle / 45;
            
//             // Adjust the cursor angle in 45-degree steps
//             cursorAngle -= steps * 45;
//         }
        
//         cout << "Cursor Angle after deduction: " << cursorAngle << endl;
        
//     } else {
//         // Similar logic for left turns
//         if (angle % 30 == 0) {
//             int steps = angle / 30;
//             cursorAngle += steps * 30;
//         }
//         else if (angle % 45 == 0) {
//             int steps = angle / 45;
//             cursorAngle += steps * 45;
//         }
//     }
    
//     while (cursorAngle >= 360.0f) cursorAngle += 360.0f;
//     while (cursorAngle <= 0.0f) cursorAngle += 360.0f;
    
//     if (cursorAngle == 180.0 || cursorAngle == 360.0 || cursorAngle == -45.0) {
//         cuAngle = cursorAngle;
//     } else if (cursorAngle == 0) {
//         cuAngle = 0;
//     } else {
//         cuAngle = (cursorAngle + 180.0f);
//     }

//     cout << "What is given to setRotation: " << cuAngle << endl;
//     cursor.setRotation(cuAngle);
// }



void turnCursor(const char command[], bool rightTurn) {
    char cmd[256];
    strcpy(cmd, command); // Copy command into a local array
    int angle = atoi(cmd + 2);

    float cuAngle;
    if (angle % 45 != 0 && angle % 30 != 0) {
        cerr << "Invalid angle. Please enter an angle that is a multiple of 45 or 30 degrees." << endl;
        return; 
    }

    if (rightTurn) {
        cout << "CursorAngle before: " << cursorAngle << endl;
        cursorAngle -= angle;
        cout << "CursorAngle After: " << cursorAngle << endl;
    } else {
        cout << "CursorAngle before: " << cursorAngle << endl;
        cursorAngle += angle;
        cout << "CursorAngle After: " << cursorAngle << endl;
    }

    // Normalize the cursor angle to 0 - 360 range
    while (cursorAngle >= 360.0f) cursorAngle -= 360.0f;
    while (cursorAngle < 0.0f) cursorAngle += 360.0f;

    cout << "Normalized: " << cursorAngle << endl;

    if (cursorAngle == 180.0 || cursorAngle == 360.0) {
        cuAngle = cursorAngle;
    }else if(fmod(cursorAngle,45.0f) == 0 || fmod(cursorAngle,30.0f) == 0 ){
        cuAngle=360.0f - cursorAngle;
    } 
    else if (cursorAngle == 0) {
        cuAngle = 0;
    } else {
        cuAngle = (cursorAngle + 180.0f);
    }

    cout << "What is given to setRotation: " << cuAngle << endl;
    cursor.setRotation(cuAngle);
}

void initializeCursorAndText() {
    cursor.setPointCount(3);
    cursor.setPoint(0, sf::Vector2f(0, -20));
    cursor.setPoint(1, sf::Vector2f(-10, 10));
    cursor.setPoint(2, sf::Vector2f(10, 10));
    cursor.setFillColor(sf::Color::Green);
    
    
    sf::Vector2f centroid = (cursor.getPoint(0) + cursor.getPoint(1) + cursor.getPoint(2)) / 3.0f;
    cursor.setOrigin(centroid);
    cursor.setPosition(window.getSize().x / 2, window.getSize().y / 2);
    if (!font.loadFromFile("arial.ttf")) {
        cerr << "Error loading font" << endl;
    }

    commandText.setFont(font);
    commandText.setCharacterSize(24);
    commandText.setFillColor(sf::Color::Black);
    commandText.setPosition(10, window.getSize().y - 40);

    inputText.setFont(font);
    inputText.setCharacterSize(24);
    inputText.setFillColor(sf::Color::Red);
    inputText.setPosition(10, 10);
}

void executeCommand(const char cmd[]) {
    char command[256];
    strcpy(command, cmd); 
    if (commandCount < MAX_COMMANDS) {
        strncpy(commands[commandCount], cmd, MAX_COMMAND_LENGTH);
        commands[commandCount][MAX_COMMAND_LENGTH - 1] = '\0';
        commandCount++;
    }

    bool knownCommand = strncmp(cmd, "fd", 2) == 0 || strncmp(cmd, "bk", 2) == 0 ||
                        strncmp(cmd, "rt", 2) == 0 || strncmp(cmd, "lt", 2) == 0 ||
                        strncmp(cmd, "pu", 2) == 0 || strncmp(cmd, "pd", 2) == 0 ||
                        strncmp(cmd, "color", 5) == 0 || strncmp(cmd, "width", 5) == 0 ||
                        strncmp(cmd, "cs", 2) == 0 || strncmp(cmd, "circle", 6) == 0 ||
                        strncmp(cmd, "repeat", 6) == 0 || strncmp(cmd, "save", 4) == 0 ||
                        strncmp(cmd, "load", 4) == 0;
    
    // Check for incomplete command (e.g., "fd" without a number)
    bool incompleteCommand = (strncmp(cmd, "fd", 2) == 0 || strncmp(cmd, "bk", 2) == 0 ||
                              strncmp(cmd, "rt", 2) == 0 || strncmp(cmd, "lt", 2) == 0 ||
                              strncmp(cmd, "color", 5) == 0 || strncmp(cmd, "width", 5) == 0 ||
                              strncmp(cmd, "circle", 6) == 0 || strncmp(cmd, "repeat", 6) == 0) &&
                              strlen(cmd) <= 3;  // Adjust this condition based on your command format

    if (incompleteCommand) {
        inputText.setString("Incomplete command");
        cerr << "Incomplete command" << endl;
    } else if (!knownCommand) {
        inputText.setString("I don't know this command");
        cerr << "Unknown command" << endl;
    } else {


    if (strncmp(command, "fd", 2) == 0 || strncmp(command, "forward", 7) == 0) {
        moveCursor(command);
    } else if (strncmp(command, "bk", 2) == 0 || strncmp(command, "backward", 8) == 0) {
        moveCursor(command);
    } else if (strncmp(command, "rt", 2) == 0) {
        turnCursor(command, true);
    } else if (strncmp(command, "lt", 2) == 0) {
        turnCursor(command, false);
        }
     else if (strncmp(cmd, "pu", 2) == 0) {
        setPenState(false);
    } else if (strncmp(cmd, "pd", 2) == 0) {
        setPenState(true);
    } else if (strncmp(cmd, "color", 5) == 0) {
        string colorName = cmd + 6;
        changePenColor(colorName);
    } else if (strncmp(cmd, "width", 5) == 0) {
        int width = atoi(cmd + 6);
        changePenWidth(width);
    } else if (strncmp(cmd, "cs", 2) == 0) {
        clearScreen();
        cursor.setPosition(window.getSize().x / 2, window.getSize().y / 2);
        cursorAngle = 0.0f;
        cursor.setRotation(cursorAngle);
    } else if (strncmp(cmd, "circle", 6) == 0) {
        int radius = atoi(cmd + 7);
        drawCircle(radius);
    } else if (strncmp(cmd, "repeat", 6) == 0) {
        repeatCommand(cmd);
    }

    if (strncmp(cmd, "save", 4) == 0) {
        saveCommands(cmd + 5);
    } else if (strncmp(cmd, "load", 4) == 0) {
        loadCommands(cmd + 5);
    }
}
}

int main() {
    initializeCursorAndText();

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode == '\b' && strlen(inputString) > 0) {
                    inputString[strlen(inputString) - 1] = '\0';
                } else if (event.text.unicode == '\r') {
                    executeCommand(inputString);
                    memset(inputString, 0, sizeof(inputString));
                } else if (event.text.unicode >= 32 && event.text.unicode < 128) {
                    size_t len = strlen(inputString);
                    if (len < sizeof(inputString) - 1) {
                        inputString[len] = static_cast<char>(event.text.unicode);
                        inputString[len + 1] = '\0';
                    }
                }
                commandText.setString(inputString);
            }
        }

        window.clear(sf::Color::White);

        // if (!lines.empty()) {
        //     window.draw(&lines[0], lines.size(), sf::Lines);
        // }
        for (const auto& line : lines) {
            window.draw(line);
        }
        for (const auto& circle : circles) {
            sf::CircleShape shape(circle.radius);
            shape.setOrigin(circle.radius, circle.radius);
            shape.setPosition(circle.position);
            shape.setFillColor(sf::Color::Transparent);
            shape.setOutlineColor(circle.color);
            shape.setOutlineThickness(circle.outlineThickness);

            window.draw(shape);
        }

        window.draw(cursor);
        window.draw(commandText);
        window.draw(inputText);
        renderCommandHistory();

        window.display();
    }

    return 0;
}
