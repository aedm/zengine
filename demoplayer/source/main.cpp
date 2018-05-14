//#define WIN32_LEAN_AND_MEAN
//#define WIN32_EXTRA_LEAN
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <Mmsystem.h>
#include <time.h>

#define GLEW_STATIC
#include <glew/glew.h>
#include <GL/gl.h>

#include <zengine.h>
#include <bass/bass.h>
#include <vector>
#include <shellapi.h>
#include "imagerecorder.h"

using namespace std;

const wstring EngineFolder = L"engine/main/";
const wstring ShaderExtension = L".shader";
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
  FORCE(TextureNode);
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
  32, 0, 0, PFD_MAIN_PLANE, 0, 0, 0, 0
};

bool running = true;
LRESULT CALLBACK gdi01_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
  case WM_DESTROY:
    running = false;
    break;
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


void Log(LogMessage Message) {
  OutputDebugStringW(Message.message);
  OutputDebugStringW(L"\n");
}


void LoadEngineShaderFolder(const wstring& folder) {
  vector<wstring> engineStubSourceFiles;
  System::ReadFilesInFolder(folder.c_str(), L"*", engineStubSourceFiles);
  for (wstring& fileName : engineStubSourceFiles) {
    if (fileName[fileName.length() - 1] == L'.') continue;
    if (wstring(fileName.end() - 7, fileName.end()) == ShaderExtension) {
      string stubName(fileName.begin() + EngineFolder.length(),
        fileName.end() - ShaderExtension.length());
      string source(System::ReadFile(fileName.c_str()));
      TheEngineStubs->SetStubSource(stubName, source);;
    }
    else {
      LoadEngineShaderFolder((fileName + L"/").c_str());
    }
  }
}


void LoadEngineShaders() {
  vector<wstring> engineStubSourceFiles;
  LoadEngineShaderFolder(EngineFolder);
  TheEngineStubs->OnLoadFinished();
}



int CALLBACK WinMain(
  _In_ HINSTANCE hInstance,
  _In_ HINSTANCE hPrevInstance,
  _In_ LPSTR     lpCmdLine,
  _In_ int       nCmdShow
) {
  TheLogger->onLog += Log;

  /// Check the command line
  LPWSTR *args;
  int argsCount;
  args = CommandLineToArgvW(GetCommandLineW(), &argsCount);
  bool recordVideo = argsCount == 2 && wcscmp(args[1], L"--video") == 0;
  bool windowed = argsCount == 2 && wcscmp(args[1], L"--window") == 0;
  LocalFree(args);

  WNDCLASS wc = { 0, gdi01_WindowProc, 0, 0, hInstance, LoadIcon(NULL, IDI_APPLICATION),
    LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_WINDOW + 1), NULL, L"GDI01" };
  RegisterClass(&wc);

  int windowWidth = 1280, windowHeight = 720;
  if (!windowed && !recordVideo) {
    windowWidth = GetSystemMetrics(SM_CXSCREEN);
    windowHeight = GetSystemMetrics(SM_CYSCREEN);
  }

  /// Create window
  HWND hwnd = 0;
  if (windowed || recordVideo) {
    hwnd = CreateWindowEx(0, L"GDI01", L"teszkos demo", WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
      HWND_DESKTOP, NULL, hInstance, NULL);
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
  }
  else {
    hwnd = CreateWindowW(L"static", NULL, WS_POPUP | WS_VISIBLE, 0, 0,
      windowWidth, windowHeight, NULL, NULL, NULL, 0);
    ShowCursor(FALSE);
  }

  /// Create device context
  HDC hDC = GetDC(hwnd);
  SetPixelFormat(hDC, ChoosePixelFormat(hDC, &pfd), &pfd);
  wglMakeCurrent(hDC, wglCreateContext(hDC));

  /// Set up image recorder
  ImageRecorder imageRecorder;

  /// Initialize BASS
  DWORD bassChannel = 0;
  if (!recordVideo) {
    QWORD pos;
    BASS_DEVICEINFO di;
    for (int a = 1; BASS_GetDeviceInfo(a, &di); a++) {
      if (di.flags & BASS_DEVICE_ENABLED) // enabled output device
        INFO("dev %d: %s\n", a, di.name);
    }
    if (!BASS_Init(1, 44100, 0, 0, NULL)) ERR("Can't initialize BASS");
    bassChannel = BASS_StreamCreateFile(FALSE, L"demo.mp3", 0, 0, BASS_STREAM_AUTOFREE);
    pos = BASS_ChannelGetLength(bassChannel, BASS_POS_BYTE);
  }

  /// Initialize Zengine
  InitZengine();
  OpenGL->OnContextSwitch();

  LoadEngineShaders();
  Vec2 windowSize = Vec2(float(windowWidth), float(windowHeight));
  RenderTarget* renderTarget = new RenderTarget(windowSize, recordVideo);

  /// Load precalc project file
  char* json = System::ReadFile(L"loading.zen");
  shared_ptr<Document> loading = FromJSON(string(json));
  ASSERT(loading);
  delete json;

  /// Show loading screen
  loading->mMovie.GetNode()->Draw(renderTarget, 0);
  wglSwapLayerBuffers(hDC, WGL_SWAP_MAIN_PLANE);

  /// Load demo file
  json = System::ReadFile(L"demo.zen");
  shared_ptr<Document> doc = FromJSON(string(json));
  ASSERT(doc);
  delete json;

  /// Compile shaders, upload resources
  vector<shared_ptr<Node>> nodes;
  doc->GenerateTransitiveClosure(nodes, true);
  for (const auto& node : nodes) node->Update();

  /// No more OpenGL resources should be allocated after this point
  //PleaseNoNewResources = true;

  /// Calculate demo length
  shared_ptr<MovieNode> movieNode = doc->mMovie.GetNode();
  float movieLength = movieNode->CalculateMovieLength();
  float beatsPerSecond = doc->mProperties.GetNode()->mBPM.Get() / 60.0f;;

  /// Start music
  if (!recordVideo) {
    BASS_ChannelSetPosition(bassChannel, BASS_ChannelSeconds2Bytes(bassChannel, 0), BASS_POS_BYTE);
    BASS_ChannelPlay(bassChannel, FALSE);
  }

  int videoWidth = int(renderTarget->mSize.x);
  int videoHeight = int(renderTarget->mSize.y);
  vector<unsigned char> pixels(videoWidth * videoHeight * 4);
  vector<unsigned char> pixelsFlip(videoWidth * videoHeight * 4);

  /// Play demo
  DWORD startTime = timeGetTime();
  UINT frameNumber = 0;
  while (running) {
    /// Handle Win32 events
    MSG msg;
    if (PeekMessage(&msg, hwnd, 0, 0, PM_NOREMOVE)) {
      GetMessage(&msg, 0, 0, 0);
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    if (!recordVideo && GetAsyncKeyState(VK_ESCAPE)) break;

    /// Measure elapsed time
    float time;
    if (recordVideo) {
      // Record at 60fps
      time = float(frameNumber) / 60.0f;
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

    wglSwapLayerBuffers(hDC, WGL_SWAP_MAIN_PLANE);
    frameNumber++;
  };

  /// Köszön olvasó.
  CloseZengine();
  if (!recordVideo) {
    BASS_Free();
  }

  return 0;
}
