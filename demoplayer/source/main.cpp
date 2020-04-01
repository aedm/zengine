//#define WIN32_LEAN_AND_MEAN
//#define WIN32_EXTRA_LEAN
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <mmsystem.h>
#include <ctime>

#include <GL/glew.h>
#include <GL/GL.h>

#include <zengine.h>
#include <bass/bass.h>
#include <vector>
#include <shellapi.h>
#include "imagerecorder.h"

const std::wstring EngineFolder = L"engine/main/";
const std::wstring ShaderExtension = L".shader";
//#define FULLSCREEN

/// HACK HACK HACK
void hack() {
#define FORCE(a) a force_##a;
  FORCE(Document);
  FORCE(Graph);
  FORCE(Drawable);
  FORCE(Material);
  FORCE(Pass);
  FORCE(StubNode);
  FORCE(FloatsToVec3Node);
  FORCE(FloatsToVec4Node);
  FORCE(StaticMeshNode);
  FORCE(GlobalTimeNode);
  FORCE(SceneTimeNode);
  FORCE(StaticTextureNode);
  FORCE(SceneNode);
  FORCE(MovieNode);
  FORCE(ClipNode);
  FORCE(PropertiesNode);
  FORCE(HalfCubeMeshNode);
}


static PIXELFORMATDESCRIPTOR pfd = {
  sizeof(PIXELFORMATDESCRIPTOR),
  1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
  PFD_TYPE_RGBA, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  32, 0, 0, PFD_MAIN_PLANE, 0, 0, 0, 0, 0
};

bool running = true;
LRESULT CALLBACK gdi01_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  if (uMsg == WM_DESTROY) {
    running = false;
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


void Log(LogMessage Message) {
  OutputDebugStringW(Message.message);
  OutputDebugStringW(L"\n");
}


void LoadEngineShaderFolder(const std::wstring& folder) {
  std::vector<std::wstring> engineStubSourceFiles;
  System::ReadFilesInFolder(folder.c_str(), L"*", engineStubSourceFiles);
  for (std::wstring& fileName : engineStubSourceFiles) {
    if (fileName[fileName.length() - 1] == L'.') continue;
    if (std::wstring(fileName.end() - 7, fileName.end()) == ShaderExtension) {
      std::string stubName = Convert::WstringToString(std::wstring(
        fileName.begin() + EngineFolder.length(),
        fileName.end() - ShaderExtension.length()
      ));
      std::string source(System::ReadFile(fileName.c_str()));
      TheEngineStubs->SetStubSource(stubName, source);;
    }
    else {
      LoadEngineShaderFolder(fileName + L"/");
    }
  }
}


void LoadEngineShaders() {
  std::vector<std::wstring> engineStubSourceFiles;
  LoadEngineShaderFolder(EngineFolder);
  EngineStubs::OnLoadFinished();
}



int CALLBACK WinMain(
  _In_ HINSTANCE hInstance,
  _In_ HINSTANCE hPrevInstance,
  _In_ LPSTR     lpCmdLine,
  _In_ int       nCmdShow
) {
  TheLogger->onLog += Log;

  int argsCount;
  LPWSTR* args = CommandLineToArgvW(GetCommandLineW(), &argsCount);
  const bool recordVideo = argsCount == 2 && wcscmp(args[1], L"--video") == 0;
  const bool windowed = argsCount == 2 && wcscmp(args[1], L"--window") == 0;
  LocalFree(args);

  WNDCLASS wc = { 0, gdi01_WindowProc, 0, 0, hInstance, LoadIcon(nullptr, IDI_APPLICATION),
    LoadCursor(nullptr, IDC_ARROW), HBRUSH(COLOR_WINDOW + 1), nullptr, L"GDI01" };
  RegisterClass(&wc);

  int windowWidth = 1280, windowHeight = 720;
  if (!windowed && !recordVideo) {
    windowWidth = GetSystemMetrics(SM_CXSCREEN);
    windowHeight = GetSystemMetrics(SM_CYSCREEN);
  }

  /// Create window
  // ReSharper disable CppInitializedValueIsAlwaysRewritten
  HWND hwnd = nullptr;
  // ReSharper restore CppInitializedValueIsAlwaysRewritten
  if (windowed || recordVideo) {
    hwnd = CreateWindowEx(0, L"GDI01", L"teszkos demo", WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
      HWND_DESKTOP, nullptr, hInstance, nullptr);
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
  }
  else {
    hwnd = CreateWindowW(L"static", NULL, WS_POPUP | WS_VISIBLE, 0, 0,
      windowWidth, windowHeight, NULL, NULL, NULL, nullptr);
    ShowCursor(FALSE);
  }

  /// Create device context
  const HDC hdc = GetDC(hwnd);  // NOLINT(misc-misplaced-const)
  SetPixelFormat(hdc, ChoosePixelFormat(hdc, &pfd), &pfd);
  wglMakeCurrent(hdc, wglCreateContext(hdc));

  /// Set up image recorder
  const ImageRecorder imageRecorder;

  /// Initialize BASS
  DWORD bassChannel = 0;
  if (!recordVideo) {
    BASS_DEVICEINFO di;
    for (int a = 1; BASS_GetDeviceInfo(a, &di); a++) {
      if (di.flags & BASS_DEVICE_ENABLED) // enabled output device
        INFO("dev %d: %s\n", a, di.name);
    }
    if (!BASS_Init(1, 44100, 0, nullptr, nullptr)) ERR("Can't initialize BASS");
    bassChannel = BASS_StreamCreateFile(FALSE, L"demo.mp3", 0, 0, BASS_STREAM_AUTOFREE);
  }

  /// Initialize Zengine
  InitZengine();
  OpenGL->OnContextSwitch();

  LoadEngineShaders();
  RenderTarget* renderTarget = new RenderTarget(ivec2(windowWidth, windowHeight));

  /// Load precalc project file
  char* json = System::ReadFile(L"loading.zen");
  std::shared_ptr<Document> loading = FromJson(std::string(json));
  ASSERT(loading);
  delete json;

  /// Show loading screen
  loading->mMovie.GetNode()->Draw(renderTarget, 0);
  wglSwapLayerBuffers(hdc, WGL_SWAP_MAIN_PLANE);

  /// Load demo file
  json = System::ReadFile(L"demo.zen");
  std::shared_ptr<Document> doc = FromJson(std::string(json));
  ASSERT(doc);
  delete json;

  /// Compile shaders, upload resources
  std::vector<std::shared_ptr<Node>> nodes;
  doc->GenerateTransitiveClosure(nodes, true);
  for (const auto& node : nodes) node->Update();

  /// No more OpenGL resources should be allocated after this point
  //PleaseNoNewResources = true;

  /// Calculate demo length
  std::shared_ptr<MovieNode> movieNode = doc->mMovie.GetNode();
  const float movieLength = movieNode->CalculateMovieLength();
  const float beatsPerSecond = doc->mProperties.GetNode()->mBPM.Get() / 60.0f;;

  /// Start music
  if (!recordVideo) {
    BASS_ChannelSetPosition(bassChannel, BASS_ChannelSeconds2Bytes(bassChannel, 0), BASS_POS_BYTE);
    BASS_ChannelPlay(bassChannel, FALSE);
  }

  const int videoWidth = int(renderTarget->mSize.x);
  const int videoHeight = int(renderTarget->mSize.y);
  std::vector<unsigned char> pixels(videoWidth * videoHeight * 4);
  std::vector<unsigned char> pixelsFlip(videoWidth * videoHeight * 4);

  /// Play demo
  const DWORD startTime = timeGetTime();
  UINT frameNumber = 0;
  while (running) {
    /// Handle Win32 events
    MSG msg;
    if (PeekMessage(&msg, hwnd, 0, 0, PM_NOREMOVE)) {
      GetMessage(&msg, nullptr, 0, 0);
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    if (!recordVideo && GetAsyncKeyState(VK_ESCAPE)) break;

    /// Measure elapsed time
    float time;
    if (recordVideo) {
      // Record at 60fps
      time = beatsPerSecond * float(frameNumber) / 60.0f;
    }
    else {
      time = beatsPerSecond * float(timeGetTime() - startTime) / 1000.0f;
    }
    if (time > movieLength) break;
    GlobalTimeNode::OnTimeChanged(time);

    /// Render demo frame
    movieNode->Draw(renderTarget, time);
    renderTarget->FinishFrame();

    /// Save rendered image to file
    if (recordVideo) {
      /// Read framebuffer
      glGetTextureImage(renderTarget->mColorTexture->mHandle, 0,
        GL_BGRA, GL_UNSIGNED_BYTE, videoHeight * videoWidth * 4, &pixels[0]);

      /// Flip scanlines
      for (int i = 0; i < videoHeight; i++) {
        memcpy(&pixelsFlip[i*videoWidth * 4],
          &pixels[(videoHeight - 1 - i)*videoWidth * 4], videoWidth * 4);
      }

      imageRecorder.RecordImage(&pixelsFlip[0], videoWidth, videoHeight, frameNumber);
    }

    wglSwapLayerBuffers(hdc, WGL_SWAP_MAIN_PLANE);
    frameNumber++;
  };

  /// Köszön olvasó.
  CloseZengine();
  if (!recordVideo) {
    BASS_Free();
  }

  return 0;
}
