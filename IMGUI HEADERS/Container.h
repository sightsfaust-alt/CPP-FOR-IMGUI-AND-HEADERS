#pragma once

#include "menuSZK/IContainer.h"
#include "Texture.h"

#include <mutex>

// Guards the container tree (children vectors, parent links) against the
// race between the touch-event thread (Input::ProcessTouchEvent ->
// GetContainerOverThatPoint) and the game thread (window/widget destruction
// mutating the same vectors). Recursive: destructors and Update/Draw nest.
extern std::recursive_mutex g_containerTreeMutex;

// Max recursion depth for GetContainerOverThatPoint's findTop — protects
// against infinite recursion if the tree ever contains a cycle.
static constexpr int kMaxContainerFindDepth = 64;

class Container : public IContainer {
public:
    CVector2D cachedPosition;

    Texture* backgroundImage = nullptr;

    Container(std::string tag);
    ~Container() override;

    IContainer* AddChild(IContainer* child) override;
    void RemoveChild(IContainer* child, bool destroy = false) override;

    void SetBackgroundImage(std::string src) override;
    void SetBackgroundImage(void* texture) override;

    //

    void ProcessPositionAndSize();

    void Update();
    void Draw();

    bool IsPointInside(const CVector2D& point);
    bool IsVisible();

    std::vector<Container*> GetAllChildrenRecursive();

    static Container* GetContainerOverThatPoint(const CVector2D& point);

private:
    int touchThatBeganDrag = -1;
};