#include <SFML/Graphics.hpp>
#include <algorithm>
#include <math.h>
#include <cmath>
#include <random>
#include <string>
#include <sstream>
#include <stdio.h>
#include <cstdlib>

std::mt19937 gen(std::random_device{}());
typedef std::uniform_real_distribution<float> urd;

std::stringstream pointss;
int points = 0;
int screen_w = 800;
int screen_h = 600;
bool food_ate = false;
float food_r = 6.5f;
float food_speed = 0.004f;
float circle_r = 10.0f;
float circle_speed = 0.008f;

urd rand_dir(0.0f, 2.0f * (float)M_PI);

float food_x = 0.0f;
float food_y = 0.0f;
float food_dx = food_speed;
float food_dy = food_speed;
float circle_growth_rate = 1.0f;
float spawn_growth_rate = 0.01f;
float despawn_growth_rate = 0.05f;
sf::Time spawn_time = sf::seconds(7.0f);
sf::Time despawn_time = sf::seconds(5.0f);
sf::Time elapsed_time = sf::seconds(0.0f);
float circle_x = (float)screen_w / 2.0f;
float circle_y = (float)screen_h / 2.0f;

float getRandomX() {
    std::mt19937 genX(std::random_device{}());
    urd rand_x(food_r, (float)screen_w - food_r);
    return rand_x(genX);
}

float getRandomY() {
    std::mt19937 genY(std::random_device{}());
    urd rand_x(food_r, (float)screen_h - circle_r);
    return rand_x(genY);
}

void setRandomFoodDir() {
    float angle = rand_dir(gen);

    float x = cosf(angle);
    float y = sinf(angle);
    float len = sqrtf(x * x + y * y);

    food_dx = x / len * food_speed;
    food_dy = y / len * food_speed;
    food_x = ((float)rand()) / ((float)RAND_MAX) * (screen_w - food_r) + food_r;
    food_y = ((float)rand()) / ((float)RAND_MAX) * (screen_h - food_r) + food_r;
}

void updateFoodLocation() {
    if ((food_x - food_r) <= 0 || (food_x + food_r) >= screen_w) {
        food_dx = -food_dx;
    }

    if ((food_y - food_r) <= 0 || (food_y + food_r) >= screen_h) {
        food_dy = -food_dy;
    }
    
    food_x += food_dx;
    food_y += food_dy;
}

void increaseSpawnRate () {
    float sec = spawn_time.asSeconds();
    spawn_time = sf::seconds(sec - spawn_growth_rate);
    spawn_growth_rate *= 2.0f;
}

void increaseDespawnRate () {
    float sec = despawn_time.asSeconds();
    despawn_time = sf::seconds(sec - despawn_growth_rate);
    despawn_growth_rate *= 1.3f;
}

void levelUp() {
    points++;
    circle_speed += (circle_speed * 0.0555f);
    food_speed += (food_speed * 0.0777f);
    circle_r += (circle_r * 0.0777f);
    food_r -= (food_r * 0.0555f);
    pointss.str(std::string());
    pointss << points;
    increaseSpawnRate();
    increaseDespawnRate();
}

void checkKeyboard (sf::Vector2f c) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && (c.x - circle_r) > 0) {
        circle_x -= circle_speed;
    }  
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && (c.x + circle_r) < screen_w) {
        circle_x += circle_speed;
    } 
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && (c.y - circle_r) > 0) {
        circle_y -= circle_speed;
    }  
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && (c.y + circle_r) < screen_h) {
        circle_y += circle_speed;
    }
}

void updateCircle(sf::CircleShape* circle) {
    circle->setPosition(circle_x, circle_y);
    circle->setRadius(circle_r);
}

bool gotFood() {
    float dx = circle_x - food_x;
    float dy = circle_y - food_y;
    float d = sqrtf(dx * dx + dy * dy);
    
    if (d <= circle_r - food_r ||
        d <= food_r - circle_r ||
        d <= food_r + circle_r) {
        return true;
    }

    return false;
}

void resetClock() { elapsed_time = sf::seconds(0.0f); }

void updateFood(sf::CircleShape* food) {
    if (food_ate) { // food has not spawned
        // if it's not time to spawn, exit early
        if (elapsed_time < spawn_time) { return; }

        // food needs to spawn
        setRandomFoodDir();
        resetClock();
        food_ate = false;
    } else { // food is spawned
        if (elapsed_time < despawn_time) {
            sf::Vector2f f_loc = food->getPosition();

            // check if food has been eaten
            if (gotFood()) {
                food_ate = true;
                levelUp();
                resetClock();
                return;
            }

            updateFoodLocation();
            food->setPosition(food_x, food_y);
            return;
        }

        // despawn food
        food_ate = true;
        resetClock();
    }
}


int main() {
    food_x = getRandomX();
    food_y = getRandomY();

    sf::RenderWindow window(sf::VideoMode(screen_w, screen_h), "SFML!");
    
    sf::Clock clock;
    
    sf::CircleShape circle(circle_r);
    circle.setFillColor(sf::Color::Green);
    circle.setOrigin(circle_r, circle_r);
    
    sf::CircleShape food(food_r);
    food.setFillColor(sf::Color::Red);
    food.setOrigin(food_r, food_r);
    food.setPosition(getRandomX(), getRandomY());

    sf::Font nerdHack;
    nerdHack.loadFromFile("NerdHack.ttf");

    sf::Text text;
    text.setFont(nerdHack);
    text.setFillColor(sf::Color::White);
    text.setCharacterSize(24);
    text.setString(L"Points: ");
    text.setPosition(10.0f, 0.0f);

    sf::Text point_str;
    point_str.setFont(nerdHack);
    point_str.setCharacterSize(24);
    point_str.setFillColor(sf::Color::White);
    point_str.setPosition(129.0f, 0.0f);
    pointss << points;


    while (window.isOpen()) {
        elapsed_time += clock.restart();
        sf::Event event;

        checkKeyboard(circle.getPosition());
        updateCircle(&circle);
        updateFood(&food);

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        window.clear();
        if (!food_ate) {
            window.draw(food);
        }

        point_str.setString(pointss.str());

        window.draw(circle);
        window.draw(text);
        window.draw(point_str);
        window.display();
    }

    return 0;
}
