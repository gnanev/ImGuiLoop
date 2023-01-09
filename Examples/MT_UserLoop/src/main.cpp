#include "ImGuiLoop.h"

class MyClient : virtual public ImGuiLoop::ImGuiLoopClient 
{
	public:
		static int msgHello;
		static int msgBye;
		static int msgTest;

		// Here you draw your GUI. Running in UI thread
		virtual bool DrawFrame()
		{
			ImVec2 size = ImGui::GetIO().DisplaySize;
			ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
			ImGui::SetNextWindowSize(size);
			ImGui::Begin("Demo window", 0, ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);
				if (ImGui::Button("Hello!"))
					ImGuiLoop::SendMessage(msgHello);
				if (ImGui::Button("Bye!"))
					ImGuiLoop::SendMessage(msgBye);
			ImGui::End();

			
			ImGui::SetNextWindowPos(ImVec2((size.x-100.0f)/2.0f, (size.y-100.0f)/2.0f));
			ImGui::SetNextWindowSize(ImVec2(100, 100));
			ImGui::Begin("window 2", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
				if (ImGui::Button("Test!"))
					ImGuiLoop::SendMessage(msgTest);

			ImGui::End();
			
			return false;
		}
};

MyClient client;

// Register messages with no handlers
int MyClient::msgHello = ImGuiLoop::RegisterMessage();
int MyClient::msgBye = ImGuiLoop::RegisterMessage();
int MyClient::msgTest = ImGuiLoop::RegisterMessage();

int main()
{
	// init framework in MT mode
	if (!ImGuiLoop::Init(&client, true))
		return 1;

	if (!ImGuiLoop::CreateWindow(500, 500, "test"))
		return 1;

	ImGuiLoop::Message msg;

	// user message loop with event handling
	while(ImGuiLoop::GetMessage(msg))
	{
		if(msg._id == MyClient::msgHello) {
			printf("onBtnHello\n");
		}
		else if (msg._id == MyClient::msgBye) {
			printf("onBtnBye\n");
			ImGuiLoop::Quit();			
		}
		else if (msg._id == MyClient::msgTest) {
			printf("onBtnTest\n");
		}		
	}

	return 0;
}