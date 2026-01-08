#ifndef MODEL3D_HPP
#define MODEL3D_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "common/tiny_obj_loader.h"
#include "common/stb_image.h"

struct Mesh {
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texCoords;
    std::vector<float> colors;
    
    GLuint VAO = 0;
    GLuint VBO_vertices = 0;
    GLuint VBO_normals = 0;
    GLuint VBO_texcoords = 0;
    GLuint VBO_colors = 0;
    
    GLuint diffuseTexID = 0;
    int materialIndex = -1;
    
    void cleanup() {
        if (VBO_vertices) { glDeleteBuffers(1, &VBO_vertices); VBO_vertices = 0; }
        if (VBO_normals) { glDeleteBuffers(1, &VBO_normals); VBO_normals = 0; }
        if (VBO_texcoords) { glDeleteBuffers(1, &VBO_texcoords); VBO_texcoords = 0; }
        if (VBO_colors) { glDeleteBuffers(1, &VBO_colors); VBO_colors = 0; }
        if (VAO) { glDeleteVertexArrays(1, &VAO); VAO = 0; }
        if (diffuseTexID) { glDeleteTextures(1, &diffuseTexID); diffuseTexID = 0; }
    }
};

struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
    std::string diffuseTexture;
    
    Material() : ambient(0.2f), diffuse(0.8f), specular(1.0f), shininess(32.0f) {}
};

class Model3D {
private:
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    std::string modelPath;
    std::string basePath;
    
    glm::vec3 minBounds, maxBounds;
    glm::vec3 center;
    float maxDimension;
    bool isLoaded;
    
    GLuint loadTexture(const std::string& path) {
        std::ifstream f(path.c_str());
        if (!f.good()) return 0;
        f.close();

        GLuint textureID;
        glGenTextures(1, &textureID);
        
        int width, height, channels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

        if (data) {
            GLenum format = GL_RGB;
            if (channels == 1) format = GL_RED;
            else if (channels == 3) format = GL_RGB;
            else if (channels == 4) format = GL_RGBA;
            
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            
            stbi_image_free(data);
            return textureID;
        }
        return 0;
    }
    
    void calculateBounds() {
        minBounds = glm::vec3(1e10f);
        maxBounds = glm::vec3(-1e10f);
        for (const auto& mesh : meshes) {
            for (size_t i = 0; i < mesh.vertices.size(); i += 3) {
                minBounds.x = std::min(minBounds.x, mesh.vertices[i + 0]);
                minBounds.y = std::min(minBounds.y, mesh.vertices[i + 1]);
                minBounds.z = std::min(minBounds.z, mesh.vertices[i + 2]);
                maxBounds.x = std::max(maxBounds.x, mesh.vertices[i + 0]);
                maxBounds.y = std::max(maxBounds.y, mesh.vertices[i + 1]);
                maxBounds.z = std::max(maxBounds.z, mesh.vertices[i + 2]);
            }
        }
        center = (minBounds + maxBounds) * 0.5f;
        glm::vec3 size = maxBounds - minBounds;
        maxDimension = std::max(size.x, std::max(size.y, size.z));
    }
    
    void centerModel() {
        for (auto& mesh : meshes) {
            for (size_t i = 0; i < mesh.vertices.size(); i += 3) {
                mesh.vertices[i + 0] -= center.x;
                mesh.vertices[i + 1] -= center.y;
                mesh.vertices[i + 2] -= center.z;
            }
        }
        maxBounds -= center; minBounds -= center; center = glm::vec3(0.0f);
    }
    
public:
    Model3D(const std::string& objPath, const std::string& baseDir = "") 
        : modelPath(objPath), basePath(baseDir), center(0.0f), maxDimension(1.0f), isLoaded(false) {
        if (basePath.empty()) {
            size_t pos = objPath.find_last_of("/\\");
            if (pos != std::string::npos) basePath = objPath.substr(0, pos + 1);
        } else if (basePath.back() != '/' && basePath.back() != '\\') basePath += "/";
    }
    
    ~Model3D() { cleanup(); }
    
    bool load() {
        std::cout << "Loading Model: " << modelPath << std::endl;
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> objMaterials;
        std::string warn, err;
        
        bool success = tinyobj::LoadObj(&attrib, &shapes, &objMaterials, &warn, &err, modelPath.c_str(), basePath.c_str());
        if (!success) { return false; }
        
        materials.resize(std::max((size_t)1, objMaterials.size()));
        for (size_t i = 0; i < objMaterials.size(); i++) {
            materials[i].ambient = glm::vec3(objMaterials[i].ambient[0], objMaterials[i].ambient[1], objMaterials[i].ambient[2]);
            materials[i].diffuse = glm::vec3(objMaterials[i].diffuse[0], objMaterials[i].diffuse[1], objMaterials[i].diffuse[2]);
            materials[i].specular = glm::vec3(objMaterials[i].specular[0], objMaterials[i].specular[1], objMaterials[i].specular[2]);
            materials[i].shininess = objMaterials[i].shininess;
            materials[i].diffuseTexture = objMaterials[i].diffuse_texname;
        }
        
        if (objMaterials.empty()) meshes.resize(1);
        else meshes.resize(objMaterials.size());
        
        for (size_t s = 0; s < shapes.size(); s++) {
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                size_t fv = shapes[s].mesh.num_face_vertices[f];
                int material_id = shapes[s].mesh.material_ids[f];
                int meshIndex = (material_id < 0) ? 0 : material_id;
                if (meshIndex >= (int)meshes.size()) meshIndex = 0;
                
                for (size_t v = 0; v < fv; v++) {
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                    if (idx.vertex_index >= 0) {
                        meshes[meshIndex].vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
                        meshes[meshIndex].vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
                        meshes[meshIndex].vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);
                    }
                    if (idx.normal_index >= 0) {
                        meshes[meshIndex].normals.push_back(attrib.normals[3 * idx.normal_index + 0]);
                        meshes[meshIndex].normals.push_back(attrib.normals[3 * idx.normal_index + 1]);
                        meshes[meshIndex].normals.push_back(attrib.normals[3 * idx.normal_index + 2]);
                    } else {
                        meshes[meshIndex].normals.push_back(0.0f); meshes[meshIndex].normals.push_back(1.0f); meshes[meshIndex].normals.push_back(0.0f);
                    }
                    if (idx.texcoord_index >= 0) {
                        meshes[meshIndex].texCoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
                        meshes[meshIndex].texCoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
                    } else {
                        meshes[meshIndex].texCoords.push_back(0.0f); meshes[meshIndex].texCoords.push_back(0.0f);
                    }
                    meshes[meshIndex].colors.push_back(1.0f); meshes[meshIndex].colors.push_back(1.0f); meshes[meshIndex].colors.push_back(1.0f);
                }
                index_offset += fv;
            }
        }
        
        int validMeshes = 0;
        for (const auto& mesh : meshes) if (!mesh.vertices.empty()) validMeshes++;
        if (validMeshes == 0) return false;
        
        calculateBounds();
        centerModel();
        isLoaded = true;
        return true;
    }
    
    void setupMeshes() {
        if (!isLoaded) return;
        for (size_t i = 0; i < meshes.size(); i++) {
            auto& mesh = meshes[i];
            if (mesh.vertices.empty()) continue;
            
            glGenVertexArrays(1, &mesh.VAO); glBindVertexArray(mesh.VAO);
            glGenBuffers(1, &mesh.VBO_vertices); glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO_vertices);
            glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(float), mesh.vertices.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); glEnableVertexAttribArray(0);
            
            if (!mesh.normals.empty()) {
                glGenBuffers(1, &mesh.VBO_normals); glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO_normals);
                glBufferData(GL_ARRAY_BUFFER, mesh.normals.size() * sizeof(float), mesh.normals.data(), GL_STATIC_DRAW);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); glEnableVertexAttribArray(1);
            }
            if (!mesh.texCoords.empty()) {
                glGenBuffers(1, &mesh.VBO_texcoords); glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO_texcoords);
                glBufferData(GL_ARRAY_BUFFER, mesh.texCoords.size() * sizeof(float), mesh.texCoords.data(), GL_STATIC_DRAW);
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0); glEnableVertexAttribArray(2);
            }
            if (!mesh.colors.empty()) {
                glGenBuffers(1, &mesh.VBO_colors); glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO_colors);
                glBufferData(GL_ARRAY_BUFFER, mesh.colors.size() * sizeof(float), mesh.colors.data(), GL_STATIC_DRAW);
                glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); glEnableVertexAttribArray(3);
            }
            glBindVertexArray(0);
        }
        for (size_t i = 0; i < meshes.size(); i++) {
            if (i < materials.size()) {
                meshes[i].materialIndex = i;
                if (!materials[i].diffuseTexture.empty()) {
                    std::string texPath = basePath + materials[i].diffuseTexture;
                    meshes[i].diffuseTexID = loadTexture(texPath);
                }
            }
        }
    }
    
    void render(GLuint shaderProgram) {
        if (!isLoaded) return;
        
        for (const auto& mesh : meshes) {
            if (mesh.vertices.empty() || mesh.VAO == 0) continue;
            
            bool hasTexture = (mesh.diffuseTexID != 0);
            if (hasTexture) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, mesh.diffuseTexID);
                glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
                glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 1);
            } else {
                glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0);
                
                glm::vec3 finalColor(0.0f);
                glm::vec3 ambientColor(0.0f);
                glm::vec3 specular(0.5f);
                float shininess = 32.0f;

                int matIdx = mesh.materialIndex;
                if (matIdx >= 0 && matIdx < (int)materials.size()) {
                    const Material& mat = materials[matIdx];
                    finalColor = mat.diffuse;
                    ambientColor = mat.ambient;
                    specular = mat.specular;
                    shininess = mat.shininess;
                }

                if (glm::length(finalColor) < 0.001f) {
                    float cinza = 0.15f; 
                    finalColor = glm::vec3(cinza);
                    ambientColor = glm::vec3(cinza * 0.5f);
                }

                glUniform3fv(glGetUniformLocation(shaderProgram, "material.diffuse"), 1, &finalColor[0]);
                glUniform3fv(glGetUniformLocation(shaderProgram, "material.ambient"), 1, &ambientColor[0]);
                glUniform3fv(glGetUniformLocation(shaderProgram, "material.specular"), 1, &specular[0]);
                glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), shininess);
            }
            
            glBindVertexArray(mesh.VAO);
            glDrawArrays(GL_TRIANGLES, 0, mesh.vertices.size() / 3);
            glBindVertexArray(0);
        }
    }
    
    void cleanup() {
        for (auto& mesh : meshes) mesh.cleanup();
        meshes.clear(); materials.clear(); isLoaded = false;
    }
    
    bool loaded() const { return isLoaded; }
    glm::vec3 getMinBounds() const { return minBounds; }
    glm::vec3 getMaxBounds() const { return maxBounds; }
    float getMaxDimension() const { return maxDimension; }
};

#endif