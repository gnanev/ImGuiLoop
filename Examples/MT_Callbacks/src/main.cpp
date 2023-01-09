#include "ImGuiLoop.h"

class MyLoop : virtual public ImGuiLoop::ImGuiLoopClient 
{
	public:
		static int msgHello;
		static int msgBye;
		static int msgTest;

		/* Those event handlers are executed in main thread */
		void onBtnHello(void* data)
		{
			printf("onBtnHello\n");
		}

		void onBtnBye(void* data)
		{
			printf("onBtnBye\n");
			ImGuiLoop::Quit();
		}

		void onBtnTest(void* data)
		{
			printf("onBtnTest\n");
		}
		/*====================================================*/

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

MyLoop loop;

// Register messages and bind them to handlers
// Those have no effect, if ImGuiLoop::Init is called for ST mode 
BIND_MSG_CALLBACK(MyLoop::msgHello, loop.onBtnHello);
BIND_MSG_CALLBACK(MyLoop::msgBye, loop.onBtnBye);
BIND_MSG_CALLBACK(MyLoop::msgTest, loop.onBtnTest);

int main()
{
	if (!ImGuiLoop::Init(&loop, true))
		return 1;

	if (!ImGuiLoop::CreateWindow(500, 500, "test"))
		return 1;

	// default message loop
	ImGuiLoop::DefaultMessageLoop();

	// ImGuiLoop::Message msg;

	// while(ImGuiLoop::GetMessage(msg))
	// {
	// 	if(msg._id == msgBtn) {
	// 		printf("BUTTON!!!\n");
	// 	}
	// }

	return 0;
}