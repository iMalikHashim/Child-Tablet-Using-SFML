#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstring>
#include <cmath>
#include <vector>
#include <sstream>
#include <iterator>

using namespace std;

// Define necessary global variables
sf::RenderWindow window(sf::VideoMode(1400, 700), "Tablet", sf::Style::Close);
sf::ConvexShape cursor;
sf::Font font;
sf::Text commandText;
sf::Text inputText;
string inputString = "";
float cursorAngle = 0.0f;  // in degrees
bool penDown = true;
sf::Color penColor = sf::Color::Black;
int penWidth = 2;
vector<sf::Vertex> lines; 
// Function declarations
void moveCursor(char* command);
void turnCursor(char* command, bool rightTurn);

void drawLine(const sf::Vector2f& from, const sf::Vector2f& to) {
    lines.push_back(sf::Vertex(from, penColor));
    lines.push_back(sf::Vertex(to, penColor));
}


void repeatCommand(int times, const vector<string>& commands) {
    for (int i = 0; i < times; ++i) {
        for (const auto& cmd : commands) {
            executeCommand(cmd); // Assumes executeCommand can handle a string command
        }
    }
}

void setPenState(bool down) {
    penDown = down;
}


void changePenColor(const string& colorName) {
    map<string, sf::Color> colorMap = {
        {"BLACK", sf::Color::Black}, {"WHITE", sf::Color::White},
        // ... other colors ...
    };
    auto it = colorMap.find(colorName);
    if (it != colorMap.end()) {
        penColor = it->second;
    }
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

// Function to execute a command
void executeCommand(const string& command) {
    char cmd[100];
    strncpy(cmd, command.c_str(), sizeof(cmd));
    cmd[sizeof(cmd) - 1] = 0;

    if (strncmp(cmd, "fd", 2) == 0 || strncmp(cmd, "forward", 7) == 0) {
        moveCursor(cmd);
    } else if (strncmp(cmd, "bk", 2) == 0 || strncmp(cmd, "backward", 8) == 0) {
        moveCursor(cmd);
    } else if (strncmp(cmd, "rt", 2) == 0) {
        turnCursor(cmd, true);
    } else if (strncmp(cmd, "lt", 2) == 0) {
        turnCursor(cmd, false);
    }
    // Add more commands here
}

int main() {
    initializeCursorAndText();

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode == '\b' && !inputString.empty()) {
                    inputString.pop_back(); // Handle backspace
                } else if (event.text.unicode == '\r') {
                    executeCommand(inputString); // Execute command on Enter
                    inputString.clear(); // Clear the input string
                } else if (event.text.unicode >= 32 && event.text.unicode < 128) {
                    inputString += static_cast<char>(event.text.unicode); // Append character
                }
                commandText.setString("Command: " + inputString); // Update command text
            }
        }

        window.clear(sf::Color::White);

        // Draw all lines
        if (!lines.empty()) {
            window.draw(&lines[0], lines.size(), sf::Lines);
        }

        window.draw(cursor);          // Draw the cursor
        window.draw(commandText);     // Draw the command text
        window.draw(inputText);       // Draw the input text

        window.display();
    }

    return 0;
}
