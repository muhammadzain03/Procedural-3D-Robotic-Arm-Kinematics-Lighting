#include <glad/gl.h>

#include "robot.h"

#include "geometry.h"
#include "mesh.h"
#include <cmath>

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
        /*
        Opens an 900x700 window using SFML configured for OpenGL 
        profile with 24-bit depth buffer. 
        */
        sf::ContextSettings settings;
        settings.majorVersion = 4;
        settings.minorVersion = 0;
        settings.attributeFlags = sf::ContextSettings::Core;
        settings.depthBits = 24;    // Why 24? So, the 3D object know whats infront of what...

        sf::Window window(sf::VideoMode({900, 700}),
                          "Unit cylinder",
                          sf::Style::Default,
                          sf::State::Windowed,
                          settings);

        if (!gladLoadGL(sf::Context::getFunction)) {
            std::cerr << "Failed to initialize GLAD\n";
            return 1;
        }

        // Makes OpenGL keep the closer surface when two triangles overlap, so the back of the cylinder doesn't draw through the front.
        glEnable(GL_DEPTH_TEST);
        // Sets the BG color used the next time window is created.
        glClearColor(0.12f, 0.13f, 0.15f, 1.0f); 

        const Geometry cylinder = makeCylinder(48); // 48 --> How many slices go around the wall.
        const std::filesystem::path shaderDirectory =
            std::filesystem::absolute(argv[0]).parent_path() / "shaders";   // argv[0] is the path of robot.exe

        Material material;
        material.lightingEnabled = true;    // turns the Phong shading on

        Mesh mesh(cylinder.vertices,
                  cylinder.indices,
                  (shaderDirectory / "mesh.vert").string(),
                  (shaderDirectory / "mesh.frag").string(),
                  {},
                  material);
        
        // --- Dynamic Camera Setup ---
        // 8 / sin(22.5°) is about 20.9. That is how far back a 45° camera must sit to fit a sphere of radius 8.

        const float fieldOfView = glm::radians(45.0f);
        const float modelRadius = 8.0f;
        const glm::vec3 target(0.0f, 3.0f, 0.0f);  // Looks at the middle of the Cylinder
        const float fitDistance = modelRadius / std::sin(fieldOfView * 0.5f);

        // only a slight raise and a slight shift to the side, then it is scaled to that distance
        const glm::vec3 viewDirection = glm::normalize(glm::vec3(0.30f, 0.15f, 1.0f));
        // The eye lands near (6, 6, 20), so the base roll will be visible later instead of a flat front view.
        const glm::mat4 view = glm::lookAt(target + viewDirection * fitDistance,  // Eye
                                           target,
                                           glm::vec3(0.0f, 1.0f, 0.0f));    // Up

        const float nearPlane = 0.1f;
        const float farPlane = 60.0f;
        // Cylinder's starting rotation aroudn Y (in radians) - Changes later by left and right keys
        float yaw = 0.6f;
        
        /*
        The outer loop runs until the window closes. The inner loop empties SFML’s 
        event queue. Closing the window, or pressing Escape, ends the program. A 
        resize updates the drawable area to the new width and height. Left and 
        right add or subtract 0.15 radians from yaw, about 8.6 degrees per press.
        */
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
            // aspect is width divided by height.
            // A height of 0 would divide by zero, so that case uses 1
            const float aspect = size.y == 0
                ? 1.0f
                : static_cast<float>(size.x) / static_cast<float>(size.y);
            const glm::mat4 projection = glm::perspective(fieldOfView, aspect, nearPlane, farPlane);
            const glm::mat4 model = glm::rotate(glm::mat4(1.0f), yaw, glm::vec3(0.0f, 1.0f, 0.0f));

            /*
            Above code explaination
            glm::perspective is GLM’s projection matrix: a 45° vertical field of view,
            that aspect, and near and far planes at 0.1 and 20. model is a fresh rotation
            around Y by the current yaw.
            */

            // glViewport matches the drawing rectangle to the window.
            glViewport(0, 0, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));
            // glClear wipes the colour buffer and the depth buffer, so the last frame is gone.
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            mesh.draw(model, view, projection);
            // window.display puts the finished image on screen.
            window.display();
        }    
    } catch (const std::exception& error) {
        std::cerr << "Application failed: " << error.what() << std::endl;
        return 1;
    }

    return 0;
}
