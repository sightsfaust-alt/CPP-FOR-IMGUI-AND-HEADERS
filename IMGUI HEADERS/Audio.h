#pragma once

#include "pch.h"

#include "menuSZK/IAudio.h"

class Audio : public IAudio {
private:
    bool is3D = false;
public:
    CAudioStream* stream = nullptr;

    ~Audio() override;

    void LoadFromSource(std::string src, bool in3d);
    void DestroyStream();
    C3DAudioStream* Get3DStream();

    //

    void Play() override;
    void Stop() override;
    void SetLoop(bool loop) override;
    bool Finished() override;
    bool Loaded() override;
    bool Is3D() override;
    void AttachToCPlaceable(void* ptr) override;
    void SetVolume(float volume) override;
};