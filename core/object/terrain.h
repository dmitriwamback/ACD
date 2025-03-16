//
//  terrain.h
//  VACD
//
//  Created by Dmitri Wamback on 2025-03-16.
//

#ifndef terrain_h
#define terrain_h

#define TERRAIN_SIZE 100

class Terrain: public RObject {
public:
    static RObject* Create();
    void Render(Shader shader) override;
private:
    static glm::vec3 CalculateNormalVector(glm::vec3 P1, glm::vec3 P2, glm::vec3 P3);
};

RObject* Terrain::Create() {
    RObject* terrain = new Terrain();
        
    std::vector<float> vertices = {};
    std::vector<uint32_t> indices = {};
    
    int index = 0;
    
    for (int x = 0; x < TERRAIN_SIZE; x++) {
        for (int z = 0; z < TERRAIN_SIZE; z++) {
            
            index = z + x * TERRAIN_SIZE;
                        
            float height = noiseLayer((float)x / TERRAIN_SIZE, (float)z / TERRAIN_SIZE, 2.2f, 0.5f, 32) * 1.5f;
            float heightR = noiseLayer((float)(x+1) / TERRAIN_SIZE, (float)z / TERRAIN_SIZE, 2.2f, 0.5f, 32) * 1.5f;
            float heightD = noiseLayer((float)x / TERRAIN_SIZE, (float)(z+1) / TERRAIN_SIZE, 2.2f, 0.5f, 32) * 1.5f;

            glm::vec3 p1 = glm::vec3(x - TERRAIN_SIZE / 2, height - 10.0f, z - TERRAIN_SIZE / 2);
            glm::vec3 p2 = glm::vec3((x + 1) - TERRAIN_SIZE / 2, heightR - 10.0f, z - TERRAIN_SIZE / 2);
            glm::vec3 p3 = glm::vec3(x - TERRAIN_SIZE / 2, heightD - 10.0f, (z + 1) - TERRAIN_SIZE / 2);

            glm::vec3 normal = -Terrain::CalculateNormalVector(p1, p2, p3);

            vertices.push_back(p1.x);
            vertices.push_back(p1.y);
            vertices.push_back(p1.z);

            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);
            
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
            
            if (x != TERRAIN_SIZE - 1 && z != TERRAIN_SIZE - 1) {
                indices.push_back(index);
                indices.push_back(index+1);
                indices.push_back(index+TERRAIN_SIZE);
                indices.push_back(index+1);
                indices.push_back(index+TERRAIN_SIZE+1);
                indices.push_back(index+TERRAIN_SIZE);
            }
        }
    }
    
    ACDIndexedMesh m_mesh;
    m_mesh.vertices = vertices;
    m_mesh.indices = indices;
    
    terrain->position = glm::vec3(0.0f, 0.0f, 0.0f);
    terrain->rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    terrain->scale = glm::vec3(0.4f, 2.0f, 0.4f);
    
    terrain->color = glm::vec3(1.0f);
    
    glGenVertexArrays(1, &m_mesh.vao);
    glBindVertexArray(m_mesh.vao);
    
    glGenBuffers(1, &m_mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_mesh.vbo);
    //throw std::runtime_error(std::to_string(vertices.size()));
    glBufferData(GL_ARRAY_BUFFER, TERRAIN_SIZE * TERRAIN_SIZE * 6 * sizeof(float), &m_mesh.vertices[0], GL_STATIC_DRAW);
    
    glGenBuffers(1, &m_mesh.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_mesh.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (TERRAIN_SIZE - 1) * (TERRAIN_SIZE - 1) * 6 * sizeof(uint32_t), &m_mesh.indices[0], GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    
    terrain->meshes.push_back(m_mesh);
    static_cast<Terrain*>(terrain)->Decompose(10);
    return terrain;
}

glm::vec3 Terrain::CalculateNormalVector(glm::vec3 P1, glm::vec3 P2, glm::vec3 P3) {
    glm::vec3 A = P2 - P1;
    glm::vec3 B = P3 - P1;
    
    return glm::normalize(glm::cross(A, B));
}

void Terrain::Render(Shader shader) {
    
    shader.Use();
    glm::mat4 model = CreateModelMatrix();
    shader.SetMatrix4("model", model);
    
    Ray ray{};
    ray.origin = camera.position;
    ray.direction = camera.mouseRayDirection;
    
    glm::mat4 modelMatrix = CreateModelMatrix();
    
    for (ACDIndexedMesh mesh : processedMeshes) {
        
        std::optional<Intersection> intersect = Raycast(ray, GetTransformedVertices(mesh, modelMatrix), mesh.indices);
        
        glBindVertexArray(mesh.vao);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(float), &mesh.vertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(uint32_t), &mesh.indices[0], GL_STATIC_DRAW);
        
        shader.SetVector3("color", mesh.color);
        if (intersect) {
            shader.SetVector3("color", glm::vec3(1.0f, 0.0f, 0.0f));
        }
        
        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
        
        shader.SetVector3("color", glm::vec3(0.0f, 0.0f, 0.0f));
        glDrawElements(GL_LINES, mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
        //glDrawElements(GL_POINTS, mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
    }
}

#endif /* terrain_h */
