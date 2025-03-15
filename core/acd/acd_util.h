//
//  acd_util.h
//  VACD
//
//  Created by Dmitri Wamback on 2025-03-13.
//

#ifndef acd_util_h
#define acd_util_h
#include <set>
#include <map>
#include <queue>

typedef struct approximateConvexDecompositionTriangle {
    uint32_t indices[3];
    std::set<uint32_t> neighbors;
} ACDTriangle;

typedef struct approximateConvexDecompositionIndexedMesh {
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    uint32_t vao, vbo, ibo;
    glm::vec3 color;
} ACDIndexedMesh;

typedef struct approximateConvexHull {
    std::vector<glm::vec3> vertices;
    std::vector<std::array<int, 3>> faces;
} ACDConvexHull;

std::vector<float> GetTransformedVertices(ACDIndexedMesh mesh, glm::mat4 model) {
    
    std::vector<float> projectedVertices = std::vector<float>();
    
    for (int i = 0; i < mesh.vertices.size()/8; i++) {
        glm::vec3 vertex = glm::vec3(mesh.vertices[i * 8], mesh.vertices[i * 8 + 1], mesh.vertices[i * 8 + 2]);
        glm::vec3 projected = glm::vec3(model * glm::vec4(vertex, 1.0));
        projectedVertices.push_back(projected.x);
        projectedVertices.push_back(projected.y);
        projectedVertices.push_back(projected.z);
    }
    return projectedVertices;
}

#endif /* acd_util_h */
