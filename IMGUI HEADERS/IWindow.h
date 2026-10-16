#pragma once

#include "include/simpleGta.h"

#include "IWindowItem.h"
#include "IWindowItemButton.h"
#include "IWindowItemOptions.h"

#include <string>
#include <functional>

class IWindow {
public:
  std::string title = "Titulo";
  std::string subTitle = "";

  // Title-bar "X" button handler. Set by DexTrainerUI per-window
  // (back to parent, or close all). Empty = default Close().
  std::function<void()> onXButton;

  // Sticky back button: when true, the LAST item in the window is rendered
  // outside pagination — always visible at the bottom. Set by DexTrainerUI
  // after adding the "< Back" button.
  bool stickyBackButton = false;

  virtual ~IWindow() {};

  virtual IWindowItem *AddText(std::string text) = 0;
  virtual IWindowItem *AddCheckbox(std::string text, bool *pBool) = 0;
  virtual IWindowItemButton *AddButton(std::string text) = 0;
  virtual IWindowItemOptions *AddOptions(std::string text) = 0;
  virtual IWindowItemOptions *AddFloatRange(std::string text,
                                            float *pFloat) = 0;
  virtual IWindowItemOptions *AddIntRange(std::string text, int *pInt, int min,
                                          int max) = 0;
  virtual IWindowItem *AddCustomItem(std::string text, float height) = 0;

  virtual void Close() = 0;
};