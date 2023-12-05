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
char inputString[256] = {0};  // Using char array instead of std::string
float cursorAngle = 0.0f;  // in degrees
bool penDown = true;
sf::Color penColor = sf::Color::Black;
int penWidth = 500;
vector<sf::Vertex> lines; 
// Function declarations
void moveCursor(char* command);
void turnCursor(char* command, bool rightTurn);
void executeCommand(const char* cmd);
void drawLine(const sf::Vector2f& from, const sf::Vector2f& to) {
    lines.push_back(sf::Vertex(from, penColor));
    lines.push_back(sf::Vertex(to, penColor));
}


void drawCircle(int radius) {
    sf::CircleShape circle(radius);
    circle.setOrigin(radius, radius); // Set origin to the center of the circle
    circle.setPosition(cursor.getPosition()); // Position at cursor's location
    circle.setFillColor(sf::Color::Transparent); // Transparent fill
    circle.setOutlineColor(penColor); // Use current pen color for outline
    circle.setOutlineThickness(penWidth); // Use current pen width for outline thickness

    window.draw(circle); // Draw the circle
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
}
    // Add more command handling here
}


    // Add more command handling here

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

    // Draw the cursor, command text, and input text
    window.draw(cursor);
    window.draw(commandText);
    window.draw(inputText);

    // Draw additional shapes (like circles) if needed
    // This can be implemented based on how you manage shapes

    window.display();
}

    return 0;
}