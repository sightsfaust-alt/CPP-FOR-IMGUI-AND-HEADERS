#pragma once

#include "pch.h"

#include "include/simpleGta.h"

#include "Container.h"

struct TouchInfo {
  int touchId;            // ID do toque
  CVector2D startPos;     // posição inicial do toque
  CVector2D currentPos;   // posição atual
  CVector2D delta;        // movimento desde o último frame
  bool isDown;            // toque ativo
  bool beganThisFrame;    // começou nesse frame
  bool releasedThisFrame; // terminou nesse frame
  Container *overContainer;
};

class Input {
public:
  static std::unordered_map<int, TouchInfo> Touches;

  static void Update();
  static void OnGameUpdate();
  static void Draw();
  static void ResetFlags();
  static bool ProcessTouchEvent(int state, int touchId, int x, int y);
  static CVector2D GetTouchPosition(int touchId);
  static int GetTouchThatContainerIsOn(Container *container);

  // Called from ~Container(): clears every cached raw pointer to the dying
  // container (containerToCallOnClick, TouchInfo::overContainer) so no
  // thread can dereference it after free. Must be called with
  // g_containerTreeMutex held.
  static void OnContainerDestroyed(Container *c);
};