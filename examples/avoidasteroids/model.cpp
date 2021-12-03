#include "model.hpp"
#include <fmt/core.h>
#include <tiny_obj_loader.h>
#include <cppitertools/itertools.hpp>
#include <glm/gtx/hash.hpp>
#include <unordered_map>

namespace std {
template <>
struct hash<Vertex> {
  size_t operator()(Vertex const& vertex) const noexcept {
    const std::size_t h1{std::hash<glm::vec3>()(vertex.position)};
    return h1;
  }
};
}

void Model::createBuffers() {
  abcg::glDeleteBuffers(1, &m_EBO);
  abcg::glDeleteBuffers(1, &m_VBO);
  abcg::glGenBuffers(1, &m_VBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  abcg::glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices[0]) * m_vertices.size(), m_vertices.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);
  abcg::glGenBuffers(1, &m_EBO);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     sizeof(m_indices[0]) * m_indices.size(), m_indices.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Model::loadObj(std::string_view path, bool standardize) {
  tinyobj::ObjReader reader;
  if (!reader.ParseFromFile(path.data())) {
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
  m_vertices.clear();
  m_indices.clear();
  std::unordered_map<Vertex, GLuint> hash{};
  for (const auto& shape : shapes) {
    for (const auto offset : iter::range(shape.mesh.indices.size())) {
      const tinyobj::index_t index{shape.mesh.indices.at(offset)};
      const std::size_t startIndex{static_cast<size_t>(3 * index.vertex_index)};
      const float vx{attrib.vertices.at(startIndex + 0)};
      const float vy{attrib.vertices.at(startIndex + 1)};
      const float vz{attrib.vertices.at(startIndex + 2)};
      Vertex vertex{};
      vertex.position = {vx, vy, vz};
      if (hash.count(vertex) == 0) {
        hash[vertex] = m_vertices.size();
        m_vertices.push_back(vertex);
      }
      m_indices.push_back(hash[vertex]);
    }
  }
  if (standardize) {
    this->standardize();
  }
  createBuffers();
}

void Model::render(int numTriangles) const {
  abcg::glBindVertexArray(m_VAO);
  const auto numIndices{(numTriangles < 0) ? m_indices.size(): numTriangles * 3};
  abcg::glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(numIndices),GL_UNSIGNED_INT, nullptr);
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
    abcg::glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
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
  abcg::glDeleteBuffers(1, &m_EBO);
  abcg::glDeleteBuffers(1, &m_VBO);
  abcg::glDeleteVertexArrays(1, &m_VAO);
}