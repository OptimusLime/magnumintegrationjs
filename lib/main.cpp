#pragma once
#define NOMINMAX

#include <napi.h>
#include "Magnum/ImGuiIntegration/Context.h"
// #include <SDL2/SDL.h>
// #include <Magnum/Platform/Sdl2Application.h>

#include "easylogging++.h"
INITIALIZE_EASYLOGGINGPP

Napi::Object PluginInit(Napi::Env env, Napi::Object exports)
{
    LOG(INFO) << "C++::main.cpp::Finished Initializing all Export \n";
    return exports;
}

NODE_API_MODULE(MagnumIntegration, PluginInit)