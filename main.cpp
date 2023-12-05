#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstring>
#include <cmath>
#include <vector>
#include <sstream>
#include <iterator>
#include <fstream>

using namespace std;

// Define necessary global variables
sf::RenderWindow window(sf::VideoMode(700, 700), "Tablet", sf::Style::Close);
sf::ConvexShape cursor;
sf::Font font;
sf::Text commandText;
sf::Text inputText;

const int MAX_COMMANDS = 100;
const int MAX_COMMAND_LENGTH = 256;
char commands[MAX_COMMANDS][MAX_COMMAND_LENGTH] = {0};
int commandCount = 0; // Keep track of the number of commands stored

sf::Text commandTexts[MAX_COMMANDS]; // Store Text objects for displaying commands
sf::Vector2f commandTextPosition(700, 680); // Starting position for displaying commands
const int commandTextSpacing = 20; // Ve

char inputString[256] = {0};  // Using char array instead of std::string
float cursorAngle = 0.0f;  // in degrees
bool penDown = true;
sf::Color penColor = sf::Color::Black;
int penWidth = 500;
struct Circle {
    sf::Vector2f position;
    int radius;
    sf::Color color;
    int outlineThickness;
};

vector <Circle> circles;
vector<sf::Vertex> lines; 
// Function declarations
void moveCursor(char* command);
void turnCursor(char* command, bool rightTurn);
void executeCommand(const char* cmd);
void drawLine(const sf::Vector2f& from, const sf::Vector2f& to) {
    lines.push_back(sf::Vertex(from, penColor));
    lines.push_back(sf::Vertex(to, penColor));
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
            strncpy(commands[commandCount], line.c_str(), MAX_COMMAND_LENGTH);
            commands[commandCount][MAX_COMMAND_LENGTH - 1] = '\0'; // Ensure null-termination
            executeCommand(commands[commandCount]);
            commandCount++;
        }
        file.close();
    } else {
        cerr << "Unable to open file for loading" << endl;
    }
}


void drawCircle(int radius) {
    Circle circle;
    circle.position = cursor.getPosition();
    circle.radius = radius;
    circle.color = penColor;
    circle.outlineThickness = penWidth;

    circles.push_back(circle); // Add circle to the vector
}


void changePenColor(const string& colorName) {
    map<string, sf::Color> colorMap = {
        {"BLACK", sf::Color::Black}, {"WHITE", sf::Color::White},
        {"RED", sf::Color::Red}, {"GREEN", sf::Color::Green},
        {"BLUE", sf::Color::Blue}, {"YELLOW", sf::Color::Yellow},
        {"MAGENTA", sf::Color::Magenta}, {"CYAN", sf::Color::Cyan},
        {"ORANGE", sf::Color(255, 165, 0)}, {"PURPLE", sf::Color(128, 0, 128)}
        // You can add more colors here
    };
    auto it = colorMap.find(colorName);
    if (it != colorMap.end()) {
        penColor = it->second;
    }
}
void renderCommandHistory() {
    const int maxDisplayCommands = 10; // Number of commands to display
    int startCommand = max(0, commandCount - maxDisplayCommands);
    float lineHeight = 24; // Assuming line height based on font size

    // Start drawing from the bottom of the screen
    float startY = window.getSize().y - lineHeight;

    for (int i = startCommand; i < commandCount; ++i) {
        sf::Text command;
        command.setFont(font);
        command.setCharacterSize(24);
        command.setFillColor(sf::Color::Black);
        command.setString(commands[i]);
        command.setPosition(window.getSize().x - command.getLocalBounds().width - 10, startY);

        window.draw(command);
        startY -= lineHeight; // Move up for the next command
    }
}
void repeatCommand(const char* cmd) {
    // Extract number of times to repeat
    int times = atoi(cmd + 7); // Assuming the format is "repeat <number> [...]"
    
    // Find the starting and ending brackets
    const char* start = strchr(cmd, '[');
    const char* end = strrchr(cmd, ']');
    
    if (!start || !end || start > end) {
        cerr << "Invalid repeat command format." << endl;
        return;
    }

    // Extract the commands within the brackets
    string commands = string(start + 1, end);
    
    // Split the commands into individual commands
    istringstream iss(commands);
    vector<string> commandList((istream_iterator<string>(iss)), istream_iterator<string>());

    // Repeat the commands
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
    lines.clear(); // Assuming 'lines' is a vector of sf::Vertex used for drawing
}


// Move cursor forward or backward
void moveCursor(char* command) {
    int distance = atoi(command + 2);
    float radianAngle = cursorAngle * 3.14159f / 180.0f;
    float dx = std::cos(radianAngle) * distance;
    float dy = std::sin(radianAngle) * distance;

    if (command[0] == 'b') { // Move backward
        dx = -dx;
        dy = -dy;
    }

    sf::Vector2f startPosition = cursor.getPosition();
    sf::Vector2f endPosition(startPosition.x + dx, startPosition.y + dy);

    if (penDown) {
        drawLine(startPosition, endPosition);
    }

    cursor.setPosition(endPosition);
}

// Turn cursor right or left
void turnCursor(char* command, bool rightTurn) {
    // Extract the angle from the command
    int angle = atoi(command + 2);

    // Update the cursor angle
    cursorAngle += rightTurn ? angle : -angle;
    cursor.setRotation(cursorAngle);
}

// Initialize cursor and text objects
void initializeCursorAndText() {
    // Setup the cursor as a triangle
    cursor.setPointCount(3);
    cursor.setPoint(0, sf::Vector2f(0, -10));
    cursor.setPoint(1, sf::Vector2f(-5, 5));
    cursor.setPoint(2, sf::Vector2f(5, 5));
    cursor.setFillColor(sf::Color::Green);
    cursor.setPosition(window.getSize().x / 2, window.getSize().y / 2);

    // Load a font
    if (!font.loadFromFile("arial.ttf")) { // Replace with your font file path
        cerr << "Error loading font" << endl;
    }

    // Setup command input text
    commandText.setFont(font);
    commandText.setCharacterSize(24);
    commandText.setFillColor(sf::Color::Black);
    commandText.setPosition(10, window.getSize().y - 40);

    // Setup input display text
    inputText.setFont(font);
    inputText.setCharacterSize(24);
    inputText.setFillColor(sf::Color::Red);
    inputText.setPosition(10, 10);
}

// ... [Previous code including global variables and function declarations] ...

void executeCommand(const char* cmd) {
    if (commandCount < MAX_COMMANDS) {
        strncpy(commands[commandCount], cmd, MAX_COMMAND_LENGTH);
        commands[commandCount][MAX_COMMAND_LENGTH - 1] = '\0'; // Ensure null-termination
        commandCount++;
    }
    if (strncmp(cmd, "fd", 2) == 0 || strncmp(cmd, "forward", 7) == 0) {
        moveCursor(const_cast<char*>(cmd));
    } else if (strncmp(cmd, "bk", 2) == 0 || strncmp(cmd, "backward", 8) == 0) {
        moveCursor(const_cast<char*>(cmd));
    } else if (strncmp(cmd, "rt", 2) == 0) {
        turnCursor(const_cast<char*>(cmd), true);
    } else if (strncmp(cmd, "lt", 2) == 0) {
        turnCursor(const_cast<char*>(cmd), false);
    } else if (strncmp(cmd, "pu", 2) == 0) {
        setPenState(false); // Pen up
    } else if (strncmp(cmd, "pd", 2) == 0) {
        setPenState(true); // Pen down
    } else if (strncmp(cmd, "color", 5) == 0) {
        string colorName = cmd + 6; // Extract the color name
        changePenColor(colorName);
    } else if (strncmp(cmd, "width", 5) == 0) {
        int width = atoi(cmd + 6); // Extract the width value
        changePenWidth(width);
    } else if (strncmp(cmd, "cs", 2) == 0) {
        clearScreen();
    } else if (strncmp(cmd, "circle", 6) == 0) {
        int radius = atoi(cmd + 7); // Extract the radius value
        drawCircle(radius);
    } else if (strncmp(cmd, "repeat", 6) == 0) {
    repeatCommand(cmd);

    if (strncmp(cmd, "save", 4) == 0) {
        saveCommands(cmd + 5); // Assuming the format is "save <filename>"
    } else if (strncmp(cmd, "load", 4) == 0) {
        loadCommands(cmd + 5); // Assuming the format is "load <filename>"
    } 
}
    // Add more command handling here
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
                    inputString[strlen(inputString) - 1] = '\0'; // Handle backspace
                } else if (event.text.unicode == '\r') {
                    executeCommand(inputString); // Execute command on Enter
                    memset(inputString, 0, sizeof(inputString)); // Clear the command array
                } else if (event.text.unicode >= 32 && event.text.unicode < 128) {
                    size_t len = strlen(inputString);
                    if (len < sizeof(inputString) - 1) {
                        inputString[len] = static_cast<char>(event.text.unicode);
                        inputString[len + 1] = '\0'; // Append character and terminate string
                    }
                }
                commandText.setString(inputString); // Update command text
            }
        }

        window.clear(sf::Color::White);

        // Draw all lines
        if (!lines.empty()) {
            window.draw(&lines[0], lines.size(), sf::Lines);
        }

        // Draw all circles
        for (const auto& circle : circles) {
            sf::CircleShape shape(circle.radius);
            shape.setOrigin(circle.radius, circle.radius);
            shape.setPosition(circle.position);
            shape.setFillColor(sf::Color::Transparent);
            shape.setOutlineColor(circle.color);
            shape.setOutlineThickness(circle.outlineThickness);

            window.draw(shape);
        }

        // Draw the cursor, command text, and input text
        window.draw(cursor);
        window.draw(commandText);
        window.draw(inputText);
        renderCommandHistory();

        window.display();
    }

    return 0;
}
