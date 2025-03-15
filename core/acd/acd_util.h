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

#endif /* acd_util_h */
