#include "model.hpp"

#include <fmt/core.h>
#include <tiny_obj_loader.h>

#include <cppitertools/itertools.hpp>
#include <filesystem>
#include <glm/gtx/hash.hpp>
#include <unordered_map>

namespace std {
template <>
struct hash<Vertex> {
  size_t operator()(Vertex const& vertex) const noexcept {
    const std::size_t h1{std::hash<glm::vec3>()(vertex.position)};
    const std::size_t h2{std::hash<glm::vec3>()(vertex.normal)};
    const std::size_t h3{std::hash<glm::vec2>()(vertex.texCoord)};
    return h1 ^ h2 ^ h3;
  }
};
}

void Model::computeNormals() {
  for (auto& vertex : m_vertices) {
    vertex.normal = glm::zero<glm::vec3>();
  }

  for (const auto offset : iter::range<int>(0, m_indices.size(), 3)) {
    Vertex& a{m_vertices.at(m_indices.at(offset + 0))};
    Vertex& b{m_vertices.at(m_indices.at(offset + 1))};
    Vertex& c{m_vertices.at(m_indices.at(offset + 2))};

    const auto edge1{b.position - a.position};
    const auto edge2{c.position - b.position};
    const glm::vec3 normal{glm::cross(edge1, edge2)};

    a.normal += normal;
    b.normal += normal;
    c.normal += normal;
  }

  for (auto& vertex : m_vertices) {
    vertex.normal = glm::normalize(vertex.normal);
  }

  m_hasNormals = true;
}

void Model::createBuffers() {
  abcg::glDeleteBuffers(1, &m_EBO);
  abcg::glDeleteBuffers(1, &m_VBO);
  abcg::glGenBuffers(1, &m_VBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  abcg::glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices[0]) * m_vertices.size(),
                     m_vertices.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);
  abcg::glGenBuffers(1, &m_EBO);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     sizeof(m_indices[0]) * m_indices.size(), m_indices.data(),
                     GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Model::loadDiffuseTexture(std::string_view path) {
  if (!std::filesystem::exists(path)) return;
  abcg::glDeleteTextures(1, &m_diffuseTexture);
  m_diffuseTexture = abcg::opengl::loadTexture(path);
}

void Model::loadObj(std::string_view path, bool standardize) {
  const auto basePath{std::filesystem::path{path}.parent_path().string() + "/"};

  tinyobj::ObjReaderConfig readerConfig;
  readerConfig.mtl_search_path = basePath;
  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(path.data(), readerConfig)) {
    if (!reader.Error().empty()) {
      throw abcg::Exception{abcg::Exception::Runtime(
          fmt::format("Failed to load model {} ({})", path, reader.Error()))};
    }
    throw abcg::Exception{
        abcg::Exception::Runtime(fmt::format("Failed to load model {}", path))};
  }

  if (!reader.Warning().empty()) {
    fmt::print("Warning: {}\n", reader.Warning());
  }

  const auto& attrib{reader.GetAttrib()};
  const auto& shapes{reader.GetShapes()};
  const auto& materials{reader.GetMaterials()};

  m_vertices.clear();
  m_indices.clear();

  m_hasNormals = false;
  m_hasTexCoords = false;

  std::unordered_map<Vertex, GLuint> hash{};
  for (const auto& shape : shapes) {
    for (const auto offset : iter::range(shape.mesh.indices.size())) {
      const tinyobj::index_t index{shape.mesh.indices.at(offset)};
      const int startIndex{3 * index.vertex_index};
      const float vx{attrib.vertices.at(startIndex + 0)};
      const float vy{attrib.vertices.at(startIndex + 1)};
      const float vz{attrib.vertices.at(startIndex + 2)};

      float nx{};
      float ny{};
      float nz{};
      if (index.normal_index >= 0) {
        m_hasNormals = true;
        const int normalStartIndex{3 * index.normal_index};
        nx = attrib.normals.at(normalStartIndex + 0);
        ny = attrib.normals.at(normalStartIndex + 1);
        nz = attrib.normals.at(normalStartIndex + 2);
      }

      float tu{};
      float tv{};
      if (index.texcoord_index >= 0) {
        m_hasTexCoords = true;
        const int texCoordsStartIndex{2 * index.texcoord_index};
        tu = attrib.texcoords.at(texCoordsStartIndex + 0);
        tv = attrib.texcoords.at(texCoordsStartIndex + 1);
      }

      Vertex vertex{};
      vertex.position = {vx, vy, vz};
      vertex.normal = {nx, ny, nz};
      vertex.texCoord = {tu, tv};

      if (hash.count(vertex) == 0) {
        hash[vertex] = m_vertices.size();
        m_vertices.push_back(vertex);
      }
      m_indices.push_back(hash[vertex]);
    }
  }

  if (!materials.empty()) {
    const auto& mat{materials.at(0)};
    m_Ka = glm::vec4(mat.ambient[0], mat.ambient[1], mat.ambient[2], 1);
    m_Kd = glm::vec4(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2], 1);
    m_Ks = glm::vec4(mat.specular[0], mat.specular[1], mat.specular[2], 1);
    m_shininess = mat.shininess;

    if (!mat.diffuse_texname.empty())
      loadDiffuseTexture(basePath + mat.diffuse_texname);
  } else {
    m_Ka = {0.1f, 0.1f, 0.1f, 1.0f};
    m_Kd = {0.7f, 0.7f, 0.7f, 1.0f};
    m_Ks = {1.0f, 1.0f, 1.0f, 1.0f};
    m_shininess = 100.0f;
  }
  if (standardize) {
    this->standardize();
  }
  if (!m_hasNormals) {
    computeNormals();
  }
  createBuffers();
}

void Model::render(int numTriangles) const {
  abcg::glBindVertexArray(m_VAO);
  abcg::glActiveTexture(GL_TEXTURE0);
  abcg::glBindTexture(GL_TEXTURE_2D, m_diffuseTexture);
  abcg::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  abcg::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  abcg::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  abcg::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  const auto numIndices{(numTriangles < 0) ? m_indices.size(): numTriangles * 3};
  abcg::glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(numIndices), GL_UNSIGNED_INT, nullptr);
  abcg::glBindVertexArray(0);
}

void Model::setupVAO(GLuint program) {
  abcg::glDeleteVertexArrays(1, &m_VAO);
  abcg::glGenVertexArrays(1, &m_VAO);
  abcg::glBindVertexArray(m_VAO);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  const GLint positionAttribute{
      abcg::glGetAttribLocation(program, "inPosition")};
  if (positionAttribute >= 0) {
    abcg::glEnableVertexAttribArray(positionAttribute);
    abcg::glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE,
                                sizeof(Vertex), nullptr);
  }
  const GLint normalAttribute{abcg::glGetAttribLocation(program, "inNormal")};
  if (normalAttribute >= 0) {
    abcg::glEnableVertexAttribArray(normalAttribute);
    GLsizei offset{sizeof(glm::vec3)};
    abcg::glVertexAttribPointer(normalAttribute, 3, GL_FLOAT, GL_FALSE,
                                sizeof(Vertex),
                                reinterpret_cast<void*>(offset));
  }
  const GLint texCoordAttribute{
      abcg::glGetAttribLocation(program, "inTexCoord")};
  if (texCoordAttribute >= 0) {
    abcg::glEnableVertexAttribArray(texCoordAttribute);
    GLsizei offset{sizeof(glm::vec3) + sizeof(glm::vec3)};
    abcg::glVertexAttribPointer(texCoordAttribute, 2, GL_FLOAT, GL_FALSE,
                                sizeof(Vertex),
                                reinterpret_cast<void*>(offset));
  }
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);
  abcg::glBindVertexArray(0);
}

void Model::standardize() {
  glm::vec3 max(std::numeric_limits<float>::lowest());
  glm::vec3 min(std::numeric_limits<float>::max());
  for (const auto& vertex : m_vertices) {
    max.x = std::max(max.x, vertex.position.x);
    max.y = std::max(max.y, vertex.position.y);
    max.z = std::max(max.z, vertex.position.z);
    min.x = std::min(min.x, vertex.position.x);
    min.y = std::min(min.y, vertex.position.y);
    min.z = std::min(min.z, vertex.position.z);
  }
  const auto center{(min + max) / 2.0f};
  const auto scaling{2.0f / glm::length(max - min)};
  for (auto& vertex : m_vertices) {
    vertex.position = (vertex.position - center) * scaling;
  }
}

void Model::terminateGL() {
  abcg::glDeleteTextures(1, &m_diffuseTexture);
  abcg::glDeleteBuffers(1, &m_EBO);
  abcg::glDeleteBuffers(1, &m_VBO);
  abcg::glDeleteVertexArrays(1, &m_VAO);
}