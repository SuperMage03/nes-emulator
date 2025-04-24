// External Library Headers
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
// Project Headers
#include "nes-window-sfml.hpp"
#include "nes-sound-sfml.hpp"
#include "nes.hpp"
#include "controller.hpp"
// Debugging Headers
#ifdef DEBUG
#include "nes-debug-window.hpp"
#endif

int main(int argc, char* argv[]) {
    NESWindowSFML nes_window;
    sf::RenderWindow& window = nes_window.getWindow();

    NESSoundSFML nes_sound;

    NES nes;
    nes.connectDisplayWindow(nes_window);
    nes.connectSoundSystem(nes_sound);
    nes.loadCartridge("./tests/nestest.nes");

    Controller controller_one;
    nes.connectController(controller_one);    

    #ifdef DEBUG
    NESDebugWindow nes_debug_window;
    nes_debug_window.attachNES(&nes);
    #endif

    // run the program as long as the window is open
    while (window.isOpen()) {
        // check all the window's events that were triggered since the last iteration of the loop
        while (const std::optional event = window.pollEvent()) {
            // "close requested" event: we close the window
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            else if (const auto* key_pressed = event->getIf<sf::Event::KeyPressed>()) {
                switch (key_pressed->scancode) {
                    case sf::Keyboard::Scancode::Escape:
                        window.close();
                        break;
                    case sf::Keyboard::Scancode::Space:
                        nes.stepFrame();
                        break;
                    case sf::Keyboard::Scancode::A:
                        controller_one.pressButton(Controller::Button::A);
                        break;
                    case sf::Keyboard::Scancode::B:
                        controller_one.pressButton(Controller::Button::B);
                        break;
                    case sf::Keyboard::Scancode::Up:
                        controller_one.pressButton(Controller::Button::UP);
                        break;
                    case sf::Keyboard::Scancode::Down:
                        controller_one.pressButton(Controller::Button::DOWN);
                        break;
                    case sf::Keyboard::Scancode::Left:
                        controller_one.pressButton(Controller::Button::LEFT);
                        break;
                    case sf::Keyboard::Scancode::Right:
                        controller_one.pressButton(Controller::Button::RIGHT);
                        break;
                    case sf::Keyboard::Scancode::Enter:
                        controller_one.pressButton(Controller::Button::START);
                        break;
                    case sf::Keyboard::Scancode::Backspace:
                        controller_one.pressButton(Controller::Button::SELECT);
                        break;
                    default:
                        break;
                }
            }
            else if (const auto* key_released = event->getIf<sf::Event::KeyReleased>()) {
                switch (key_released->scancode) {
                    case sf::Keyboard::Scancode::A:
                        controller_one.releaseButton(Controller::Button::A);
                        break;
                    case sf::Keyboard::Scancode::B:
                        controller_one.releaseButton(Controller::Button::B);
                        break;
                    case sf::Keyboard::Scancode::Up:
                        controller_one.releaseButton(Controller::Button::UP);
                        break;
                    case sf::Keyboard::Scancode::Down:
                        controller_one.releaseButton(Controller::Button::DOWN);
                        break;
                    case sf::Keyboard::Scancode::Left:
                        controller_one.releaseButton(Controller::Button::LEFT);
                        break;
                    case sf::Keyboard::Scancode::Right:
                        controller_one.releaseButton(Controller::Button::RIGHT);
                        break;
                    case sf::Keyboard::Scancode::Enter:
                        controller_one.releaseButton(Controller::Button::START);
                        break;
                    case sf::Keyboard::Scancode::Backspace:
                        controller_one.releaseButton(Controller::Button::SELECT);
                        break;
                    default:
                        break;
                }
            }
            else if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                const float screenWidth = 256;
                const float screenHeight = 240;
                sf::Vector2u new_size = window.getSize();
                // setup my wanted aspect ratio
                float  heightRatio = screenHeight / screenWidth;
                float  widthRatio = screenWidth / screenHeight;
                if (new_size.y * widthRatio <= new_size.x) {
                    new_size.x = new_size.y * widthRatio;
                }
                else if (new_size.x * heightRatio <= new_size.y) {
                    new_size.y = new_size.x * heightRatio;
                }
                window.setSize(sf::Vector2u(new_size.x, new_size.y));
            }
        }

        nes.stepFrame();
        nes_sound.play();
        nes_window.render();

        #ifdef DEBUG
        nes_debug_window.update();
        #endif
    }
    return 0;
}
