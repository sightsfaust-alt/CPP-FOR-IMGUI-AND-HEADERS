#include "Audio.h"

#include "Debug.h"

Audio::~Audio()
{

}

void Audio::LoadFromSource(std::string src, bool in3d)
{
    is3D = in3d;

    //menuDebug->AddLine("Loading audio " + src);

    if(!file_exists(src))
    {
        menuDebug->AddLine("~r~Audio not found: " + src);
        return;
    }

    if(BASS)
    {
        stream = soundsys->LoadStream(src.c_str(), in3d);

        if(stream)
        {
            //menuDebug->AddLine("~g~Audio loaded");
        } else {
            menuDebug->AddLine("~r~Audio was not loaded: " + src);
        }
    }
}

void Audio::DestroyStream()
{
    if(stream == nullptr) return;

    soundsys->UnloadStream(stream);
    stream = nullptr;
}


C3DAudioStream* Audio::Get3DStream()
{
    if(!is3D) return nullptr;

    return (C3DAudioStream*)stream;
}

void Audio::Play()
{
    if(!Loaded()) return;

    stream->Play();
}

void Audio::Stop()
{
    if(!Loaded()) return;

    stream->Stop();
}

void Audio::SetLoop(bool loop)
{
    if(!Loaded()) return;

    stream->Loop(loop);
}

bool Audio::Finished()
{
    if(!Loaded()) return true;

    return stream->finished;
}

bool Audio::Loaded()
{
    if(stream == nullptr) return false;
    return stream->OK;
}

bool Audio::Is3D()
{
    return is3D;
}

void Audio::AttachToCPlaceable(void* ptr)
{
    auto stream = Get3DStream();
    if(!stream) return;

    auto vehicle = (CPlaceable*)ptr;

    stream->Link(vehicle);
}

void Audio::SetVolume(float volume)
{
    if(stream == nullptr) return;
    stream->SetVolume(volume);
}