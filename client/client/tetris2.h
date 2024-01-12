#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <functional>
#include <string.h>

#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "network.h"

class Tetris2 {

    
    static const std::uint32_t lines{ 20 };
    static const std::uint32_t cols{ 10 };
    static const std::uint32_t squares{ 4 };
    static const std::uint32_t shapes{ 7 };

    std::vector<std::vector<std::uint32_t>> area;
    std::vector<std::vector<std::uint32_t>> forms;

    struct Coords {
        std::uint32_t x, y;
    } z[squares], k[squares];

    std::shared_ptr<sf::RenderWindow> window;
    sf::Texture tiles, bg, bg1;
    std::shared_ptr<sf::Sprite> sprite, background, background1;
    sf::Clock clock;
    sf::Font  font;
    sf::Text txtScore, txtGameOver, textResult;

    int dirx, color, score, level;
    bool rotate, gameover, sendResult;
    float timercount, delay;
    std::string nameLog;
    

protected:
    void events();
    void draw();
    void moveToDown();
    void setRotate();
    void resetValues();
    void changePosition();
    bool maxLimit();
    void setScore();
    std::function<void(int)> gameOverCallback;

public:
    std::vector<std::vector<std::uint32_t>> areaEnermy;
    Tetris2(int socket, std::string name, std::string nameLog);
    ~Tetris2();
    void run();
    void setGameOverCallback(std::function<void(int)> callback);
    int getScore() const;
    void receiveData();
    bool sendVector2D(int socket, const std::vector<std::vector<std::uint32_t>>& data);

    // void updateGameSpeedDisplay();
    int gameSocket;
    std::string roomName;
};