#pragma once 

#include "mesh.h"

#include <vector>

struct Geometry{    // a container called Geometry that holds two lists:
    std::vector<Vertex> vertices;   // 3D: Direction it faces (normal), and 2D position (texture coordinates).
    std::vector<unsigned int> indices;  // A list of instructions on how to connect those dots. Like to make a triangle
};

Geometry makeCylinder(unsigned int segments);
Geometry makeBox();
Geometry makeSphere(unsigned int slices, unsigned int stacks);