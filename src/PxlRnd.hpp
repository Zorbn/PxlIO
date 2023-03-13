#include <memory>

#include "OpenGL/GLRenderer.hpp"

#ifndef EMSCRIPTEN
    #include "Vulkan/VKRenderer.hpp"
#endif

class PxlRnd
{
public:
    static std::shared_ptr<Renderer> Create(const std::string &windowName,
                                            int32_t windowWidth, int32_t windowHeight,
                                            int32_t viewWidth, int32_t viewHeight, bool enableVsync = true)
    {
    #ifdef EMSCRIPTEN
        return std::shared_ptr<Renderer>(new GLRenderer(windowName, windowWidth,
            windowHeight, viewWidth, viewHeight, enableVsync));
    #else
        return std::shared_ptr<Renderer>(new VKRenderer(windowName, windowWidth,
            windowHeight, viewWidth, viewHeight, enableVsync));
    #endif
    }
};