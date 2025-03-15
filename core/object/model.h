//
//  model.h
//  VACD
//
//  Created by Dmitri Wamback on 2025-03-13.
//

#ifndef model_h
#define model_h

class Model: public RObject {
public:
    static RObject* Create(std::string assetPath);
    void Render(Shader shader) override;
private:
    void ProcessNode(aiNode *node, const aiScene *scene);
    void ProcessMesh(aiMesh *mesh, const aiScene *scene);
};

RObject* Model::Create(std::string assetPath) {
    RObject* model = new Model();
    
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile("/Users/dmitriwamback/Documents/models/blendermonkey.obj",
                                             aiProcess_Triangulate |
                                             aiProcess_FlipUVs |
                                             aiProcess_JoinIdenticalVertices |
                                             aiProcess_GenSmoothNormals | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph);
    
    aiNode* rootNode = scene->mRootNode;
    static_cast<Model*>(model)->ProcessNode(rootNode, scene);
    static_cast<Model*>(model)->Decompose(1000);
    model->scale    = glm::vec3(5.0f, 5.0f, 5.0f);
    model->rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    model->position = glm::vec3(0.0f);
    
    return model;
}

void Model::ProcessNode(aiNode *node, const aiScene *scene) {
    for (int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessMesh(mesh, scene);
    }
    for (int i = 0; i < node->mNumChildren; i++) {
        ProcessNode(node->mChildren[i], scene);
    }
}

void Model::ProcessMesh(aiMesh *mesh, const aiScene *scene) {
    
    std::vector<float>      m_vertices;
    std::vector<uint32_t>   m_indices;
    
    ACDIndexedMesh m_mesh{};
    
    for (int i = 0; i < mesh->mNumVertices; i++) {
        glm::vec3 vertexVector;
        vertexVector.x = mesh->mVertices[i].x;
        vertexVector.y = mesh->mVertices[i].y;
        vertexVector.z = mesh->mVertices[i].z;
        
        m_vertices.push_back(vertexVector.x); m_vertices.push_back(vertexVector.y); m_vertices.push_back(vertexVector.z);
        
        glm::vec3 normalVector;
        normalVector.x = mesh->mNormals[i].x;
        normalVector.y = mesh->mNormals[i].y;
        normalVector.z = mesh->mNormals[i].z;
        
        m_vertices.push_back(normalVector.x); m_vertices.push_back(normalVector.y); m_vertices.push_back(normalVector.z);
        
        if (mesh->mTextureCoords[0]) {
            glm::vec2 uv;
            uv.x = mesh->mTextureCoords[0][i].x;
            uv.y = mesh->mTextureCoords[0][i].y;
            m_vertices.push_back(uv.x); m_vertices.push_back(uv.y);
        }
        else {
            m_vertices.push_back(0); m_vertices.push_back(0);
        }
    }
    
    for (int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (int j = 0; j < face.mNumIndices; j++) {
            m_indices.push_back(face.mIndices[j]);
        }
    }
    
    m_mesh.vertices = m_vertices;
    m_mesh.indices = m_indices;
    
    meshes.push_back(m_mesh);
}

void Model::Render(Shader shader) {
    
    shader.Use();
    glm::mat4 model = CreateModelMatrix();
    shader.SetMatrix4("model", model);
    
    for (ACDIndexedMesh mesh : processedMeshes) {
        
        glBindVertexArray(mesh.vao);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(float), &mesh.vertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(uint32_t), &mesh.indices[0], GL_STATIC_DRAW);
        
        shader.SetVector3("color", mesh.color);
        
        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
    }
}

#endif /* model_h */
