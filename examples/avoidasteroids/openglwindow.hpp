#ifndef OPENGLWINDOW_HPP_
#define OPENGLWINDOW_HPP_

#include <random>

#include "abcg.hpp"
#include "model.hpp"

class OpenGLWindow : public abcg::OpenGLWindow {
 protected:
  void handleEvent(SDL_Event& handleEvent) override;
  void initializeGL() override;
  void paintGL() override;
  void paintUI() override;
  void resizeGL(int width, int height) override;
  void terminateGL() override;

 private:
  static const int m_numAsteroids{320};
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
  
  float m_angle{};

  glm::mat4 m_viewMatrix{1.0f};
  glm::mat4 m_projMatrix{1.0f};
  float m_FOV{140.0f};

  int cont_collisions{2};
  bool lost{false};
  abcg::ElapsedTimer m_collisionTimer;
  abcg::ElapsedTimer m_restartWaitTimer;

  void randomizeAsteroid(glm::vec3 &position, glm::vec3 &rotation);
  void randomizePlanet(glm::vec3 &position, glm::vec3 &rotation);

  void update();

  void restart();
  glm::vec3 m_shipPosition = glm::vec3(0.0f, -6.0f, -5.0f);
  glm::vec3 m_shipRotation = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));  glm::mat4 m_modelMatrix{1.0f};
  std::vector<const char*> m_shaderNames{"texture", "blinnphong", "phong",
                                         "gouraud", "normal", "depth"};
  std::vector<GLuint> m_programs;
  int m_currentProgramIndex{};
  int m_mappingMode{};
  // Light and material properties
  /*
  glm::vec4 m_lightDir{-1.0f, -1.0f, -1.0f, 0.0f};
  glm::vec4 m_Ia{1.0f};
  glm::vec4 m_Id{1.0f};
  glm::vec4 m_Is{1.0f};
  glm::vec4 m_Ka;
  glm::vec4 m_Kd;
  glm::vec4 m_Ks;
  float m_shininess{};
  */
  void loadModel(std::string path_obj, std::string path_text, Model &model);
};

#endif