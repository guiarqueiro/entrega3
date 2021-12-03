#ifndef OPENGLWINDOW_HPP_
#define OPENGLWINDOW_HPP_

#include <random>

#include "abcg.hpp"
#include "model.hpp"

class OpenGLWindow : public abcg::OpenGLWindow {
 protected:
  void initializeGL() override;
  void paintGL() override;
  void paintUI() override;
  void resizeGL(int width, int height) override;
  void terminateGL() override;

 private:
  static const int m_numAsteroids{500};
  static const int m_numPlanets{12};

  GLuint m_program{};

  int m_viewportWidth{};
  int m_viewportHeight{};

  std::default_random_engine m_randomEngine;

  Model m_asteroid;
  Model m_ship;
  Model m_planetRing;
  Model m_planetRound;

  std::array<glm::vec3, m_numAsteroids> m_asteroidPositions;
  std::array<glm::vec3, m_numAsteroids> m_asteroidRotations;
  std::array<glm::vec3, m_numPlanets> m_planetPositions;
  std::array<glm::vec3, m_numPlanets> m_planetRotations;

  glm::vec3 m_shipPosition = glm::vec3(0.0f, -6.0f, -5.0f);
  glm::vec3 m_shipRotation = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));

  float m_angle{};

  glm::mat4 m_viewMatrix{1.0f};
  glm::mat4 m_projMatrix{1.0f};
  float m_FOV{140.0f};

  void randomizeAsteroid(glm::vec3 &position, glm::vec3 &rotation);
  void randomizePlanet(glm::vec3 &position, glm::vec3 &rotation);
  
  void update();
};

#endif