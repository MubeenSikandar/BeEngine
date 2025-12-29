#pragma once

#include "Core.hpp"

namespace BeEngine {

    /**
     * @brief Abstract graphics context interface
     *
     * Represents a graphics API context (OpenGL, Vulkan, DirectX, etc.)
     * This provides platform-independent context management.
     *
     * Design Pattern: Strategy Pattern
     * - GraphicsContext = Strategy Interface
     * - OpenGLContext = Concrete Strategy (OpenGL)
     * - VulkanContext = Concrete Strategy (Vulkan)
     */

     class BE_API GraphicsContext{
         public:
          virtual ~GraphicsContext() = default;

          /**
             * @brief Initialize the graphics context
             *
             * Creates and activates the rendering context.
             * Must be called after window creation.
             */
             virtual void Init() = 0;


             /**
              * @brief Swap front and back buffers (double buffering)
              *
              * Presents the rendered frame to the screen.
              * Called at the end of each frame.
              */
              virtual void SwapBuffers() = 0;

              /**
                 * @brief Make this context current on the calling thread
                 *
                 * Required before making any rendering calls.
                 * OpenGL contexts are thread-local.
                 */
                 virtual void MakeCurrent() = 0;

                 /**
                    * @brief Get context information
                    */
                   virtual const char* GetRendererInfo() const = 0;
                   virtual const char* GetVersionInfo() const = 0;

                   /**
                    * @brief Factory method to create a context for the current API
                    *
                    * @param windowHandle Native window handle (GLFWwindow*, HWND, etc.)
                    * @return Unique pointer to the created context
                    */
                   static std::unique_ptr<GraphicsContext> Create(void* windowHandle);
     };

}
