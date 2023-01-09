#include "ImGuiLoop.h"

class MyClient : virtual public ImGuiLoop::ImGuiLoopClient 
{
	public:
		// Here you draw your GUI. Running in main thread
		virtual bool DrawFrame()
		{
			bool quit = false;

			ImVec2 size = ImGui::GetIO().DisplaySize;
			ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
			ImGui::SetNextWindowSize(size);
			ImGui::Begin("Demo window", 0, ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);
				if (ImGui::Button("Hello!"))
					printf("onBtnHello\n");
				if (ImGui::Button("Bye!")) {
					printf("onBtnBye\n");
					quit = true; // returning true ends application
				}
			ImGui::End();

			
			ImGui::SetNextWindowPos(ImVec2((size.x-100.0f)/2.0f, (size.y-100.0f)/2.0f));
			ImGui::SetNextWindowSize(ImVec2(100, 100));
			ImGui::Begin("window 2", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
				if (ImGui::Button("Test!"))
					printf("onBtnTest\n");

			ImGui::End();
			
			return quit;
		}
};

MyClient client;

int main()
{
	// init framework in ST mode
	if (!ImGuiLoop::Init(&client, false))
		return 1;

	if (!ImGuiLoop::CreateWindow(500, 500, "test"))
		return 1;

	return 0;
}