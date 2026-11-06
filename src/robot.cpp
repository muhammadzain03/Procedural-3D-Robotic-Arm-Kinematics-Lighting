#include <glad/gl.h>

#include "robot.h"

#include "geometry.h"
#include "mesh.h"
#include <cmath>

#include <SFML/System/Clock.hpp>
#include <SFML/Window.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>
#include <stdexcept>

int main(int argc, char* argv[])
{
    try {
        /*
        Opens a 900x700 window using SFML configured for an OpenGL
        4.0 Core profile with a 24-bit depth buffer.
        */
        sf::ContextSettings settings;
        settings.majorVersion = 4;
        settings.minorVersion = 0;
        settings.attributeFlags = sf::ContextSettings::Core;
        settings.depthBits = 24;    // Why 24? So, the 3D object know whats infront of what...

        sf::Window window(sf::VideoMode({900, 700}),
                          "Robotic arm",
                          sf::Style::Default,
                          sf::State::Windowed,
                          settings);

        if (!gladLoadGL(sf::Context::getFunction)) {
            std::cerr << "Failed to initialize GLAD\n";
            return 1;
        }

        // Makes OpenGL keep the closer surface when two triangles overlap, so the back of the cylinder doesn't draw through the front.
        glEnable(GL_DEPTH_TEST);
        // Drops triangles that face away. The caps and walls have to wind outward or they vanish.
        glEnable(GL_CULL_FACE);
        // Sets the colour glClear fills the window with every frame.
        glClearColor(0.12f, 0.13f, 0.15f, 1.0f); 

        const Geometry plainCylinderShape = makeCylinder(48); // 48 --> How many slices go around the wall.
        const Geometry texturedCylinderShape = makeCylinder(48, 4.0f, 2.0f);
        const Geometry discShape = makeDisc(48);
        const Geometry sphereShape = makeSphere(32, 16);
        const Geometry boxShape = makeBox();

        const std::filesystem::path shaderDirectory =
            std::filesystem::absolute(argv[0]).parent_path() / "shaders";   // argv[0] is the path of robot.exe
        
        const std::filesystem::path assetDirectory =
            std::filesystem::absolute(argv[0]).parent_path() / "assets";   // argv[0] is the path of robot.exe
        
        const std::string linkageTexture = (assetDirectory / "texture_arm_linkage.png").string();
        const std::string baseTopTexture = (assetDirectory / "texture_base_top.png").string();
        
        Material linkMaterial;
        linkMaterial.ambient = glm::vec3(0.15f);
        linkMaterial.diffuse = glm::vec3(0.70f);
        linkMaterial.specular = glm::vec3(0.25f);
        linkMaterial.shininess = 32.0f;
        linkMaterial.lightingEnabled = true;

        Material jointMaterial;
        jointMaterial.ambient = glm::vec3(0.08f);
        jointMaterial.diffuse = glm::vec3(0.35f);
        jointMaterial.specular = glm::vec3(0.80f);
        jointMaterial.shininess = 96.0f;
        jointMaterial.lightingEnabled = true;

        Material flatMaterial;
        flatMaterial.ambient = glm::vec3(0.18f);
        flatMaterial.diffuse = glm::vec3(0.65f);
        flatMaterial.specular = glm::vec3(0.05f);
        flatMaterial.shininess = 8.0f;
        flatMaterial.lightingEnabled = true;

        const std::string vertexShader = (shaderDirectory / "mesh.vert").string();
        const std::string fragmentShader = (shaderDirectory / "mesh.frag").string();
        Mesh cylinderPlain(plainCylinderShape.vertices, plainCylinderShape.indices, vertexShader, fragmentShader, {}, flatMaterial);
        Mesh cylinderTextured(texturedCylinderShape.vertices, texturedCylinderShape.indices, vertexShader, fragmentShader, linkageTexture, linkMaterial);
        Mesh discTextured(discShape.vertices, discShape.indices, vertexShader, fragmentShader, baseTopTexture, linkMaterial);
        Mesh sphere(sphereShape.vertices, sphereShape.indices, vertexShader, fragmentShader, {}, jointMaterial);
        Mesh box(boxShape.vertices, boxShape.indices, vertexShader, fragmentShader, {}, flatMaterial);
        
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

        const glm::vec3 lightPositionWorld(4.0f, 9.0f, 6.0f);
        const glm::vec3 lightPositionView = glm::vec3(view * glm::vec4(lightPositionWorld, 1.0f));

        const float nearPlane = 0.1f;
        const float farPlane = 60.0f;
        float baseDeg = 0.0f;
        float shoulderDeg = 0.0f;
        float elbowDeg = 0.0f;
        float wristDeg = 0.0f;
        float jawDeg = 0.0f;
        const float speed = 75.0f;
        sf::Clock frameClock;

        /*
        The outer loop runs until the window closes. The inner loop empties SFML’s
        event queue. Closing the window, or pressing Escape, ends the program. The
        viewport is set from the window size every frame. Joint keys are polled
        each frame so the arm moves continuously.
        */
        while (window.isOpen()) {
            while (const std::optional event = window.pollEvent()) {
                if (event->is<sf::Event::Closed>()) {
                    window.close();
                } else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                    if (key->code == sf::Keyboard::Key::Escape) {
                        window.close();
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
            // glm::perspective is the projection matrix: a 45° vertical field of view,
            // that aspect, and near and far planes at 0.1 and 60.
            const glm::mat4 projection = glm::perspective(fieldOfView, aspect, nearPlane, farPlane);
            const float dt = frameClock.restart().asSeconds();

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
                baseDeg -= speed * dt;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
                baseDeg += speed * dt;
            }
            baseDeg = std::fmod(baseDeg, 360.0f);
            if (baseDeg < 0.0f) {
                baseDeg += 360.0f;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
                shoulderDeg += speed * dt;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
                shoulderDeg -= speed * dt;
            }
            shoulderDeg = std::clamp(shoulderDeg, -10.0f, 78.0f);

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
                elbowDeg += speed * dt;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F)) {
                elbowDeg -= speed * dt;
            }
            elbowDeg = std::clamp(elbowDeg, -150.0f, 10.0f);

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::T)) {
                wristDeg += speed * dt;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::G)) {
                wristDeg -= speed * dt;
            }
            wristDeg = std::clamp(wristDeg, -90.0f, 90.0f);

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::O)) {
                jawDeg += speed * dt;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::C)) {
                jawDeg -= speed * dt;
            }
            // Positive opens past parallel. The inner faces meet near -22.6°,
            // so -22° stops just short of the tips crossing.
            jawDeg = std::clamp(jawDeg, -22.0f, 45.0f);

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
                baseDeg = 0.0f;
                shoulderDeg = 0.0f;
                elbowDeg = 0.0f;
                wristDeg = 0.0f;
                jawDeg = 0.0f;
            }

            // glViewport matches the drawing rectangle to the window.
            glViewport(0, 0, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));
            // glClear wipes the colour buffer and the depth buffer, so the last frame is gone.
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // The Kinematics Chain
            const glm::vec3 xAxis(1.0f, 0.0f, 0.0f);
            const glm::vec3 yAxis(0.0f, 1.0f, 0.0f);

            glm::mat4 base = glm::rotate(glm::mat4(1.0f), glm::radians(baseDeg), yAxis);
            cylinderPlain.draw(base * glm::scale(glm::mat4(1.0f),
                                        glm::vec3(arm::baseRadius, arm::baseHeight, arm::baseRadius)), view, projection, lightPositionView);
            discTextured.draw(base
                                  * glm::translate(glm::mat4(1.0f),
                                                   glm::vec3(0.0f, arm::baseHeight + arm::discGap, 0.0f))
                                  * glm::scale(glm::mat4(1.0f),
                                               glm::vec3(arm::baseRadius, 1.0f, arm::baseRadius)),
                              view, projection, lightPositionView);

            // (Below) glm::rotate(glm::translate(parent, offset), angle, axis)
            // is parent * T * R. The shoulder axis is X, (1, 0, 0)
            glm::mat4 shoulder = glm::rotate(
                glm::translate(base, glm::vec3(0.0f, arm::baseHeight, 0.0f)),
                glm::radians(shoulderDeg), xAxis);
            sphere.draw(shoulder * glm::scale(glm::mat4(1.0f),
                                        glm::vec3(arm::shoulderRadius)), view, projection, lightPositionView);
            cylinderTextured.draw(shoulder * glm::scale(glm::mat4(1.0f),
                                        glm::vec3(arm::upperArmRadius, arm::upperArmLength, arm::upperArmRadius)), view, projection, lightPositionView);

            glm::mat4 elbow = glm::rotate(
                glm::translate(shoulder, glm::vec3(0.0f, arm::upperArmLength, 0.0f)),
                glm::radians(elbowDeg), xAxis);
            sphere.draw(elbow * glm::scale(glm::mat4(1.0f),
                                        glm::vec3(arm::elbowRadius)), view, projection, lightPositionView);
            cylinderTextured.draw(elbow * glm::scale(glm::mat4(1.0f),
                                        glm::vec3(arm::forearmRadius, arm::forearmLength, arm::forearmRadius)), view, projection, lightPositionView);

            glm::mat4 wrist = glm::rotate(
                glm::translate(elbow, glm::vec3(0.0f, arm::forearmLength, 0.0f)),
                glm::radians(wristDeg), xAxis);
            sphere.draw(wrist * glm::scale(glm::mat4(1.0f),
                                        glm::vec3(arm::wristRadius)), view, projection, lightPositionView);
            box.draw(wrist
                         * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, arm::palmLength * 0.5f, 0.0f))
                         * glm::scale(glm::mat4(1.0f), glm::vec3(arm::palmThickness, arm::palmLength, arm::palmHeight)),
                     view, projection, lightPositionView);

            for (float side : {1.0f, -1.0f}) {
                glm::mat4 stem = glm::translate(wrist, glm::vec3(0.0f, arm::palmLength, side * arm::palmHeight * 0.5f));
                cylinderPlain.draw(stem * glm::scale(glm::mat4(1.0f),
                                    glm::vec3(arm::stemRadius, arm::stemLength, arm::stemRadius)), view, projection, lightPositionView);

                glm::mat4 fingerJoint = glm::translate(stem, glm::vec3(0.0f, arm::stemLength, 0.0f));
                sphere.draw(fingerJoint * glm::scale(glm::mat4(1.0f),
                                    glm::vec3(arm::fingerJointRadius)), view, projection, lightPositionView);

                glm::mat4 finger = glm::rotate(fingerJoint, glm::radians(side * jawDeg), xAxis);
                box.draw(finger
                             * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, arm::fingerLength * 0.5f, 0.0f))
                             * glm::scale(glm::mat4(1.0f), glm::vec3(arm::fingerThickness, arm::fingerLength, arm::fingerHeight)),
                         view, projection, lightPositionView);
            }

            // window.display puts the finished image on screen.
            window.display();
        }    
    } catch (const std::exception& error) {
        std::cerr << "Application failed: " << error.what() << std::endl;
        return 1;
    }

    return 0;
}
