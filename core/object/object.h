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
    std::vector<ACDIndexedMesh> meshes;
    std::vector<ACDIndexedMesh> processedMeshes;
    
    int vao, vbo, ibo;
    glm::vec3 position, scale, rotation, color;
    
    static RObject* Create();
    virtual void Render(Shader shader) {}
    void Decompose(int maxClusters);

    std::vector<ACDTriangle> ConstructAdjacency(ACDIndexedMesh mesh);
    std::vector<std::vector<ACDTriangle>> ACD(std::vector<ACDTriangle>& triangles, ACDIndexedMesh mesh, int maxClusters);
    ACDIndexedMesh CreateConvexMesh(const std::vector<ACDTriangle>& cluster, ACDIndexedMesh mesh);
    glm::mat4 CreateModelMatrix();
    float ComputeConcavity(const std::vector<ACDTriangle>& triangles, const std::set<uint32_t>& cluster, uint32_t newTriangleIndex, ACDIndexedMesh mesh);
    ACDConvexHull ComputeConvexHull(const std::vector<glm::vec3>& cluster);
    float DistanceFromHull(const ACDConvexHull& hull, const glm::vec3& point);
    glm::vec3 GetVertexFromIndex(ACDIndexedMesh& object, uint32_t index);
};



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

glm::vec3 RObject::GetVertexFromIndex(ACDIndexedMesh& object, uint32_t index) {
    uint32_t stride = 8;
    uint32_t vertex = index * stride;
    
    return glm::vec3(object.vertices[vertex], object.vertices[vertex+1], object.vertices[vertex+2]);
}

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

ACDConvexHull RObject::ComputeConvexHull(const std::vector<glm::vec3>& cluster) {
    
    if (cluster.size() < 4) {
        std::cout << "Invalid\n";
        return {cluster, {}};
    }
        
    ACDConvexHull hull;
    
    int minx = 0, maxx = 0, miny = 0, maxy = 0, minz = 0, maxz = 0;
    for (int i = 1; i < cluster.size(); i++) {
        if (cluster[i].x < cluster[minx].x) minx = i;
        if (cluster[i].x > cluster[maxx].x) maxx = i;
        if (cluster[i].y < cluster[miny].y) miny = i;
        if (cluster[i].y > cluster[maxy].y) maxy = i;
        if (cluster[i].z < cluster[minz].z) minz = i;
        if (cluster[i].z > cluster[maxz].z) maxz = i;
    }
    
    std::set<int> foundation = {minx, maxx, miny, maxy, minz, maxz};
    for (int index : foundation) {
        hull.vertices.push_back(cluster[index]);
    }
    
    if (hull.vertices.size() >= 4) {
        hull.faces.push_back({0, 1, 2});
        hull.faces.push_back({0, 1, 3});
        hull.faces.push_back({0, 2, 3});
        hull.faces.push_back({1, 2, 3});
    }
    
    std::cout << hull.faces.size() << "FACES" << '\n';
    
    return hull;
}

float RObject::ComputeConcavity(const std::vector<ACDTriangle>& triangles, const std::set<uint32_t>& cluster, uint32_t newTriangleIndex, ACDIndexedMesh mesh) {
    
    std::vector<glm::vec3> clusterPoints;
    
    for (uint32_t idx : cluster) {
        for (int i = 0; i < 3; i++) {
            clusterPoints.push_back(GetVertexFromIndex(mesh, triangles[idx].indices[i]));
        }
    }
    
    if (clusterPoints.size() == 3) {
        glm::vec3 fourthPoint = clusterPoints[0] + glm::vec3(0.01f, 0.01f, 0.01f);
        clusterPoints.push_back(fourthPoint);
    }
        
    ACDConvexHull hull = ComputeConvexHull(clusterPoints);
    
    const ACDTriangle newTriangle = triangles[newTriangleIndex];
    
    float maxDev = 0.0f;
    
    for (int i = 0; i < 3; i++) {
        glm::vec3 vertex = GetVertexFromIndex(mesh, newTriangle.indices[i]);
        float deviation = DistanceFromHull(hull, vertex);
        maxDev = std::max(maxDev, deviation);
    }
        
    return maxDev;
}

std::vector<std::vector<ACDTriangle>> RObject::ACD(std::vector<ACDTriangle>& triangles, ACDIndexedMesh mesh, int maxClusters) {
    
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

                    float concavity = ComputeConcavity(triangles, cluster, neighbor, mesh);
                    
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

void RObject::Decompose(int maxClusters = 10) {
    
    for (ACDIndexedMesh mesh : meshes) {
        std::vector<ACDTriangle> triangles = ConstructAdjacency(mesh);
        std::vector<std::vector<ACDTriangle>> convexClusters = ACD(triangles, mesh, maxClusters);
        
        float t = 0.0f;
        
        for (auto& polytope : convexClusters) {
            ACDIndexedMesh convexMesh = CreateConvexMesh(polytope, mesh);
            srand(time(0) + t);
            convexMesh.color = glm::vec3(rand()%255 / 255.0, rand()%255 / 255.0, rand()%255 / 255.0);
            processedMeshes.push_back(convexMesh);
            t += 100.0f;
        }
    }
}

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

#endif /* object_h */
