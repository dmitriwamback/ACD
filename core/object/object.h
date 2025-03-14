//
//  object.h
//  VACD
//
//  Created by Dmitri Wamback on 2025-03-13.
//

#ifndef object_h
#define object_h

class RObject {
public:
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    std::vector<ACDIndexedMesh> meshes;
    
    int vao, vbo, ibo;
    glm::vec3 position, scale, rotation, color;
    
    RObject* Create();
    virtual void Render(/*Shader shader*/) {}
    void Decompose(int maxClusters);
private:
    std::vector<ACDTriangle> ConstructAdjacency();
    std::vector<std::vector<ACDTriangle>> ACD(std::vector<ACDTriangle>& triangles, int maxClusters);
    ACDIndexedMesh CreateConvexMesh(const std::vector<ACDTriangle>& cluster);
};



std::vector<ACDTriangle> RObject::ConstructAdjacency() {
    
    std::vector<ACDTriangle> triangles;
    std::unordered_map<uint32_t, std::set<uint32_t>> adjacency;
    
    for (int i = 0; i < indices.size(); i += 3) {
        ACDTriangle triangle = {{indices[i], indices[i+1], indices[i+2]}, {}};
        triangles.push_back(triangle);
        
        adjacency[indices[i]].insert(i / 3);
        adjacency[indices[i+1]].insert(i / 3);
        adjacency[indices[i+2]].insert(i / 3);
    }
    
    for (int i = 0; i < triangles.size(); i++) {
        for (uint32_t vi : triangles[i].indices) {
            for (uint32_t n : adjacency[vi]) {
                if (n != i) triangles[i].neighbors.insert(n);
            }
        }
    }
    
    return triangles;
}

std::vector<std::vector<ACDTriangle>> RObject::ACD(std::vector<ACDTriangle>& triangles, int maxClusters) {
    
    std::set<uint32_t> unprocessed;
    for (int i = 0; i < triangles.size(); i++) {
        unprocessed.insert(i);
    }
    
    std::vector<std::vector<ACDTriangle>> convexClusters;
    
    return convexClusters;
}

ACDIndexedMesh RObject::CreateConvexMesh(const std::vector<ACDTriangle>& cluster) {
    
    ACDIndexedMesh m{};
    return m;
}

#endif /* object_h */
