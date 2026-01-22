#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Cursor.hpp"
#include "Event.hpp"
#include "Render.hpp"
struct GLFWwindow;
namespace esl
{
    class Sprite;
    class Sprite3D;
    class Cursor;
    class Event;
    class Text;
    class TextM;  // 添加TextM前向声明
    class Shape;
    
    class Window : public RenderTarget
    {
        GLFWwindow* m_Window = nullptr;
        glm::vec4 m_BackgroundColor;
        double m_Framerate = 0;
        bool m_Vsync = false;
        double m_LastFrameTime = 0;
        double m_TimePerFrame = 0;
        Event m_Event;
        Cursor m_Cursor;
        static void KeyEventCallback(GLFWwindow* window, int key, int scancode, int action, int modes);
        static void MouseEventCallback(GLFWwindow* window, int button, int action, int mods);
    public:
        Window(unsigned int width, unsigned int height, const char* title, bool visiable = true, bool resizeable = true);
        ~Window();
        bool isOpen();
        void display();
        void clear();
        void pollEvents(Event& e);
        void showWindow();
        void hideWindow();
        void setBackgroundColor(glm::vec4 rgba_float);
        void setBackgroundColor(glm::uvec4 rgba_int);
        void setWindowSize(glm::vec2 size);
        glm::ivec2 getWindowSize();
        glm::dvec2 getCursorPosition();
        void setWindowPosition(glm::vec2 pos);
        glm::vec2 getWindowPosition();
        void closeWindow();
        void setWindowTitle(const char* title);
        void move(glm::vec2 offset);
        void setVSync(bool value);
        void setFramerateLimit(double framerate);
        virtual void draw(Renderable& renderObject);
        void draw(Text& text);
        void setCursorStyle(Cursor::Style style);
        void setCursorIcon(const char* iconPath);
        void setCursorState(Cursor::State state);
        void setWindowIcon(const char* iconPath);
        GLFWwindow* getWindowHandle();
    };
}
