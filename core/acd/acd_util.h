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

std::vector<std::pair<uint32_t, uint32_t>> GetEdges(const ACDTriangle& T) {
    // Create a vector to store the edges of the triangle
    std::vector<std::pair<uint32_t, uint32_t>> edges;

    // Ensure the vertices are ordered such that the smaller index comes first
    edges.push_back({std::min(T.indices[0], T.indices[1]), std::max(T.indices[0], T.indices[1])}); // Edge between v0 and v1
    edges.push_back({std::min(T.indices[1], T.indices[2]), std::max(T.indices[1], T.indices[2])}); // Edge between v1 and v2
    edges.push_back({std::min(T.indices[2], T.indices[0]), std::max(T.indices[2], T.indices[0])}); // Edge between v2 and v0

    return edges;
}

bool HasSharedEdge(const ACDTriangle& tri1, const ACDTriangle& tri2) {
    auto edges1 = GetEdges(tri1);
    auto edges2 = GetEdges(tri2);

    // Check if any edge from tri1 matches any edge from tri2
    for (const auto& edge1 : edges1) {
        for (const auto& edge2 : edges2) {
            if (edge1 == edge2) {
                return true;
            }
        }
    }
    return false;
}

#endif /* acd_util_h */
