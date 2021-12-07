#include "openglwindow.hpp"

#include <imgui.h>

#include <cppitertools/itertools.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <glm/gtc/matrix_inverse.hpp>

void OpenGLWindow::handleEvent(SDL_Event& handleEvent) {
  const float deltaTime{static_cast<float>(getDeltaTime())};
  if (handleEvent.type == SDL_KEYDOWN) {
    if (handleEvent.key.keysym.sym == SDLK_UP || handleEvent.key.keysym.sym == SDLK_w){
      if(m_shipPosition.y <= 0.2f) m_shipPosition.y += deltaTime * 2.0f;
    }
    if (handleEvent.key.keysym.sym == SDLK_LEFT || handleEvent.key.keysym.sym == SDLK_a){
      if(m_shipPosition.x >= -0.2f) m_shipPosition.x -= deltaTime * 2.0f;
    }
    if (handleEvent.key.keysym.sym == SDLK_DOWN || handleEvent.key.keysym.sym == SDLK_s){
      if(m_shipPosition.y >= -0.2f) m_shipPosition.y -= deltaTime * 2.0f;
    }
    if (handleEvent.key.keysym.sym == SDLK_RIGHT || handleEvent.key.keysym.sym == SDLK_d){
      if(m_shipPosition.x <= 0.2f) m_shipPosition.x += deltaTime * 2.0f;
    }
  }
}

void OpenGLWindow::initializeGL() {
  abcg::glClearColor(0, 0, 0, 1);
  abcg::glEnable(GL_DEPTH_TEST);
  for (const auto& name : m_shaderNames) {
    const auto path{getAssetsPath() + "shaders/" + name};
    const auto program{createProgramFromFile(path + ".vert", path + ".frag")};
    m_programs.push_back(program);
  }
  m_mappingMode = 3;
  m_viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  
  //sky
  m_skybox.loadCubeTexture(getAssetsPath() + "maps/cube/");
  initializeSkybox();
  
  //asteroids
  loadModel("asteroid.obj", "asteroid.jpg", m_asteroid);
  for (const auto index : iter::range(m_numAsteroids)) {
    auto &position{m_asteroidPositions.at(index)};
    auto &rotation{m_asteroidRotations.at(index)};
    randomizeAsteroid(position, rotation);
  }

  //planets
  loadModel("planetRound.obj", "planetRound.jpg", m_planetRound);
  loadModel("planetRing.obj", "planetRing.jpg", m_planetRing);
  for (const auto index : iter::range(m_numPlanets)) {
    auto &position{m_planetPositions.at(index)};
    auto &rotation{m_planetRotations.at(index)};
    randomizePlanet(position, rotation);
  }
  
  //ship
  loadModel("ship.obj", "ship.jpg", m_ship);
  m_shipPosition = glm::vec3(0.0f, -0.05f, -0.085f);
  hp_qtt = 3;
}

void OpenGLWindow::initializeSkybox() {	
  const auto path{getAssetsPath() + "shaders/" + m_skyShaderName};	
  m_skyProgram = createProgramFromFile(path + ".vert", path + ".frag");	
  abcg::glGenBuffers(1, &m_skyVBO);	
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_skyVBO);	
  abcg::glBufferData(GL_ARRAY_BUFFER, sizeof(m_skyPositions),	
                     m_skyPositions.data(), GL_STATIC_DRAW);	
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);	
  const GLint positionAttributeSky{abcg::glGetAttribLocation(m_skyProgram, "inPosition")};	
  abcg::glGenVertexArrays(1, &m_skyVAO);	
  abcg::glBindVertexArray(m_skyVAO);	
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_skyVBO);	
  abcg::glEnableVertexAttribArray(positionAttributeSky);	
  abcg::glVertexAttribPointer(positionAttributeSky, 3, GL_FLOAT, GL_FALSE, 0, nullptr);	
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);	
  abcg::glBindVertexArray(0);	
}

void OpenGLWindow::loadModel(std::string path_obj, std::string path_text, Model &model) {
  model.terminateGL();
  model.loadDiffuseTexture(getAssetsPath() + "maps/" + path_text);
  model.loadNormalTexture(getAssetsPath() + "maps/pattern_normal.png");
  model.loadObj(getAssetsPath() + path_obj);
  model.setupVAO(m_programs.at(m_currentProgramIndex));
  model.m_Ka = model.getKa();
  model.m_Kd = model.getKd();
  model.m_Ks = model.getKs();
  model.m_shininess = model.getShininess();
}

void OpenGLWindow::randomizeAsteroid(glm::vec3 &position, glm::vec3 &rotation) {
  std::uniform_real_distribution<float> distPosXY(-20.0f, 20.0f);
  std::uniform_real_distribution<float> distPosZ(-100.0f, 0.0f);
  position = glm::vec3(distPosXY(m_randomEngine), distPosXY(m_randomEngine),
                       distPosZ(m_randomEngine));
  std::uniform_real_distribution<float> distRotAxis(-1.0f, 1.0f);
  rotation = glm::normalize(glm::vec3(distRotAxis(m_randomEngine),
                                      distRotAxis(m_randomEngine),
                                      distRotAxis(m_randomEngine)));
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
  const auto program{m_programs.at(m_currentProgramIndex)};
  abcg::glUseProgram(program);
  const GLint viewMatrixLoc{abcg::glGetUniformLocation(program, "viewMatrix")};
  const GLint projMatrixLoc{abcg::glGetUniformLocation(program, "projMatrix")};
  const GLint modelMatrixLoc{abcg::glGetUniformLocation(program, "modelMatrix")};
  const GLint colorLoc{abcg::glGetUniformLocation(program, "color")};
  const GLint normalMatrixLoc{abcg::glGetUniformLocation(program, "normalMatrix")};
  const GLint lightDirLoc{abcg::glGetUniformLocation(program, "lightDirWorldSpace")};
  const GLint shininessLoc{abcg::glGetUniformLocation(program, "shininess")};
  const GLint IaLoc{abcg::glGetUniformLocation(program, "Ia")};
  const GLint IdLoc{abcg::glGetUniformLocation(program, "Id")};
  const GLint IsLoc{abcg::glGetUniformLocation(program, "Is")};
  const GLint KaLoc{abcg::glGetUniformLocation(program, "Ka")};
  const GLint KdLoc{abcg::glGetUniformLocation(program, "Kd")};
  const GLint KsLoc{abcg::glGetUniformLocation(program, "Ks")};
  const GLint diffuseTexLoc{abcg::glGetUniformLocation(program, "diffuseTex")};
  const GLint mappingModeLoc{abcg::glGetUniformLocation(program, "mappingMode")};
  const GLint normalTexLoc{abcg::glGetUniformLocation(program, "normalTex")};	
  const GLint cubeTexLoc{abcg::glGetUniformLocation(program, "cubeTex")};
  const GLint texMatrixLoc{abcg::glGetUniformLocation(program, "texMatrix")};

  abcg::glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &m_viewMatrix[0][0]);
  abcg::glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &m_projMatrix[0][0]);
  abcg::glUniform4f(colorLoc, 0.6f, 0.2f, 0.0f, 1.0f);
  abcg::glUniform1i(diffuseTexLoc, 0);
  abcg::glUniform1i(normalTexLoc, 1);	
  abcg::glUniform1i(cubeTexLoc, 2);
  abcg::glUniform1i(mappingModeLoc, m_mappingMode);
  const glm::mat3 texMatrix{glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3{1.0f})};
  abcg::glUniformMatrix3fv(texMatrixLoc, 1, GL_TRUE, &texMatrix[0][0]);
  abcg::glUniform4fv(lightDirLoc, 1, &m_asteroid.m_lightDir.x);
  abcg::glUniform4fv(IaLoc, 1, &m_asteroid.m_Ia.x);
  abcg::glUniform4fv(IdLoc, 1, &m_asteroid.m_Id.x);
  abcg::glUniform4fv(IsLoc, 1, &m_asteroid.m_Is.x);

  for (const auto index : iter::range(m_numAsteroids)) {
    abcg::glFrontFace(GL_CCW);
    const auto &position{m_asteroidPositions.at(index)};
    const auto &rotation{m_asteroidRotations.at(index)};
    glm::mat4 modelMatrix{1.0f};
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.2f));
    modelMatrix = glm::rotate(modelMatrix, m_angle, rotation);
    abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &modelMatrix[0][0]);
    const auto modelViewMatrix{glm::mat3(m_viewMatrix * modelMatrix)};
    glm::mat3 normalMatrix{glm::inverseTranspose(modelViewMatrix)};
    abcg::glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, &normalMatrix[0][0]);
    abcg::glUniform1f(shininessLoc, m_asteroid.m_shininess);
    abcg::glUniform4fv(KaLoc, 1, &m_asteroid.m_Ka.x);
    abcg::glUniform4fv(KdLoc, 1, &m_asteroid.m_Kd.x);
    abcg::glUniform4fv(KsLoc, 1, &m_asteroid.m_Ks.x);
    m_asteroid.render();
    
  }
  
  glm::mat4 modelShipMatrix{1.0f};
  abcg::glFrontFace(GL_CCW);
  modelShipMatrix = glm::translate(modelShipMatrix, m_shipPosition);
  modelShipMatrix = glm::scale(modelShipMatrix, glm::vec3(0.07f));
  abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &modelShipMatrix[0][0]);
  const auto modelViewMatrix{glm::mat3(m_viewMatrix * modelShipMatrix)};
  glm::mat3 normalMatrix{glm::inverseTranspose(modelViewMatrix)};
  abcg::glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, &normalMatrix[0][0]);
  abcg::glUniform1f(shininessLoc, m_ship.m_shininess);
  abcg::glUniform4fv(KaLoc, 1, &m_ship.m_Ka.x);
  abcg::glUniform4fv(KdLoc, 1, &m_ship.m_Kd.x);
  abcg::glUniform4fv(KsLoc, 1, &m_ship.m_Ks.x);
  m_ship.render();

  for (const auto index : iter::range(m_numPlanets)) {
    abcg::glFrontFace(GL_CCW);
    const auto &position{m_planetPositions.at(index)};
    const auto &rotation{m_planetRotations.at(index)};
    glm::mat4 modelMatrix{1.0f};
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f));
    modelMatrix = glm::rotate(modelMatrix, m_angle, rotation);
    abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &modelMatrix[0][0]);
	  const auto modelViewMatrix{glm::mat3(m_viewMatrix * modelMatrix)};
    glm::mat3 normalMatrix{glm::inverseTranspose(modelViewMatrix)};
    abcg::glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, &normalMatrix[0][0]);
    abcg::glUniform1f(shininessLoc, m_planetRing.m_shininess);
	  abcg::glUniform4fv(KaLoc, 1, &m_planetRing.m_Ka.x);
    abcg::glUniform4fv(KdLoc, 1, &m_planetRing.m_Kd.x);
    abcg::glUniform4fv(KsLoc, 1, &m_planetRing.m_Ks.x);
	  abcg::glUniform1f(shininessLoc, m_planetRound.m_shininess);
    abcg::glUniform4fv(KaLoc, 1, &m_planetRound.m_Ka.x);
    abcg::glUniform4fv(KdLoc, 1, &m_planetRound.m_Kd.x);
    abcg::glUniform4fv(KsLoc, 1, &m_planetRound.m_Ks.x);
    if(index < 3){
      m_planetRing.render();
    }
    if(index >= 3 && index < 6){
      m_planetRound.render();
    }
    if(index >= 6 && index < 9){
      m_planetRound.render();
    }
    if(index >= 9 && index < 12){
      m_planetRing.render();
    }
  }
  abcg::glUseProgram(0);

  abcg::glUseProgram(m_skyProgram);
  const GLint viewMatrixLocSky{abcg::glGetUniformLocation(m_skyProgram, "viewMatrix")};	
  const GLint projMatrixLocSky{abcg::glGetUniformLocation(m_skyProgram, "projMatrix")};	
  const GLint skyTexLoc{abcg::glGetUniformLocation(m_skyProgram, "skyTex")};	
  const auto viewMatrixSky{glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3{1.0f})};	
  abcg::glUniformMatrix4fv(viewMatrixLocSky, 1, GL_FALSE, &viewMatrixSky[0][0]);	
  abcg::glUniformMatrix4fv(projMatrixLocSky, 1, GL_FALSE, &m_projMatrix[0][0]);	
  abcg::glUniform1i(skyTexLoc, 0);	
  abcg::glBindVertexArray(m_skyVAO);	
  abcg::glActiveTexture(GL_TEXTURE0);	
  abcg::glBindTexture(GL_TEXTURE_CUBE_MAP, m_skybox.getCubeTexture());	
  abcg::glEnable(GL_CULL_FACE);	
  abcg::glFrontFace(GL_CW);	
  abcg::glDepthFunc(GL_LEQUAL);	
  abcg::glDrawArrays(GL_TRIANGLES, 0, m_skyPositions.size());	
  abcg::glDepthFunc(GL_LESS);	
  abcg::glBindVertexArray(0);	
  abcg::glUseProgram(0);
  
}

void OpenGLWindow::paintUI() {
  abcg::OpenGLWindow::paintUI();
  {
    const auto widgetSize{ImVec2(222, 85)};
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
      ImGui::Text("HEALTH POINTS: %d", hp_qtt);
      ImGui::Text("SCORE: %d", score);
      ImGui::PopItemWidth();
    }
    ImGui::End();
  }

  {
    const auto size{ImVec2(150, 150)};
    const auto position{ImVec2((m_viewportWidth - size.x) / 2.0f,
                               (m_viewportHeight - size.y) / 2.0f)};
    ImGui::SetNextWindowPos(position);
    ImGui::SetNextWindowSize(size);
    ImGuiWindowFlags flags{ImGuiWindowFlags_NoBackground |
                           ImGuiWindowFlags_NoTitleBar |
                           ImGuiWindowFlags_NoInputs};
    ImGui::Begin(" ", nullptr, flags);
    if(lost) ImGui::Text(" *GAME OVER!* ");
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
  m_skybox.terminateGL();
  for (const auto& program : m_programs) {
    abcg::glDeleteProgram(program);
  }
  terminateSkybox();
}

void OpenGLWindow::terminateSkybox() {	
  abcg::glDeleteProgram(m_skyProgram);	
  abcg::glDeleteBuffers(1, &m_skyVBO);	
  abcg::glDeleteVertexArrays(1, &m_skyVAO);	
}

void OpenGLWindow::update() {
  score++;
  float rndAst = sin(getElapsedTime())*3.0f;
  const float deltaTime{static_cast<float>(getDeltaTime())};
  m_angle = glm::wrapAngle(m_angle + glm::radians(90.0f) * deltaTime);
  for (const auto index : iter::range(m_numPlanets)) {
    auto &position{m_planetPositions.at(index)};
    auto &rotation{m_planetRotations.at(index)};
    position.z += deltaTime * 10.0f;
    if (position.z > 0.1f) {
      randomizePlanet(position, rotation);
      position.z = -100.0f;
    }
    if(lost) position.z = 20.0f;
  } 

   for (const auto index : iter::range(m_numAsteroids)) {
    auto &position{m_asteroidPositions.at(index)};
    auto &rotation{m_asteroidRotations.at(index)};
    position.z += deltaTime * 10.0f;
    position.y += deltaTime* rndAst;
    if(!lost){
      if (position.z > 0.1f) {
        randomizeAsteroid(position, rotation);
        position.z = -100.0f;
      }
      if ((m_shipPosition.x <= position.x + 1.0f && m_shipPosition.x >= position.x - 1.0f) && (m_shipPosition.y <= position.y + 1.0f && m_shipPosition.y >= position.y - 1.0f) && (m_shipPosition.z <= position.z + 1.0f && m_shipPosition.z >= position.z - 1.0f)){
        if(m_hitTimer.elapsed() >= 1){
          hp_qtt--;
          if(hp_qtt == 0){
            m_restartWaitTimer.restart();
            m_shipPosition.z = 20.0f;
            lost = true;            
          }
          m_hitTimer.restart();
        }
      }
    } else {
      score = 0;
      position.z = 20.0f;
    }
  }

  if(lost && m_restartWaitTimer.elapsed() > 4) restart();   
}

void OpenGLWindow::restart() {
    lost = false;
    initializeGL();
}