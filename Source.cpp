#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>

const int windowWidth = 800;
const int windowHeight = 600;
const float gravity = 0.5f;
const float jumpVelocity = -10.0f;
const int pipeGap = 200;
const std::string highScoresFile = "highscores.txt";

struct Bird {
    sf::Sprite sprite;
    sf::Vector2f velocity;
    int score = 0;
};

class Game {
public:
    Game() : window(sf::VideoMode(windowWidth, windowHeight), "Flappy Bird"), font() {
        if (!font.loadFromFile("arial.ttf")) {
            std::cout << "Failed to load font." << std::endl;
        }

        resetGame();
    }

    void run() {
        while (window.isOpen()) {
            processEvents();
            update();
            render();
        }
    }

private:
    sf::RenderWindow window;
    sf::Font font;
    sf::Texture birdTexture;
    sf::Texture pipeTexture;
    std::vector<Bird> birds;
    std::vector<sf::Sprite> pipes;
    std::vector<std::pair<std::string, int>> highScores;

    void processEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            for (auto& bird : birds) {
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Up && bird.sprite.getPosition().y > 0) {
                    bird.velocity.y = jumpVelocity;
                }
            }

            for (auto& bird : birds) {
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::W && bird.sprite.getPosition().y > 0) {
                    bird.velocity.y = jumpVelocity;
                }
            }
        }
    }

    void update() {
        for (auto& bird : birds) {
            bird.velocity.y += gravity;
            bird.sprite.move(bird.velocity);

            if (bird.sprite.getPosition().y < 0) {
                bird.sprite.setPosition(bird.sprite.getPosition().x, 0);
            }
        }

        spawnPipe();

        for (auto& pipe : pipes) {
            pipe.move(-2.0f, 0);

            if (pipe.getPosition().x < -pipe.getGlobalBounds().width) {
                pipes.erase(pipes.begin());
                pipes.erase(pipes.begin());
                break;
            }
        }

        checkCollisions();
        updateScores();
    }

    void render() {
        window.clear(sf::Color::Black);

        for (auto& bird : birds) {
            window.draw(bird.sprite);
        }

        for (auto& pipe : pipes) {
            window.draw(pipe);
        }

        drawScores();

        window.display();
    }

    void spawnPipe() {
        int pipeHeight = rand() % (windowHeight - pipeGap - 200) + 100;

        sf::Sprite pipeUp;
        sf::Texture pipeUpTexture;
        if (!pipeUpTexture.loadFromFile("pipe1.png")) {
            std::cout << "Failed to load pipe top texture." << std::endl;
            return;
        }
        pipeUp.setTexture(pipeUpTexture);
        pipeUp.setScale(1, -1);
        pipeUp.setPosition(windowWidth, pipeHeight - pipeUp.getGlobalBounds().height);
        pipes.push_back(pipeUp);

        sf::Sprite pipeDown;
        sf::Texture pipeDownTexture;
        if (!pipeDownTexture.loadFromFile("pipe2.png")) {
            std::cout << "Failed to load pipe bottom texture." << std::endl;
            return;
        }
        pipeDown.setTexture(pipeDownTexture);
        pipeDown.setPosition(windowWidth, pipeHeight + pipeGap);
        pipes.push_back(pipeDown);
    }



    void checkCollisions() {
        for (auto& bird : birds) {
            sf::FloatRect birdBounds = bird.sprite.getGlobalBounds();

            for (auto& pipe : pipes) {
                sf::FloatRect pipeBounds = pipe.getGlobalBounds();

                if (birdBounds.intersects(pipeBounds)) {
                    // Game over
                    saveHighScores();
                    resetGame();
                    return;
                }
            }
        }
    }

    void updateScores() {
        for (auto& bird : birds) {
            if (pipeCounted(bird)) {
                bird.score++;
            }
        }
    }

    bool pipeCounted(const Bird& bird) {
        sf::FloatRect birdBounds = bird.sprite.getGlobalBounds();

        for (auto& pipe : pipes) {
            sf::FloatRect pipeBounds = pipe.getGlobalBounds();

            if (pipe.getPosition().x + pipe.getGlobalBounds().width < bird.sprite.getPosition().x && !pipeBounds.intersects(birdBounds)) {
                return true;
            }
        }

        return false;
    }

    void drawScores() {
        sf::Text text;
        text.setFont(font);
        text.setCharacterSize(20);
        text.setFillColor(sf::Color::White);
        text.setString("High Scores:");
        text.setPosition(windowWidth / 2 + 10, 40);
        window.draw(text);

        std::sort(highScores.begin(), highScores.end(), [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
            return a.second > b.second;
            });

        int count = 0;
        for (auto& score : highScores) {
            text.setString(score.first + ": " + std::to_string(score.second));
            text.setPosition(windowWidth / 2 + 10, (count + 1) * 30 + 40);
            window.draw(text);
            count++;

            if (count >= 5)
                break;
        }
    }


    void saveHighScores() {
        std::ofstream outputFile(highScoresFile);
        if (outputFile.is_open()) {
            for (auto& score : highScores) {
                outputFile << score.first << " " << score.second << "\n";
            }
            outputFile.close();
        }
        else {
            std::cout << "Failed to open high scores file for writing." << std::endl;
        }
    }

    void loadHighScores() {
        std::ifstream inputFile(highScoresFile);
        if (inputFile.is_open()) {
            std::string name;
            int score;
            while (inputFile >> name >> score) {
                highScores.push_back(std::make_pair(name, score));
            }
            inputFile.close();
        }
        else {
            std::cout << "Failed to open high scores file for reading." << std::endl;
        }
    }

    void resetGame() {
        birds.clear();
        pipes.clear();
        loadHighScores();

        Bird bird1;
        bird1.sprite.setTexture(birdTexture);
        bird1.sprite.setPosition(windowWidth / 4, windowHeight / 2);
        bird1.velocity = sf::Vector2f(0, 0);
        bird1.score = 0;
        birds.push_back(bird1);

        Bird bird2;
        bird2.sprite.setTexture(birdTexture);
        bird2.sprite.setPosition(windowWidth * 3 / 4, windowHeight / 2);
        bird2.velocity = sf::Vector2f(0, 0);
        bird2.score = 0;
        birds.push_back(bird2);
    }
};

int main() {
    sf::Texture birdTexture;
    if (!birdTexture.loadFromFile("bird.png")) {
        std::cout << "Failed to load bird texture." << std::endl;
        return -1;
    }

    sf::Texture pipeTexture;
    if (!pipeTexture.loadFromFile("pipe.png")) {
        std::cout << "Failed to load pipe texture." << std::endl;
        return -1;
    }

    Game game;
    game.run();

    return 0;
}
