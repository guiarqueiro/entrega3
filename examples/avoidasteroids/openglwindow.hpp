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
  static const int m_numAsteroids{150};
  static const int m_numPlanets{12};

  std::vector<GLuint> m_programs;

  int m_viewportWidth{};
  int m_viewportHeight{};

  std::default_random_engine m_randomEngine;

  Model m_asteroid;
  Model m_ship;
  Model m_planetRing;
  Model m_planetRound;
  Model m_skybox;

  std::array<glm::vec3, m_numAsteroids> m_asteroidPositions;
  std::array<glm::vec3, m_numAsteroids> m_asteroidRotations;
  std::array<glm::vec3, m_numPlanets> m_planetPositions;
  std::array<glm::vec3, m_numPlanets> m_planetRotations;
  
  float m_angle{};

  glm::mat4 m_viewMatrix{1.0f};
  glm::mat4 m_projMatrix{1.0f};
  float m_FOV{140.0f};

  int hp_qtt{2};
  abcg::ElapsedTimer m_hitTimer;
  bool lost{false};
  abcg::ElapsedTimer m_restartWaitTimer;

  void randomizeAsteroid(glm::vec3 &position, glm::vec3 &rotation);
  void randomizePlanet(glm::vec3 &position, glm::vec3 &rotation);

  void update();
  void restart();
  glm::vec3 m_shipPosition = glm::vec3(0.0f, 0.0f, 0.0f);
  std::vector<const char*> m_shaderNames{"texture"};
  int m_currentProgramIndex{};
  int m_mappingMode{};
 
  // Skybox
  const std::string m_skyShaderName{"skybox"};
  GLuint m_skyVAO{};
  GLuint m_skyVBO{};
  GLuint m_skyProgram{};
  const std::array<glm::vec3, 36>  m_skyPositions{
    glm::vec3{-1, -1, +1}, glm::vec3{+1, -1, +1}, glm::vec3{+1, +1, +1},
    glm::vec3{-1, -1, +1}, glm::vec3{+1, +1, +1}, glm::vec3{-1, +1, +1},
    glm::vec3{+1, -1, -1}, glm::vec3{-1, -1, -1}, glm::vec3{-1, +1, -1},
    glm::vec3{+1, -1, -1}, glm::vec3{-1, +1, -1}, glm::vec3{+1, +1, -1},
    glm::vec3{+1, -1, -1}, glm::vec3{+1, +1, -1}, glm::vec3{+1, +1, +1},
    glm::vec3{+1, -1, -1}, glm::vec3{+1, +1, +1}, glm::vec3{+1, -1, +1},
    glm::vec3{-1, -1, +1}, glm::vec3{-1, +1, +1}, glm::vec3{-1, +1, -1},
    glm::vec3{-1, -1, +1}, glm::vec3{-1, +1, -1}, glm::vec3{-1, -1, -1},
    glm::vec3{-1, +1, +1}, glm::vec3{+1, +1, +1}, glm::vec3{+1, +1, -1},
    glm::vec3{-1, +1, +1}, glm::vec3{+1, +1, -1}, glm::vec3{-1, +1, -1},
    glm::vec3{-1, -1, -1}, glm::vec3{+1, -1, -1}, glm::vec3{+1, -1, +1},
    glm::vec3{-1, -1, -1}, glm::vec3{+1, -1, +1}, glm::vec3{-1, -1, +1}
  };
  void initializeSkybox();
  void renderSkybox();
  void terminateSkybox();
  void loadModel(std::string path_obj, std::string path_text, Model &model);
};

#endif