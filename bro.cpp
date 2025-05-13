#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>

struct Ball {
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float mass;
    float maxSpeed;  // Changed back to non-static member

    Ball(float radius, sf::Vector2f position, sf::Vector2f velocity, sf::Color color, float maxSpeed)
        : velocity(velocity), mass(radius * radius), maxSpeed(maxSpeed) {
        shape.setRadius(radius);
        shape.setPosition(position);
        shape.setFillColor(color);
        shape.setOrigin(radius, radius);
    }

    void update(const sf::RenderWindow& window, bool gravityEnabled) {
        // Apply gravity if enabled
        if (gravityEnabled) {
            velocity.y += 0.2f; // Gravity acceleration
        }

        // Limit maximum speed
        float currentSpeed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
        if (currentSpeed > maxSpeed) {
            velocity = velocity * (maxSpeed / currentSpeed);
        }

        sf::Vector2f pos = shape.getPosition();
        pos += velocity;

        float r = shape.getRadius();
        sf::Vector2u winSize = window.getSize();

        // Bounce off walls with energy loss when gravity is enabled
        if (pos.x - r < 0) {
            pos.x = r;
            velocity.x = std::abs(velocity.x) * (gravityEnabled ? 0.8f : 1.0f);
        }
        else if (pos.x + r > winSize.x) {
            pos.x = winSize.x - r;
            velocity.x = -std::abs(velocity.x) * (gravityEnabled ? 0.8f : 1.0f);
        }
        
        if (pos.y - r < 0) {
            pos.y = r;
            velocity.y = std::abs(velocity.y) * (gravityEnabled ? 0.8f : 1.0f);
        }
        else if (pos.y + r > winSize.y) {
            pos.y = winSize.y - r;
            velocity.y = -std::abs(velocity.y) * (gravityEnabled ? 0.8f : 1.0f);
        }

        shape.setPosition(pos);
    }

    bool checkCollision(const Ball& other) const {
        sf::Vector2f diff = shape.getPosition() - other.shape.getPosition();
        float distanceSquared = diff.x * diff.x + diff.y * diff.y;
        float minDistance = shape.getRadius() + other.shape.getRadius();
        return distanceSquared <= minDistance * minDistance;
    }

    void resolveCollision(Ball& other, bool gravityEnabled) {
        sf::Vector2f diff = other.shape.getPosition() - shape.getPosition();
        float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        
        // Prevent division by zero
        if (distance < 0.0001f) {
            diff = sf::Vector2f(1.0f, 0.0f);
            distance = 1.0f;
        }
        
        // Normalize the collision vector
        sf::Vector2f normal = diff / distance;
        
        // If gravity is on, make balls stick together
        if (gravityEnabled) {
            // Calculate the average velocity
            sf::Vector2f avgVelocity = (velocity * mass + other.velocity * other.mass) / (mass + other.mass);
            // Set both balls to the same velocity
            velocity = avgVelocity;
            other.velocity = avgVelocity;
        } else {
            // Normal elastic collision when gravity is off
            sf::Vector2f relativeVelocity = velocity - other.velocity;
            float velocityAlongNormal = relativeVelocity.x * normal.x + relativeVelocity.y * normal.y;
            
            // Only resolve if balls are moving towards each other
            if (velocityAlongNormal > 0) return;
            
            // Calculate impulse
            float restitution = 0.8f;
            float j = -(1.0f + restitution) * velocityAlongNormal;
            j /= 1.0f/mass + 1.0f/other.mass;
            
            // Apply impulse
            sf::Vector2f impulse = j * normal;
            velocity -= impulse / mass;
            other.velocity += impulse / other.mass;
        }
        
        // Prevent balls from sticking together
        float overlap = (shape.getRadius() + other.shape.getRadius() - distance);
        if (overlap > 0) {
            sf::Vector2f correction = normal * overlap * 0.5f;
            shape.setPosition(shape.getPosition() - correction);
            other.shape.setPosition(other.shape.getPosition() + correction);
        }
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Bouncing Balls");
    window.setFramerateLimit(60);

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    std::vector<Ball> balls;
    bool gravityEnabled = false;
    
    // Create 4 red balls (fastest)
    for (int i = 0; i < 4; ++i) {
        float radius = 15.0f;
        float x = 100 + std::rand() % 600;
        float y = 100 + std::rand() % 400;
        float vx = -4 + static_cast<float>(std::rand() % 8); // -4 to 4
        float vy = -4 + static_cast<float>(std::rand() % 8); // -4 to 4
        balls.emplace_back(radius, sf::Vector2f(x, y), sf::Vector2f(vx, vy), 
                          sf::Color::Red, 7.0f); // Red balls are fastest
    }

    // Create 3 blue balls (medium speed)
    for (int i = 0; i < 3; ++i) {
        float radius = 15.0f;
        float x = 100 + std::rand() % 600;
        float y = 100 + std::rand() % 400;
        float vx = -2.5f + static_cast<float>(std::rand() % 5); // -2.5 to 2.5
        float vy = -2.5f + static_cast<float>(std::rand() % 5); // -2.5 to 2.5
        balls.emplace_back(radius, sf::Vector2f(x, y), sf::Vector2f(vx, vy), 
                          sf::Color::Blue, 5.0f); // Blue balls are medium speed
    }

    // Create 3 green balls (slowest)
    for (int i = 0; i < 3; ++i) {
        float radius = 15.0f;
        float x = 100 + std::rand() % 600;
        float y = 100 + std::rand() % 400;
        float vx = -1.5f + static_cast<float>(std::rand() % 3); // -1.5 to 1.5
        float vy = -1.5f + static_cast<float>(std::rand() % 3); // -1.5 to 1.5
        balls.emplace_back(radius, sf::Vector2f(x, y), sf::Vector2f(vx, vy), 
                          sf::Color::Green, 3.0f); // Green balls are slowest
    }

    // Create a background rectangle and text for gravity status
    sf::RectangleShape statusBox(sf::Vector2f(200, 60));
    statusBox.setFillColor(sf::Color(0, 0, 0, 200));  // Semi-transparent black
    statusBox.setPosition(10, 10);

    // Create and setup the text
    sf::Font font;
    if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
        // If Arial is not available, try to use the default font
        return -1;  // Exit if we can't load a font
    }

    sf::Text gravityText;
    gravityText.setFont(font);  // Set the font
    gravityText.setString("Gravity: OFF");
    gravityText.setCharacterSize(40);
    gravityText.setFillColor(sf::Color::White);
    gravityText.setPosition(20, 20);
    gravityText.setStyle(sf::Text::Bold);  // Make text bold

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::G) {
                    gravityEnabled = !gravityEnabled;
                    gravityText.setString(gravityEnabled ? "Gravity: ON" : "Gravity: OFF");
                    
                    // Add speed boost when turning gravity off
                    if (!gravityEnabled) {
                        for (auto& ball : balls) {
                            // Add a random boost to each ball's velocity
                            float boostX = -2.0f + static_cast<float>(std::rand() % 4); // -2 to 2
                            float boostY = -2.0f + static_cast<float>(std::rand() % 4); // -2 to 2
                            ball.velocity.x += boostX;
                            ball.velocity.y += boostY;
                        }
                    }
                }
                else if (event.key.code == sf::Keyboard::A) {
                    // Add a new random ball
                    float radius = 15.0f;
                    float x = 100 + std::rand() % 600;
                    float y = 100 + std::rand() % 400;
                    float vx = -2.0f + static_cast<float>(std::rand() % 4); // -2 to 2
                    float vy = -2.0f + static_cast<float>(std::rand() % 4); // -2 to 2
                    
                    // Randomly choose a color
                    sf::Color color;
                    int colorChoice = std::rand() % 3;
                    if (colorChoice == 0) {
                        color = sf::Color::Red;
                        balls.emplace_back(radius, sf::Vector2f(x, y), sf::Vector2f(vx, vy), 
                                         color, 7.0f); // Red balls are fastest
                    }
                    else if (colorChoice == 1) {
                        color = sf::Color::Blue;
                        balls.emplace_back(radius, sf::Vector2f(x, y), sf::Vector2f(vx, vy), 
                                         color, 5.0f); // Blue balls are medium speed
                    }
                    else {
                        color = sf::Color::Green;
                        balls.emplace_back(radius, sf::Vector2f(x, y), sf::Vector2f(vx, vy), 
                                         color, 3.0f); // Green balls are slowest
                    }
                }
                else if (event.key.code == sf::Keyboard::R) {
                    // Remove a random ball if there are any balls
                    if (!balls.empty()) {
                        int indexToRemove = std::rand() % balls.size();
                        balls.erase(balls.begin() + indexToRemove);
                    }
                }
            }
        }

        // First check for collisions
        for (size_t i = 0; i < balls.size(); ++i) {
            for (size_t j = i + 1; j < balls.size(); ++j) {
                if (balls[i].checkCollision(balls[j])) {
                    balls[i].resolveCollision(balls[j], gravityEnabled);
                }
            }
        }

        // Then update positions
        for (auto& ball : balls) {
            ball.update(window, gravityEnabled);
        }

        window.clear(sf::Color::Black);
        for (const auto& ball : balls)
            window.draw(ball.shape);
        window.draw(statusBox);
        window.draw(gravityText);
        window.display();
    }

    return 0;
}
