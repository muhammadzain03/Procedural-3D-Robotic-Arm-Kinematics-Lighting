#include <glad/gl.h>

#include "robot.h"

#include "geometry.h"
#include "mesh.h"

#include <SFML/Window.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>
#include <stdexcept>

int main(int argc, char* argv[])
{
    try {
        sf::ContextSettings settings;
        settings.majorVersion = 4;
        settings.minorVersion = 0;
        settings.attributeFlags = sf::ContextSettings::Core;
        settings.depthBits = 24;

        sf::Window window(sf::VideoMode({900, 700}),
                          "Unit cylinder",
                          sf::Style::Default,
                          sf::State::Windowed,
                          settings);

        if (!gladLoadGL(sf::Context::getFunction)) {
            std::cerr << "Failed to initialize GLAD\n";
            return 1;
        }

        glEnable(GL_DEPTH_TEST);
        glClearColor(0.12f, 0.13f, 0.15f, 1.0f);

        const Geometry cylinder = makeCylinder(48);
        const std::filesystem::path shaderDirectory =
            std::filesystem::absolute(argv[0]).parent_path() / "shaders";

        Material material;
        material.lightingEnabled = true;

        Mesh mesh(cylinder.vertices,
                  cylinder.indices,
                  (shaderDirectory / "mesh.vert").string(),
                  (shaderDirectory / "mesh.frag").string(),
                  {},
                  material);

        const glm::mat4 view = glm::lookAt(glm::vec3(2.6f, 1.7f, 2.4f),
                                           glm::vec3(0.0f, 0.5f, 0.0f),
                                           glm::vec3(0.0f, 1.0f, 0.0f));
        float yaw = 0.6f;

        while (window.isOpen()) {
            while (const std::optional event = window.pollEvent()) {
                if (event->is<sf::Event::Closed>()) {
                    window.close();
                } else if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                    glViewport(0, 0, static_cast<GLsizei>(resized->size.x), static_cast<GLsizei>(resized->size.y));
                } else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                    if (key->code == sf::Keyboard::Key::Escape) {
                        window.close();
                    } else if (key->code == sf::Keyboard::Key::Left) {
                        yaw += 0.15f;
                    } else if (key->code == sf::Keyboard::Key::Right) {
                        yaw -= 0.15f;
                    }
                }
            }

            if (!window.isOpen()) {
                break;
            }

            const sf::Vector2u size = window.getSize();
            const float aspect = size.y == 0
                ? 1.0f
                : static_cast<float>(size.x) / static_cast<float>(size.y);
            const glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 20.0f);
            const glm::mat4 model = glm::rotate(glm::mat4(1.0f), yaw, glm::vec3(0.0f, 1.0f, 0.0f));

            glViewport(0, 0, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            mesh.draw(model, view, projection);
            window.display();
        }
    } catch (const std::exception& error) {
        std::cerr << "Application failed: " << error.what() << std::endl;
        return 1;
    }

    return 0;
}
