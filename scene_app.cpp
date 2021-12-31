#include "scene_app.h"
#include <system/platform.h>
#include <graphics/sprite_renderer.h>
#include <graphics/font.h>
#include <system/debug_log.h>
#include <graphics/renderer_3d.h>
#include <graphics/mesh.h>
#include <maths/math_utils.h>
#include <input/sony_controller_input_manager.h>
#include <input/touch_input_manager.h>
#include <graphics/sprite.h>
#include "load_texture.h"

#include "imgui.h"
//#include "examples/imgui_impl_dx11.h"
#include "backends/imgui_impl_dx11.h"
//#include "examples/imgui_impl_win32.h"
#include "backends/imgui_impl_win32.h"
#include <platform/d3d11/system/platform_d3d11.h>
#include <platform/d3d11/input/keyboard_d3d11.h>

// Our state
bool show_demo_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

SceneApp::SceneApp(gef::Platform& platform) :
	Application(platform),
	sprite_renderer_(NULL),
	input_manager_(NULL),
	font_(NULL),
	sprite_texture_(NULL)
{
}

void SceneApp::Init()
{
	sprite_renderer_ = gef::SpriteRenderer::Create(platform_);
	InitFont();

	// initialise input manager
	input_manager_ = gef::InputManager::Create(platform_);

	sprite_texture_ = CreateTextureFromPNG("Idle (1).png", platform_);

	sprite_.set_texture(sprite_texture_);
	sprite_.set_position(gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f, -0.99f));
	sprite_.set_height(128.0f);
	sprite_.set_width(128.0f);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	gef::PlatformD3D11& platform_d3d = static_cast<gef::PlatformD3D11&>(platform_);

	ImGui_ImplWin32_Init(platform_d3d.hwnd());
	ImGui_ImplDX11_Init(platform_d3d.device(), platform_d3d.device_context());
}

void SceneApp::CleanUp()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	delete sprite_texture_;
	sprite_texture_ = NULL;

	delete input_manager_;
	input_manager_ = NULL;

	CleanUpFont();

	delete sprite_renderer_;
	sprite_renderer_ = NULL;
}

bool SceneApp::Update(float frame_time)
{
	fps_ = 1.0f / frame_time;


	input_manager_->Update();

	UpdateImGuiIO();

	return true;
}

void SceneApp::Render()
{
	sprite_renderer_->Begin();

	// Render button icon
	sprite_renderer_->DrawSprite(sprite_);

	DrawHUD();
	sprite_renderer_->End();

	ImGuiRender();
}

void SceneApp::InitFont()
{
	font_ = new gef::Font(platform_);
	font_->Load("comic_sans");
}

void SceneApp::CleanUpFont()
{
	delete font_;
	font_ = NULL;
}

void SceneApp::DrawHUD()
{
	if(font_)
	{
		// display frame rate
		font_->RenderText(sprite_renderer_, gef::Vector4(platform_.width() - 120.0f, platform_.height() - 40.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "FPS: %.1f", fps_);
	}
}

void SceneApp::ImGuiRender()
{
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	// 3. Show another simple window.
	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}

	// Rendering
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void SceneApp::UpdateImGuiIO()
{
	ImGuiIO& io = ImGui::GetIO();
	io.MouseDown[0] = input_manager_->touch_manager()->is_button_down(0);
	io.MouseDown[1] = input_manager_->touch_manager()->is_button_down(1);
	io.MouseWheel += input_manager_->touch_manager()->mouse_rel().z() / WHEEL_DELTA;

	gef::KeyboardD3D11* keyboard_d3d11 = (gef::KeyboardD3D11*)input_manager_->keyboard();


	io.KeyShift = keyboard_d3d11->IsKeyDown(gef::Keyboard::KC_LSHIFT) || keyboard_d3d11->IsKeyDown(gef::Keyboard::KC_RSHIFT);
	io.KeyCtrl = keyboard_d3d11->IsKeyDown(gef::Keyboard::KC_LCONTROL) || keyboard_d3d11->IsKeyDown(gef::Keyboard::KC_RCONTROL);
	io.KeyAlt = keyboard_d3d11->IsKeyDown(gef::Keyboard::KC_LALT) || keyboard_d3d11->IsKeyDown(gef::Keyboard::KC_RALT);


	for (int key_code_num = 0; key_code_num < gef::Keyboard::NUM_KEY_CODES; ++key_code_num)
	{
		gef::Keyboard::KeyCode key_code = (gef::Keyboard::KeyCode)key_code_num;
		int scan_code = keyboard_d3d11->GetScanCode(key_code);

		int vk_code = MapVirtualKey(scan_code, MAPVK_VSC_TO_VK_EX);

		io.KeysDown[vk_code] = keyboard_d3d11->IsKeyDown(key_code);
		if (keyboard_d3d11->IsKeyPrintable(key_code) && keyboard_d3d11->IsKeyPressed(key_code))
		{
			if (!io.KeyShift && vk_code >= 'A' && vk_code <= 'Z')
				vk_code = 'a' + (vk_code - 'A');
			io.AddInputCharacter(vk_code);
		}
	}
}