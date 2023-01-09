#ifndef _ImGuiLoop_h_
#define _ImGuiLoop_h_

#include <stdio.h>
#include <iostream>
#include <imgui.h>
#include <queue>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <GL/glew.h> 
#include <GLFW/glfw3.h>

typedef void (*messageCallback)(void*);

#define DEFAULT_BACKGROUND {0.45f, 0.55f, 0.60f, 1.00f}
#define BIND_MSG_CALLBACK(M,C) int M = ImGuiLoop::RegisterMessage(static_cast<messageCallback>([] (void* data) { C(data); })) 

namespace ImGuiLoop
{
    struct COLOR
    {
        GLfloat r, g, b, a;
    };

    struct Message
    {
        Message() {}
        Message(int id, void* data) : _id(id), _data(data) {}
        int     _id;
        void*   _data;
    };

    class ImGuiLoopClient
    {
        public:
            virtual void Start();
            virtual bool DrawFrame() = 0;
    };

    bool Init(ImGuiLoopClient* client, bool threaded = false);
    bool CreateWindow(int width, int height, const char* title, const COLOR& background = DEFAULT_BACKGROUND);

    void SendMessage(int msg_id, void*data = NULL);
    bool GetMessage(Message& msg);
    int  RegisterMessage(messageCallback callback = NULL);
    void Quit();
    void DefaultMessageLoop();
}

#endif /* _ImGuiLoop_h_ */
