#include <memory>
#include <optional>
#include <functional>
#include <thread>
#include <mutex>
#include <vector>
#include "ImGuiLoop.h"

using namespace ImGuiLoop;

#define MSG_QUIT                    0
#define MSG_INIT                    1
#define MSG_CREATE_WINDOW           3
#define MSG_SYS_LAST               20

#define MSG_INIT_RESULT             22
#define MSG_CREATE_WINDOW_RESULT    23
#define MSG_RESULT_LAST             40

const char* glsl_version = "#version 130";
GLFWwindow* mainWindow = NULL;
ImGuiLoopClient* theClient = NULL;
COLOR mainWindowBackground = { 0 };

bool isThreaded = false;

typedef std::vector<messageCallback> callbackVector;
std::unique_ptr<callbackVector> callbacks;

struct MessageQueue
{
    std::queue<ImGuiLoop::Message> messageQueue;
    std::mutex mutexQueue;
};

#define SYSTEM_QUEUE_INDEX  0
#define RESULT_QUEUE_INDEX  1
#define USER_QUEUE_INDEX    2

MessageQueue queueArr[3];

#define USER_MESSAGE_BOUNDARY (MSG_RESULT_LAST + 1) 

int lastRegisteredMessage = USER_MESSAGE_BOUNDARY;

std::unique_ptr<std::thread> thread_obj;

struct WindowCreationData
{
    WindowCreationData(int width, int height, const char* title, const COLOR& background) : 
        _width(width), _height(height), _title(title), _background(background) {}

    int _width;
    int _height;
    std::string _title;
    COLOR _background;
};

static void MainLoop();
static bool DrawInner();
static bool InitInner();
static bool HaveMessageInner(int queue_index);
static void SendMessageInner(int msg_id, void*data, int queue_index = USER_QUEUE_INDEX);
static bool GetMessageInner(Message& msg, int queue_index = USER_QUEUE_INDEX);
static bool PeekSystemMessage(Message& msg);
static bool CreateWindowInner(WindowCreationData* wcd);

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    DrawInner();
}

inline static int MessageType(int msg_id)
{
    if (msg_id < MSG_SYS_LAST)
        return SYSTEM_QUEUE_INDEX;
    
    if (msg_id < MSG_RESULT_LAST)
        return RESULT_QUEUE_INDEX;

    return USER_QUEUE_INDEX;
} 

static void SystemLoop()
{
    Message msg;
    bool clientBreak = false;

    while(!clientBreak)
    {
        if (HaveMessageInner(SYSTEM_QUEUE_INDEX))
        {
            GetMessageInner(msg, SYSTEM_QUEUE_INDEX);

            switch (msg._id)
            {
                case MSG_CREATE_WINDOW:
                    CreateWindowInner(static_cast<WindowCreationData*>(msg._data));
                    break;

                case MSG_QUIT:
                    ImGui::DestroyContext();
                    SendMessageInner(MSG_QUIT, NULL, USER_QUEUE_INDEX); // terminate user message loop
                    return;
            }
        }
        else {
            if (mainWindow != NULL) {
                if (!glfwWindowShouldClose(mainWindow)  && !clientBreak) {
                    clientBreak = DrawInner();
                }
                else {
                    ImGui::DestroyContext();
                    SendMessageInner(MSG_QUIT, NULL, USER_QUEUE_INDEX); // terminate user message loop
                    return;
                }
            }            
            std::this_thread::yield();
        }
    }
}

static void MainWindowLoop()
{
    bool clientBreak = false;

    while (!glfwWindowShouldClose(mainWindow) && !clientBreak) {
        clientBreak = DrawInner();
    }

    ImGui::DestroyContext();
}

static void SendMessageInner(int msg_id, void*data, int queue_index)
{
    queueArr[queue_index].mutexQueue.lock();
    queueArr[queue_index].messageQueue.emplace(msg_id, data); 
    queueArr[queue_index].mutexQueue.unlock();
}

static bool GetMessageInner(Message& msg, int queue_index)
{
    while (!HaveMessageInner(queue_index)) { // always blocking
        std::this_thread::yield();
    }

    queueArr[queue_index].mutexQueue.lock();
    msg = queueArr[queue_index].messageQueue.front();
    queueArr[queue_index].messageQueue.pop();
    queueArr[queue_index].mutexQueue.unlock();
    return msg._id != MSG_QUIT;
}

static bool HaveMessageInner(int queue_index)
{
    queueArr[queue_index].mutexQueue.lock();
    bool ret_val = !queueArr[queue_index].messageQueue.empty();
    queueArr[queue_index].mutexQueue.unlock();
    return ret_val;
}

static bool InitInner()
{
    glfwSetErrorCallback(glfw_error_callback);
    return glfwInit() == GLFW_TRUE;
}

static bool CreateWindowInner(WindowCreationData* wcd)
{
    mainWindow = glfwCreateWindow(wcd->_width, wcd->_height, wcd->_title.c_str(), NULL, NULL);
    if (mainWindow == NULL) {
        fprintf(stderr, "Failed to create main window!\n");
        if (isThreaded)
            SendMessage(MSG_CREATE_WINDOW_RESULT, (void*)false);        
        return false;
    }

    glfwMakeContextCurrent(mainWindow);
    glfwSetFramebufferSizeCallback(mainWindow, framebuffer_size_callback);
    glfwSwapInterval(1);
    
    bool err = glewInit() != GLEW_OK;

    if (err) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        if (isThreaded)
            SendMessage(MSG_CREATE_WINDOW_RESULT, (void*)false);
        return false;
    }

    mainWindowBackground = wcd->_background;
    
    int screen_width, screen_height;
    glfwGetFramebufferSize(mainWindow, &screen_width, &screen_height);
    glViewport(0, 0, screen_width, screen_height);
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    ImGui_ImplGlfw_InitForOpenGL(mainWindow, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    theClient->Start();
    
    if (isThreaded) {
        SendMessage(MSG_CREATE_WINDOW_RESULT, (void*)true);
    }
    else {
        MainWindowLoop();
    }

    return true;    
}

static bool DrawInner()
{
    bool clientBreak = false;

    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    clientBreak = theClient->DrawFrame();
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(mainWindow, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(mainWindowBackground.r, mainWindowBackground.g, mainWindowBackground.b, mainWindowBackground.a);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwMakeContextCurrent(mainWindow);
    glfwSwapBuffers(mainWindow);

    return clientBreak;
}

static void mainProc()
{
    bool result = InitInner();
    SendMessage(MSG_INIT_RESULT, (void*)result);
    SystemLoop();
}

void ImGuiLoop::SendMessage(int msg_id, void*data)
{
    if (!isThreaded)
        return;

    int idx = MessageType(msg_id);
    SendMessageInner(msg_id, data, idx);
}

bool ImGuiLoop::GetMessage(Message& msg)
{
    if (!isThreaded)
        return false;
 
    return GetMessageInner(msg);
}

bool ImGuiLoop::Init(ImGuiLoopClient* client, bool threaded)
{
    bool result = false;

    if (client == NULL) {
        fprintf(stderr, "FATAL: Client NULL!\n");
        return false;
    }

    theClient = client;
    isThreaded = threaded;

    if (isThreaded) {
        thread_obj = std::make_unique<std::thread>(mainProc);
        thread_obj->detach();
        Message msg;
        GetMessageInner(msg, RESULT_QUEUE_INDEX);
        if (msg._id != MSG_INIT_RESULT)
        {
            fprintf(stderr, "FATAL: expected %d (MSG_INIT_RESULT) got %d!\n", MSG_INIT_RESULT, msg._id);
            return false;
        }
        return (bool)msg._data;
    }
    else {
        result = InitInner();
    }

    return result;
}

bool ImGuiLoop::CreateWindow(int width, int height, const char* title, const COLOR& background)
{
    std::unique_ptr<WindowCreationData> wcd = std::make_unique<WindowCreationData>(width, height, title, background);

    if (isThreaded) {
        SendMessage(MSG_CREATE_WINDOW, static_cast<void*>(wcd.get()));
        Message msg;
        GetMessageInner(msg, RESULT_QUEUE_INDEX);
        if (msg._id != MSG_CREATE_WINDOW_RESULT)
        {
            fprintf(stderr, "FATAL: expected %d (MSG_CREATE_WINDOW_RESULT) got %d!\n", MSG_CREATE_WINDOW_RESULT, msg._id);
            SendMessage(MSG_QUIT);
            return false;
        }

        return (bool)msg._data;
    }
    else {
        return CreateWindowInner(wcd.get());
    }

    return false; // should never get here
}

int ImGuiLoop::RegisterMessage(messageCallback callback)
{
    if (callback != NULL) {
        int lastIndex = (lastRegisteredMessage-USER_MESSAGE_BOUNDARY);

        if (!callbacks) {
            callbacks = std::make_unique<callbackVector>();
        }

        size_t size = callbacks->size();
        if (callbacks->size() < lastIndex)
        {
            fprintf(stderr, "Mixing callback/no callback messages are not allowed");
            return -1;
        }
        callbacks->push_back(callback);
        size = callbacks->size();
        int n = (int)size - 1;
    }

    return lastRegisteredMessage++;
}

void ImGuiLoop::Quit()
{
    SendMessage(MSG_QUIT);
}

void ImGuiLoop::DefaultMessageLoop()
{   Message msg;
    while(GetMessage(msg))
	{
		if ((msg._id > lastRegisteredMessage) || (msg._id < (USER_MESSAGE_BOUNDARY-1)))  {
			fprintf(stderr, "WARNING: Unregistered message %d", msg._id);
            continue;
		}

        size_t size = callbacks->size();
        int index = msg._id - USER_MESSAGE_BOUNDARY;
        (*callbacks)[index](msg._data);
	}
}

void ImGuiLoopClient::Start()
{
    ImGui::StyleColorsDark();
}
