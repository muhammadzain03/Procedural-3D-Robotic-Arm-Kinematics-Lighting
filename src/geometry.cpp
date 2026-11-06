#include "geometry.h"

#include <cmath>
#include <vector>

namespace {
    constexpr float pi = 3.14159265358979323846f;

    Vertex makeVertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& texCoord) {
        Vertex vertex;
        vertex.position = position;
        vertex.normal = normal;
        vertex.texCoord = texCoord;
        return vertex;
    }
}

Geometry makeCylinder(unsigned int segments, float aroundRepeats, float alongRepeats) {
    Geometry geometry;
    // Side wall. i runs through segments inclusive so the last column
    // repeats the first position but carries u = 1, which closes the seam.
    for (unsigned int i = 0; i <= segments; ++i) {
        const float u = static_cast<float>(i) / static_cast<float>(segments);
        const float theta = 2.0f * pi * u;
        const glm::vec3 dir(std::cos(theta), 0.0f, std::sin(theta));
        geometry.vertices.push_back(makeVertex(dir, dir, {u * aroundRepeats, 0.0f}));
        geometry.vertices.push_back(makeVertex(dir + glm::vec3(0.0f, 1.0f, 0.0f),
                                               dir,
                                               {u * aroundRepeats, alongRepeats}));
    }
    for (unsigned int i = 0; i < segments; ++i) {
        const unsigned int bottom0 = i * 2;
        const unsigned int top0 = bottom0 + 1;
        const unsigned int bottom1 = bottom0 + 2;
        const unsigned int top1 = bottom0 + 3;
        geometry.indices.insert(geometry.indices.end(),
                                {bottom0, top0, bottom1,
                                 top0, top1, bottom1});
    }
    // Bottom cap. Flat normal pointing down. Fan winds opposite the top
    // so this face points outward.
    const unsigned int bottomCenter = static_cast<unsigned int>(geometry.vertices.size());
    geometry.vertices.push_back(makeVertex({0.0f, 0.0f, 0.0f},
                                           {0.0f, -1.0f, 0.0f},
                                           {0.5f, 0.5f}));
    for (unsigned int i = 0; i <= segments; ++i) {
        const float theta = 2.0f * pi * static_cast<float>(i) / static_cast<float>(segments);
        const float c = std::cos(theta);
        const float s = std::sin(theta);
        geometry.vertices.push_back(makeVertex({c, 0.0f, s},
                                               {0.0f, -1.0f, 0.0f},
                                               {0.5f + 0.5f * c, 0.5f + 0.5f * s}));
    }
    const unsigned int bottomRim = bottomCenter + 1;
    for (unsigned int i = 0; i < segments; ++i) {
        geometry.indices.insert(geometry.indices.end(),
                                {bottomCenter, bottomRim + i, bottomRim + i + 1});
    }
    // Top cap. Flat normal pointing up. Centre is at y = 1.
    const unsigned int topCenter = static_cast<unsigned int>(geometry.vertices.size());
    geometry.vertices.push_back(makeVertex({0.0f, 1.0f, 0.0f},
                                           {0.0f, 1.0f, 0.0f},
                                           {0.5f, 0.5f}));
    for (unsigned int i = 0; i <= segments; ++i) {
        const float theta = 2.0f * pi * static_cast<float>(i) / static_cast<float>(segments);
        const float c = std::cos(theta);
        const float s = std::sin(theta);
        geometry.vertices.push_back(makeVertex({c, 1.0f, s},
                                               {0.0f, 1.0f, 0.0f},
                                               {0.5f + 0.5f * c, 0.5f + 0.5f * s}));
    }
    const unsigned int topRim = topCenter + 1;
    for (unsigned int i = 0; i < segments; ++i) {
        geometry.indices.insert(geometry.indices.end(),
                                {topCenter, topRim + i + 1, topRim + i});
    }
    return geometry;
}


Geometry makeBox() {
    Geometry geometry;

    // 6 faces, 4 vertices each. Corners are duplicated because each face
    // has its own normal. Order is bottom-left, bottom-right, top-right, top-left
    // as seen from outside. Half-extent is 0.5, so the box is 1 x 1 x 1.
    const glm::vec3 positions[6][4] = {
        {{ -0.5f, -0.5f,  0.5f }, {  0.5f, -0.5f,  0.5f }, {  0.5f,  0.5f,  0.5f }, { -0.5f,  0.5f,  0.5f }},
        {{  0.5f, -0.5f, -0.5f }, { -0.5f, -0.5f, -0.5f }, { -0.5f,  0.5f, -0.5f }, {  0.5f,  0.5f, -0.5f }},
        {{  0.5f, -0.5f,  0.5f }, {  0.5f, -0.5f, -0.5f }, {  0.5f,  0.5f, -0.5f }, {  0.5f,  0.5f,  0.5f }},
        {{ -0.5f, -0.5f, -0.5f }, { -0.5f, -0.5f,  0.5f }, { -0.5f,  0.5f,  0.5f }, { -0.5f,  0.5f, -0.5f }},
        {{ -0.5f,  0.5f,  0.5f }, {  0.5f,  0.5f,  0.5f }, {  0.5f,  0.5f, -0.5f }, { -0.5f,  0.5f, -0.5f }},
        {{ -0.5f, -0.5f, -0.5f }, {  0.5f, -0.5f, -0.5f }, {  0.5f, -0.5f,  0.5f }, { -0.5f, -0.5f,  0.5f }}
    };
    const glm::vec3 normals[6] = {
        { 0.0f,  0.0f,  1.0f },
        { 0.0f,  0.0f, -1.0f },
        { 1.0f,  0.0f,  0.0f },
        {-1.0f,  0.0f,  0.0f },
        { 0.0f,  1.0f,  0.0f },
        { 0.0f, -1.0f,  0.0f }
    };

    for (int face = 0; face < 6; ++face){
        const unsigned int first = static_cast<unsigned int>(geometry.vertices.size());
        geometry.vertices.push_back(makeVertex(positions[face][0], normals[face], {0.0f, 0.0f}));
        geometry.vertices.push_back(makeVertex(positions[face][1], normals[face], {1.0f, 0.0f}));
        geometry.vertices.push_back(makeVertex(positions[face][2], normals[face], {1.0f, 1.0f}));
        geometry.vertices.push_back(makeVertex(positions[face][3], normals[face], {0.0f, 1.0f}));
        geometry.indices.insert(geometry.indices.end(), 
                                {first, first + 1, first + 2,
                                 first, first + 2, first + 3});
    }

    return geometry;
}


Geometry makeSphere(unsigned int slices, unsigned int stacks) {
    Geometry geometry;
    geometry.vertices.reserve(static_cast<std::size_t>(stacks + 1) * (slices + 1));
    geometry.indices.reserve(static_cast<std::size_t>(stacks) * slices * 6);

    for (unsigned int stack = 0; stack <= stacks; ++stack){
        const float v = static_cast<float>(stack) / static_cast<float>(stacks);
        const float theta = pi * v;
        const float sinTheta = std::sin(theta);
        const float cosTheta = std::cos(theta);
        

        for (unsigned int slice = 0; slice <= slices; ++slice){
            const float u = static_cast<float>(slice) / static_cast<float>(slices);
            const float phi = 2.0f * pi * u;
            const glm::vec3 normal(sinTheta * std::cos(phi),
                                   cosTheta, 
                                   sinTheta * std::sin(phi));
            geometry.vertices.push_back(makeVertex(normal, normal, {u, 1.0f - v}));
        }
    }

    const unsigned int rowLength = slices + 1;
    for(unsigned int stack = 0; stack < stacks; ++stack){
        for(unsigned int slice = 0; slice < slices; ++slice){
            const unsigned int topLeft = stack * rowLength + slice;
            const unsigned int bottomLeft = topLeft + rowLength;
            geometry.indices.insert(geometry.indices.end(),
                                   {topLeft, topLeft + 1, bottomLeft,
                                    topLeft + 1, bottomLeft + 1, bottomLeft});
        }
    }

    return geometry;
}

Geometry makeDisc(unsigned int segments) {
    Geometry geometry;
    // Unit disc on the XZ plane at y = 0. Normal is +Y.
    // Fan order matches the cylinder top cap
    geometry.vertices.push_back(makeVertex({0.0f, 0.0f, 0.0f},
                                           {0.0f, 1.0f, 0.0f},
                                           {0.5f, 0.5f}));
    for (unsigned int i = 0; i <= segments; ++i) {
        const float theta = 2.0f * pi * static_cast<float>(i) / static_cast<float>(segments);
        const float c = std::cos(theta);
        const float s = std::sin(theta);
        geometry.vertices.push_back(makeVertex({c, 0.0f, s},
                                               {0.0f, 1.0f, 0.0f},
                                               {0.5f + 0.5f * c, 0.5f + 0.5f * s}));
    }
    const unsigned int rim = 1;
    for (unsigned int i = 0; i< segments; ++i) {
        geometry.indices.insert(geometry.indices.end(),
                               {0, rim + i + 1, rim + i});
    }
    return geometry;
}