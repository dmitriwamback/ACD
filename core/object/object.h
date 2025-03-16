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
    std::vector<ACDIndexedMesh> meshes;
    std::vector<ACDIndexedMesh> processedMeshes;
    
    int vao, vbo, ibo;
    glm::vec3 position, scale, rotation, color;
    
    static RObject* Create();
    virtual void Render(Shader shader) {}
    void Decompose(int maxClusters);

    std::vector<ACDTriangle> ConstructAdjacency(ACDIndexedMesh mesh);
    std::vector<std::vector<ACDTriangle>> ACD(std::vector<ACDTriangle> triangles, ACDIndexedMesh mesh, int maxClusters);
    ACDIndexedMesh CreateConvexMesh(const std::vector<ACDTriangle>& cluster, ACDIndexedMesh mesh);
    glm::mat4 CreateModelMatrix();
    float ComputeConvexity(const std::vector<ACDTriangle>& triangles, const std::set<uint32_t>& cluster, uint32_t newTriangleIndex, ACDIndexedMesh mesh);
    ACDConvexHull ComputeConvexHull(const std::vector<glm::vec3>& cluster);
    float DistanceFromHull(const ACDConvexHull& hull, const glm::vec3& point);
    glm::vec3 GetVertexFromIndex(ACDIndexedMesh& object, uint32_t index);
    ACDIndexedMesh CreateOpenGLMesh(ACDIndexedMesh mesh);
private:
    float PointLineDistance(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& cluster);
    float DistanceToCluster(const ACDTriangle& triangleA, const ACDTriangle& triangleB, ACDIndexedMesh mesh);
    float AngleBetweenVectors(glm::vec3 v1, glm::vec3 v2);
    
    std::set<uint32_t> ExpandCluster(const std::vector<ACDTriangle>& triangles, uint32_t idx, const ACDIndexedMesh& mesh, float convexityThreshold);
    void ProcessCluster(const std::set<uint32_t>& cluster, const std::vector<ACDTriangle>& triangles);
    std::vector<std::set<uint32_t>> GenerateClusters(const std::vector<ACDTriangle>& triangles, const ACDIndexedMesh& mesh);
    
    uint32_t FindFurthest(const std::vector<glm::vec3>& points, const glm::vec3& p1, const glm::vec3& p2);
};



// ------------------------------------------------------------------------------------------------------------- //
// ConstructAdjacency //
// ------------------------------------------------------------------------------------------------------------- //

std::vector<ACDTriangle> RObject::ConstructAdjacency(ACDIndexedMesh mesh) {
    
    std::vector<ACDTriangle> triangles;
    std::unordered_map<uint32_t, std::set<uint32_t>> adjacency;
    
    for (int i = 0; i < mesh.indices.size(); i += 3) {
        ACDTriangle triangle = {{mesh.indices[i], mesh.indices[i+1], mesh.indices[i+2]}, {}};
        triangles.push_back(triangle);
        
        adjacency[mesh.indices[i]].insert(i / 3);
        adjacency[mesh.indices[i+1]].insert(i / 3);
        adjacency[mesh.indices[i+2]].insert(i / 3);
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



// ------------------------------------------------------------------------------------------------------------- //
// GetVertexFromIndex //
// ------------------------------------------------------------------------------------------------------------- //

glm::vec3 RObject::GetVertexFromIndex(ACDIndexedMesh& object, uint32_t index) {
    uint32_t stride = 8;
    uint32_t vertex = index * stride;
    
    return glm::vec3(object.vertices[vertex], object.vertices[vertex+1], object.vertices[vertex+2]);
}



// ------------------------------------------------------------------------------------------------------------- //
// PointLineDistance //
// ------------------------------------------------------------------------------------------------------------- //

float RObject::PointLineDistance(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& cluster) {
    glm::vec3 L = p2 - p1;
    glm::vec3 P = cluster - p1;
    return glm::dot(glm::cross(L, P), L);
}



// ------------------------------------------------------------------------------------------------------------- //
// DistanceToCluster //
// ------------------------------------------------------------------------------------------------------------- //

float RObject::DistanceToCluster(const ACDTriangle& triangleA, const ACDTriangle& triangleB, ACDIndexedMesh mesh) {
    glm::vec3 centroidA = (GetVertexFromIndex(mesh, triangleA.indices[0]) + GetVertexFromIndex(mesh, triangleA.indices[1]) + GetVertexFromIndex(mesh, triangleA.indices[2])) / 3.0f;
    glm::vec3 centroidB = (GetVertexFromIndex(mesh, triangleB.indices[0]) + GetVertexFromIndex(mesh, triangleB.indices[1]) + GetVertexFromIndex(mesh, triangleB.indices[2])) / 3.0f;
    
    return glm::length(centroidA - centroidB);
}



// ------------------------------------------------------------------------------------------------------------- //
// AngleBetweenvectors //
// ------------------------------------------------------------------------------------------------------------- //

float RObject::AngleBetweenVectors(glm::vec3 v1, glm::vec3 v2) {
    float dotProduct = glm::dot(v1, v2);
    float magnitude1 = glm::length(v1);
    float magnitude2 = glm::length(v2);
    float cosTheta = dotProduct / (magnitude1 * magnitude2);
    return std::acos(glm::clamp(cosTheta, -1.0f, 1.0f));
}



// ------------------------------------------------------------------------------------------------------------- //
// DistanceFromHull //
// ------------------------------------------------------------------------------------------------------------- //

float RObject::DistanceFromHull(const ACDConvexHull& hull, const glm::vec3& point) {
    
    float mindst = FLT_MAX;
    if (hull.faces.empty()) {
        std::cout << "empty hull\n";
        return 0.0f;
    }
    
    for (const auto& face : hull.faces) {
        glm::vec3 A = hull.vertices[face[0]];
        glm::vec3 B = hull.vertices[face[1]];
        glm::vec3 C = hull.vertices[face[2]];
        
        glm::vec3 normal = glm::normalize(glm::cross(B - A, C - A));
        
        float dst = glm::dot(point - A, normal);
        mindst = std::min(dst, abs(dst));
    }
    
    return mindst;
}



// ------------------------------------------------------------------------------------------------------------- //
// FindFurthest //
// ------------------------------------------------------------------------------------------------------------- //

uint32_t RObject::FindFurthest(const std::vector<glm::vec3>& cluster, const glm::vec3& p1, const glm::vec3& p2) {
    
    uint32_t furthest = -1;
    float maxdst = -FLT_MAX;
    
    for (uint32_t i = 0; i < cluster.size(); i++) {
        float dst = PointLineDistance(p1, p2, cluster[i]);
        if (dst > maxdst) {
            maxdst = dst;
            furthest = i;
        }
    }
    
    return furthest;
}



// ------------------------------------------------------------------------------------------------------------- //
// ComputeConvexHull //
// ------------------------------------------------------------------------------------------------------------- //

ACDConvexHull RObject::ComputeConvexHull(const std::vector<glm::vec3>& clusters) {
    
    if (clusters.size() < 3) {
        std::cout << "Invalid\n";
        return {clusters, {}};
    }
        
    ACDConvexHull hull;
    
    auto minmaxX = std::minmax_element(clusters.begin(), clusters.end(), [](const glm::vec3& a, const glm::vec3& b) {
        return a.x < b.x;
    });
    
    glm::vec3 p1 = *minmaxX.first;
    glm::vec3 p2 = *minmaxX.second;
    
    hull.vertices.push_back(p1);
    hull.vertices.push_back(p2);
    
    std::vector<glm::vec3> leftSet, rightSet;

    for (const auto& cluster : clusters) {
        if (cluster == p1 || cluster == p2) continue;
        if (PointLineDistance(p1, p2, cluster) > 0) {
            leftSet.push_back(cluster);
        }
        else {
            rightSet.push_back(cluster);
        }
    }
    
    std::function<void(const std::vector<glm::vec3>&, const glm::vec3&, const glm::vec3&)> recursiveHull;
    
    recursiveHull = [&](const std::vector<glm::vec3>& subset, const glm::vec3& p1, const glm::vec3& p2) {
        
        if (subset.empty()) return;
        if (subset.size() < 3) return;
        
        uint32_t farthestIndex = FindFurthest(subset, p1, p2);
        glm::vec3 furthestPoint = subset[farthestIndex];
        hull.vertices.push_back(furthestPoint);
        
        std::vector<glm::vec3> leftSubset;
        std::vector<glm::vec3> rightSubset;
        
        for (const auto& point : subset) {
            if (PointLineDistance(p1, furthestPoint, point) > 0) {
                leftSubset.push_back(point);
            }
            if (PointLineDistance(furthestPoint, p2, point) > 0) {
                rightSubset.push_back(point);
            }
        }
        
        if (leftSubset.size() > 0) {
            recursiveHull(leftSubset, p1, furthestPoint);
        }

        if (rightSubset.size() > 0) {
            recursiveHull(rightSubset, furthestPoint, p2);
        }
    };
    
    recursiveHull(leftSet, p1, p2);
    recursiveHull(rightSet, p2, p1);
    
    for (int i = 1; i < hull.vertices.size(); i++) {
        hull.faces.push_back({0, i, static_cast<int>((i + 1) % hull.vertices.size())});
    }
    
    return hull;
}



// ------------------------------------------------------------------------------------------------------------- //
// ComputeConvexity //
// ------------------------------------------------------------------------------------------------------------- //

float RObject::ComputeConvexity(const std::vector<ACDTriangle>& triangles, const std::set<uint32_t>& cluster, uint32_t idx, ACDIndexedMesh mesh) {
    
    ACDTriangle currentTriangle = triangles[idx];

    // Compute the normal of the current triangle
    glm::vec3 v0 = GetVertexFromIndex(mesh, currentTriangle.indices[0]);
    glm::vec3 v1 = GetVertexFromIndex(mesh, currentTriangle.indices[1]);
    glm::vec3 v2 = GetVertexFromIndex(mesh, currentTriangle.indices[2]);

    glm::vec3 normalCurrent = glm::normalize(glm::cross(v1 - v0, v2 - v0));

    float totalAngleDifference = 0.0f;
    int validNeighbors = 0;

    // Compare with the normals of neighboring triangles
    for (uint32_t neighborIdx : currentTriangle.neighbors) {
        if (cluster.find(neighborIdx) != cluster.end()) {
            ACDTriangle neighbor = triangles[neighborIdx];

            glm::vec3 nv0 = GetVertexFromIndex(mesh, neighbor.indices[0]);
            glm::vec3 nv1 = GetVertexFromIndex(mesh, neighbor.indices[1]);
            glm::vec3 nv2 = GetVertexFromIndex(mesh, neighbor.indices[2]);

            glm::vec3 normalNeighbor = glm::normalize(glm::cross(nv1 - nv0, nv2 - nv0));

            // Compute the angle between normals
            float dotProduct = glm::dot(normalCurrent, normalNeighbor);
            dotProduct = glm::clamp(dotProduct, -1.0f, 1.0f);  // Prevent floating-point errors
            float angleBetweenNormals = glm::degrees(acos(dotProduct));

            totalAngleDifference += angleBetweenNormals;
            validNeighbors++;
        }
    }

    // Return the average angle instead of max
    if (validNeighbors > 0) {
        return totalAngleDifference / validNeighbors;
    } 
    else {
        return 0.0f;  // No valid neighbors, convexity is 0
    }
}



// ------------------------------------------------------------------------------------------------------------- //
// ACD //
// ------------------------------------------------------------------------------------------------------------- //

std::vector<std::vector<ACDTriangle>> RObject::ACD(std::vector<ACDTriangle> triangles, ACDIndexedMesh mesh, int maxClusters) {
    
    std::set<uint32_t> unprocessed;
    for (size_t i = 0; i < triangles.size(); i++) {
        unprocessed.insert(i);
    }

    std::vector<std::vector<ACDTriangle>> convexClusters;

    while (!unprocessed.empty() && convexClusters.size() < maxClusters) {
        std::queue<uint32_t> queue;
        std::set<uint32_t> cluster;

        uint32_t start = *unprocessed.begin();
        queue.push(start);
        cluster.insert(start);
        unprocessed.erase(start);

        while (!queue.empty()) {
            uint32_t curr = queue.front();
            queue.pop();

            for (uint32_t neighbor : triangles[curr].neighbors) {
                if (unprocessed.find(neighbor) != unprocessed.end()) {

                    float concavity = ComputeConvexity(triangles, cluster, neighbor, mesh);
                    
                    std::cout << concavity << "CONCAVITY\n";

                    if (concavity < 0.5f) {
                        queue.push(neighbor);
                        cluster.insert(neighbor);
                        unprocessed.erase(neighbor);
                    }
                }
            }
        }

        std::vector<ACDTriangle> part;
        for (uint32_t idx : cluster) {
            part.push_back(triangles[idx]);
        }
        convexClusters.push_back(part);
    }
    
    return convexClusters;
}



// ------------------------------------------------------------------------------------------------------------- //
// CreateConvexMesh //
// ------------------------------------------------------------------------------------------------------------- //

ACDIndexedMesh RObject::CreateConvexMesh(const std::vector<ACDTriangle>& cluster, ACDIndexedMesh mesh) {
    
    ACDIndexedMesh convexMesh;
    std::unordered_map<uint32_t, uint32_t> vertexMap;
    
    for (const auto& triangle : cluster) {
        for (uint32_t v : triangle.indices) {
            if (vertexMap.find(v) == vertexMap.end()) {
                vertexMap[v] = convexMesh.vertices.size() / 8;
                convexMesh.vertices.push_back(mesh.vertices[v * 8]);
                convexMesh.vertices.push_back(mesh.vertices[v * 8 + 1]);
                convexMesh.vertices.push_back(mesh.vertices[v * 8 + 2]);
                convexMesh.vertices.push_back(mesh.vertices[v * 8 + 3]);
                convexMesh.vertices.push_back(mesh.vertices[v * 8 + 4]);
                convexMesh.vertices.push_back(mesh.vertices[v * 8 + 5]);
                convexMesh.vertices.push_back(mesh.vertices[v * 8 + 6]);
                convexMesh.vertices.push_back(mesh.vertices[v * 8 + 7]);
            }
            convexMesh.indices.push_back(vertexMap[v]);
        }
    }
    return convexMesh;
}



std::set<uint32_t> RObject::ExpandCluster(const std::vector<ACDTriangle>& triangles, uint32_t idx, const ACDIndexedMesh& mesh, float convexityThreshold = 0.5f) {
    
    std::set<uint32_t> cluster;
    std::queue<uint32_t> expansion;
    expansion.push(idx);

    while (!expansion.empty()) {
        uint32_t cidx = expansion.front();
        expansion.pop();

        if (cluster.find(cidx) != cluster.end()) continue;

        cluster.insert(cidx);

        float convexity = ComputeConvexity(triangles, cluster, cidx, mesh);

        if (convexity < convexityThreshold) {
            for (uint32_t nidx : triangles[cidx].neighbors) {
                if (cluster.find(nidx) == cluster.end()) {
                    expansion.push(nidx);
                }
            }
        }
    }
    return cluster;
}

std::vector<std::set<uint32_t>> RObject::GenerateClusters(const std::vector<ACDTriangle>& triangles, const ACDIndexedMesh& mesh) {
    
    std::vector<std::set<uint32_t>> clusters;
    std::set<uint32_t> currentCluster;
    
    for (uint32_t i = 0; i < triangles.size(); ++i) {
        currentCluster.insert(i);
        
        if (currentCluster.size() > 5) {
            //std::set<uint32_t> newCluster = ExpandCluster(triangles, i, mesh, 10.1f);
            clusters.push_back(currentCluster);
            currentCluster.clear();
        }
    }
    
    if (!currentCluster.empty()) {
        clusters.push_back(currentCluster);
    }

    return clusters;
}

// ------------------------------------------------------------------------------------------------------------- //
// Decompose //
// ------------------------------------------------------------------------------------------------------------- //

void RObject::Decompose(int maxClusters = 10) {
    
    std::vector<std::future<std::vector<ACDIndexedMesh>>> futures;
    
    for (ACDIndexedMesh mesh : meshes) {
        std::vector<ACDTriangle> triangles = ConstructAdjacency(mesh);
        
        futures.push_back(std::async(std::launch::async, [triangles, mesh, maxClusters, this]() {
            
            std::vector<ACDIndexedMesh> localMeshes;
            //std::vector<std::vector<ACDTriangle>> convexClusters = ACD(triangles, mesh, maxClusters);
            std::vector<std::set<uint32_t>> clusters = GenerateClusters(triangles, mesh);
            
            float t = 0.0f;
            
            for (const auto& cluster : clusters) {
                std::vector<ACDTriangle> clusterTriangles;

                for (const auto& triangleIndex : cluster) {
                    clusterTriangles.push_back(triangles[triangleIndex]);
                }

                ACDIndexedMesh convexMesh = CreateConvexMesh(clusterTriangles, mesh);
                srand(time(0) + t);
                convexMesh.color = glm::vec3(0.8);
                localMeshes.push_back(convexMesh);
                t += 100.0f;
            }

            
            return localMeshes;
        }));
    }
    
    for (auto& future : futures) {
        std::vector<ACDIndexedMesh> decomposedMeshes = future.get();
        std::vector<ACDIndexedMesh> openglInitializedMeshes;
        for (ACDIndexedMesh mesh : decomposedMeshes) {
            openglInitializedMeshes.push_back(CreateOpenGLMesh(mesh));
        }
        processedMeshes.insert(processedMeshes.end(), openglInitializedMeshes.begin(), openglInitializedMeshes.end());
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

ACDIndexedMesh RObject::CreateOpenGLMesh(ACDIndexedMesh convexMesh) {
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
