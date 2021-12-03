#include "openglwindow.hpp"

#include <imgui.h>

#include <cppitertools/itertools.hpp>
#include <glm/gtx/fast_trigonometry.hpp>

void OpenGLWindow::initializeGL() {
  abcg::glClearColor(0, 0, 0, 1);
  abcg::glEnable(GL_DEPTH_TEST);
  m_program = createProgramFromFile(getAssetsPath() + "depth.vert", getAssetsPath() + "depth.frag");
  m_asteroid.loadObj(getAssetsPath() + "asteroid.obj");
  m_ship.loadObj(getAssetsPath() + "ship.obj");
  m_planetRing.loadObj(getAssetsPath() + "planetRing.obj");
  m_planetRound.loadObj(getAssetsPath() + "planetRound.obj");
  m_asteroid.setupVAO(m_program);
  m_ship.setupVAO(m_program);
  m_planetRing.setupVAO(m_program);
  m_planetRound.setupVAO(m_program);

  m_viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f),glm::vec3(0.0f, 1.0f, 0.0f));
  for (const auto index : iter::range(m_numAsteroids)) {
    auto &position{m_asteroidPositions.at(index)};
    auto &rotation{m_asteroidRotations.at(index)};
    randomizeAsteroid(position, rotation);
  }
  for (const auto index : iter::range(m_numPlanets)) {
    auto &position{m_planetPositions.at(index)};
    auto &rotation{m_planetRotations.at(index)};
    randomizePlanet(position, rotation);
  }
}

void OpenGLWindow::randomizeAsteroid(glm::vec3 &position, glm::vec3 &rotation) {
  std::uniform_real_distribution<float> distPosXY(-20.0f, 20.0f);
  std::uniform_real_distribution<float> distPosZ(-100.0f, 0.0f);
  position = glm::vec3(distPosXY(m_randomEngine), distPosXY(m_randomEngine), distPosZ(m_randomEngine));
  std::uniform_real_distribution<float> distRotAxis(-1.0f, 1.0f);
  rotation = glm::normalize(glm::vec3(distRotAxis(m_randomEngine), distRotAxis(m_randomEngine), distRotAxis(m_randomEngine)));
}

void OpenGLWindow::randomizePlanet(glm::vec3 &position, glm::vec3 &rotation) {
  std::uniform_real_distribution<float> distPosXY(-10.0f, 10.0f);
  std::uniform_real_distribution<float> distPosZ(-100.0f, 0.0f);
  float X = distPosXY(m_randomEngine); 
  float Y = distPosXY(m_randomEngine);
  if(Y <= 0 && X <= 0){
    if(abs(X)>=5 && abs(Y)>=5) position = glm::vec3(X-10.0f, Y-10.0f, distPosZ(m_randomEngine));
    else if(abs(X) > abs(Y)) position = glm::vec3(X-10.0f, Y, distPosZ(m_randomEngine));
    else if(abs(X) < abs(Y)) position = glm::vec3(X, Y-10.0f, distPosZ(m_randomEngine));
  }
  if(Y <= 0 && X > 0){
    if(X>=5 && abs(Y)>=5) position = glm::vec3(X+10.0f, Y-10.0f, distPosZ(m_randomEngine));
    else if(X>abs(Y)) position = glm::vec3(X+10.0f, Y, distPosZ(m_randomEngine));
    else if(X<abs(Y)) position = glm::vec3(X, Y-10.0f, distPosZ(m_randomEngine));
  }
  if(Y > 0 && X < 0){
    if(abs(X)>=5 && Y>=5) position = glm::vec3(X-10.0f, Y+10.0f, distPosZ(m_randomEngine));
    else if(abs(X) > Y) position = glm::vec3(X-10.0f, Y, distPosZ(m_randomEngine));
    else if(abs(X) < Y) position = glm::vec3(X, Y+10.0f, distPosZ(m_randomEngine));
  }
  if(Y > 0 && X > 0){
    if(X>=5 && Y>=5) position = glm::vec3(X+10.0f, Y+10.0f, distPosZ(m_randomEngine));
    else if(X>Y) position = glm::vec3(X+10.0f, Y, distPosZ(m_randomEngine));
    else if(X<Y) position = glm::vec3(X, Y+10.0f, distPosZ(m_randomEngine));
  }
  std::uniform_real_distribution<float> distRotAxis(-0.0005f, 0.0005f);
  rotation = glm::normalize(glm::vec3(distRotAxis(m_randomEngine), distRotAxis(m_randomEngine), 0.0f));
}

void OpenGLWindow::paintGL() {
  update();
  abcg::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  abcg::glViewport(0, 0, m_viewportWidth, m_viewportHeight);
  abcg::glUseProgram(m_program);
  const GLint viewMatrixLoc{abcg::glGetUniformLocation(m_program, "viewMatrix")};
  const GLint projMatrixLoc{abcg::glGetUniformLocation(m_program, "projMatrix")};
  const GLint modelMatrixLoc{abcg::glGetUniformLocation(m_program, "modelMatrix")};
  const GLint colorLoc{abcg::glGetUniformLocation(m_program, "color")};
  abcg::glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &m_viewMatrix[0][0]);
  abcg::glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &m_projMatrix[0][0]);
  abcg::glUniform4f(colorLoc, 0.6f, 0.2f, 0.0f, 1.0f);

  for (const auto index : iter::range(m_numAsteroids)) {
    const auto &position{m_asteroidPositions.at(index)};
    const auto &rotation{m_asteroidRotations.at(index)};
    glm::mat4 modelMatrix{1.0f};
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f));
    modelMatrix = glm::rotate(modelMatrix, m_angle, rotation);
    abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &modelMatrix[0][0]);
    m_asteroid.render();
  }

  //render da ship
  glm::mat4 modelShipMatrix{1.0f};
  modelShipMatrix = glm::translate(modelShipMatrix, m_shipPosition);
  modelShipMatrix = glm::scale(modelShipMatrix, glm::vec3(1.50f));
  modelShipMatrix = glm::rotate(modelShipMatrix, 0.0f, m_shipRotation);
  abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &modelShipMatrix[0][0]);
  abcg::glUniform4f(colorLoc, 1.0f, 0.0f, 0.0f, 1.0f);
  m_ship.render();


  for (const auto index : iter::range(m_numPlanets)) {
    const auto &position{m_planetPositions.at(index)};
    const auto &rotation{m_planetRotations.at(index)};
    glm::mat4 modelMatrix{1.0f};
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f));
    modelMatrix = glm::rotate(modelMatrix, m_angle, rotation);
    abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &modelMatrix[0][0]);
    if(index < 3){
      abcg::glUniform4f(colorLoc, 0.93f, 0.82f, 0.0f, 1.0f);
      m_planetRing.render();
    }
    if(index >= 3 && index < 6){
      abcg::glUniform4f(colorLoc, 0.0f, 0.5f, 0.9f, 1.0f);
      m_planetRound.render();
    }
    if(index >= 6 && index < 9){
      abcg::glUniform4f(colorLoc, 0.0f, 0.5f, 0.5f, 1.0f);
      m_planetRound.render();
    }
    if(index >= 9 && index < 12){
      abcg::glUniform4f(colorLoc, 0.8f, 0.0f, 0.6f, 1.0f);
      m_planetRing.render();
    }
  }
  abcg::glUseProgram(0);
}

void OpenGLWindow::paintUI() {
  abcg::OpenGLWindow::paintUI();
  {
    const auto widgetSize{ImVec2(218, 32)};
    ImGui::SetNextWindowPos(ImVec2(m_viewportWidth - widgetSize.x - 5, 5));
    ImGui::SetNextWindowSize(widgetSize);
    ImGui::Begin("Widget window", nullptr, ImGuiWindowFlags_NoDecoration);
    {
      ImGui::PushItemWidth(120);
      static std::size_t currentIndex{};
      ImGui::PopItemWidth();
      ImGui::PushItemWidth(170);
      const auto aspect{static_cast<float>(m_viewportWidth) /
                        static_cast<float>(m_viewportHeight)};
      if (currentIndex == 0) {
        m_projMatrix =
            glm::perspective(glm::radians(m_FOV), aspect, 0.01f, 100.0f);
        ImGui::SliderFloat("FOV", &m_FOV, 5.0f, 179.0f, "%.0f degrees");
      } else {
        m_projMatrix = glm::ortho(-20.0f * aspect, 20.0f * aspect, -20.0f,
                                  20.0f, 0.01f, 100.0f);
      }
      ImGui::PopItemWidth();
    }
    ImGui::End();
  }
}

void OpenGLWindow::resizeGL(int width, int height) {
  m_viewportWidth = width;
  m_viewportHeight = height;
}

void OpenGLWindow::terminateGL() {
  m_asteroid.terminateGL();
  m_planetRing.terminateGL();
  m_planetRound.terminateGL();
  m_ship.terminateGL();
  abcg::glDeleteProgram(m_program);
}

void OpenGLWindow::update() {
  float rnd = sin(getElapsedTime())*10.0f;
  float rndAst = sin(getElapsedTime())*3.0f;
  const float deltaTime{static_cast<float>(getDeltaTime())};
  m_angle = glm::wrapAngle(m_angle + glm::radians(90.0f) * deltaTime);
  for (const auto index : iter::range(m_numAsteroids)) {
    auto &position{m_asteroidPositions.at(index)};
    auto &rotation{m_asteroidRotations.at(index)};
    position.z += deltaTime * 10.0f;
    position.y += deltaTime* rndAst;
    if (position.z > 0.1f) {
      randomizeAsteroid(position, rotation);
      position.z = -100.0f;
    }
  }
  for (const auto index : iter::range(m_numPlanets)) {
    auto &position{m_planetPositions.at(index)};
    auto &rotation{m_planetRotations.at(index)};
    position.z += deltaTime * 10.0f;
    if (position.z > 0.1f) {
      randomizePlanet(position, rotation);
      position.z = -100.0f;
    }
  }
  
  m_shipPosition.x = rnd;
}