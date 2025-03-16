//
//  object.h
//  VACD
//
//  Created by Dmitri Wamback on 2025-03-13.
//

#ifndef object_h
#define object_h

#include <future>

class RObject {
public:
    std::vector<Mesh> meshes;
    std::vector<Mesh> processedMeshes;
    
    int vao, vbo, ibo;
    glm::vec3 position, scale, rotation, color;
    
    static RObject* Create();
    virtual void Render(Shader shader) {}
    
    
    void Decompose(int maxClusters);
    
    glm::mat4 CreateModelMatrix();
    Mesh CreateOpenGLMesh(Mesh convexMesh);
    
private:
    ConvexHull ComputeConvexHull(const std::vector<glm::vec3>& points);
    std::vector<ConvexHull> ApproximateConvexDecomposition(const Mesh& mesh);
    bool IsNeighbor(const Triangle& T1, const Triangle& T2);
    void CollectConvexPiece(const std::vector<Triangle>& triangles, int startIndex, std::set<int>& visitedTriangles, std::set<int>& convexPiece, std::vector<glm::vec3>& points);
};



// ------------------------------------------------------------------------------------------------------------- //
// ComputeConvexHull //
// ------------------------------------------------------------------------------------------------------------- //

ConvexHull RObject::ComputeConvexHull(const std::vector<glm::vec3> &points) {
    
    ConvexHull hull;
    
    for (const auto& P : points) {
        hull.vertices.push_back(P);
    }
    hull.faces.push_back({0, 1, 2});
    
    return hull;
}



// ------------------------------------------------------------------------------------------------------------- //
// ApproximateConvexDecomposition //
// ------------------------------------------------------------------------------------------------------------- //

std::vector<ConvexHull> RObject::ApproximateConvexDecomposition(const Mesh& mesh) {
    
    std::vector<Triangle> triangles;
    std::vector<ConvexHull> convexHulls;
    
    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
        Triangle t;
        t.indices[0] = mesh.indices[i];
        t.indices[1] = mesh.indices[i + 1];
        t.indices[2] = mesh.indices[i + 2];
        triangles.push_back(t);
    }

    // Step 2: Form the neighborhood relationships
    for (size_t i = 0; i < triangles.size(); ++i) {
        for (size_t j = i + 1; j < triangles.size(); ++j) {

            if (IsNeighbor(triangles[i], triangles[j])) {
                triangles[i].neighbors.insert(j);
                triangles[j].neighbors.insert(i);
            }
        }
    }

    // Step 3: Greedily find convex sub-regions
    std::set<int> visitedTriangles;
    for (size_t i = 0; i < triangles.size(); ++i) {
        if (visitedTriangles.find(i) == visitedTriangles.end()) {
            
            std::vector<glm::vec3> points;
            std::set<int> convexPiece;
            CollectConvexPiece(triangles, i, visitedTriangles, convexPiece, points);
            
            ConvexHull hull = ComputeConvexHull(points);
            convexHulls.push_back(hull);
        }
    }

    return convexHulls;
}

// ------------------------------------------------------------------------------------------------------------- //
// CollectConvexPiece //
// ------------------------------------------------------------------------------------------------------------- //

void RObject::CollectConvexPiece(const std::vector<Triangle> &triangles, int startIndex, std::set<int> &visitedTriangles, std::set<int> &convexPiece, std::vector<glm::vec3> &points) {
    
    convexPiece.insert(startIndex);
    visitedTriangles.insert(startIndex);
    const Triangle& t = triangles[startIndex];
    
    for (int i = 0; i < 3; ++i) {
        points.push_back(glm::vec3(t.indices[i]));
    }
    
    for (int neighbor : t.neighbors) {
        if (visitedTriangles.find(neighbor) == visitedTriangles.end()) {
            CollectConvexPiece(triangles, neighbor, visitedTriangles, convexPiece, points);
        }
    }
}

bool RObject::IsNeighbor(const Triangle& T1, const Triangle& T2) {
    
    std::set<uint32_t> edges1 = {T1.indices[0], T1.indices[1], T1.indices[1], T1.indices[2], T1.indices[2], T1.indices[0]};
    std::set<uint32_t> edges2 = {T2.indices[0], T2.indices[1], T2.indices[1], T2.indices[2], T2.indices[2], T2.indices[0]};
    
    for (auto edge1 : edges1) {
        for (auto edge2 : edges2) {
            if (edge1 == edge2) {
                return true;
            }
        }
    }
    return false;
    
}


// ------------------------------------------------------------------------------------------------------------- //
// Decompose //
// ------------------------------------------------------------------------------------------------------------- //

void RObject::Decompose(int maxClusters = 10) {
    
    for (Mesh mesh : meshes) {
        
    }
}



// ------------------------------------------------------------------------------------------------------------- //
// CreateModelMatrix //
// ------------------------------------------------------------------------------------------------------------- //

glm::mat4 RObject::CreateModelMatrix() {
    
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 translationMatrix = glm::mat4(1.0f);
    translationMatrix = glm::translate(translationMatrix, position);
    
    glm::mat4 scaleMatrix = glm::mat4(1.0f);
    scaleMatrix = glm::scale(scaleMatrix, scale);
    
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1, 0, 0)) *
                               glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0, 1, 0)) *
                               glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0, 0, 1));
    
    model = translationMatrix * rotationMatrix * scaleMatrix;
    
    return model;
}

Mesh RObject::CreateOpenGLMesh(Mesh convexMesh) {
    glGenVertexArrays(1, &convexMesh.vao);
    glGenBuffers(1, &convexMesh.vbo);
    glGenBuffers(1, &convexMesh.ibo);

    glBindVertexArray(convexMesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, convexMesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, convexMesh.vertices.size() * sizeof(float), convexMesh.vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, convexMesh.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, convexMesh.indices.size() * sizeof(uint32_t), convexMesh.indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    
    return convexMesh;
}

#endif /* object_h */
