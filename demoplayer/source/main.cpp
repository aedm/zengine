#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <Mmsystem.h>
#include <time.h>
#include <GL/gl.h>
#include <zengine.h>
#include <bass/bass.h>
#include <vector>

using namespace std;

const wstring EngineFolder = L"engine/main/";
const wstring ShaderExtension = L".shader";
#define FULLSCREEN

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
  FORCE(StaticMeshNode);
  FORCE(GlobalTimeNode);
  FORCE(SceneTimeNode);
  FORCE(TextureNode);
  FORCE(SceneNode);
  FORCE(MovieNode);
  FORCE(ClipNode);
  FORCE(PropertiesNode);
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
    } else {
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

  WNDCLASS wc = { 0, gdi01_WindowProc, 0, 0, hInstance, LoadIcon(NULL, IDI_APPLICATION), 
    LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_WINDOW + 1), NULL, L"GDI01" };
  RegisterClass(&wc);

#ifdef FULLSCREEN
  int windowWidth = GetSystemMetrics(SM_CXSCREEN);
  int windowHeight = GetSystemMetrics(SM_CYSCREEN);
  //static DEVMODEW dmScreenSettings = {
  //  L"", 0, 0, sizeof(dmScreenSettings), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  //  L"", 0, 0, DWORD(-1), DWORD(-1), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  //};
  //ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
  //HWND hwnd = CreateWindowW(L"edit", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 
  //  0, 0, 0, 0, 0, 0, 0, 0);
  //HDC hDC = GetDC(hwnd);
  //ShowCursor(FALSE);
  //UpdateWindow(hwnd);

  auto hwnd = CreateWindowW(L"static", NULL, WS_POPUP | WS_VISIBLE, 0, 0, 
    windowWidth, windowHeight, NULL, NULL, NULL, 0);
  auto hDC = GetDC(hwnd);
  SetPixelFormat(hDC, ChoosePixelFormat(hDC, &pfd), &pfd);
  wglMakeCurrent(hDC, wglCreateContext(hDC));
  ShowCursor(FALSE);
#else 
  int windowWidth = 1280;
  int windowHeight = 720;

  HWND hwnd = CreateWindowEx(
    0,
    L"GDI01",
    L"teszkos demo",
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
    HWND_DESKTOP, NULL, hInstance, NULL);
  HDC hDC = GetDC(hwnd);
  ShowWindow(hwnd, nCmdShow);
  UpdateWindow(hwnd);

  SetPixelFormat(hDC, ChoosePixelFormat(hDC, &pfd), &pfd);
  wglMakeCurrent(hDC, wglCreateContext(hDC));
#endif 

  DWORD chan;
  QWORD pos;
  BASS_DEVICEINFO di;
  for (int a = 1; BASS_GetDeviceInfo(a, &di); a++) {
    if (di.flags&BASS_DEVICE_ENABLED) // enabled output device
      INFO("dev %d: %s\n", a, di.name);
  }

  if (!BASS_Init(1, 44100, 0, 0, NULL)) ERR("Can't initialize BASS");
  chan = BASS_StreamCreateFile(FALSE, L"demo.mp3", 0, 0, BASS_STREAM_AUTOFREE);
  pos = BASS_ChannelGetLength(chan, BASS_POS_BYTE);

  /// Initialize Zengine
  InitZengine();
  OpenGL->OnContextSwitch();

  LoadEngineShaders();
  Vec2 windowSize = Vec2(float(windowWidth), float(windowHeight));
  RenderTarget* renderTarget = new RenderTarget(windowSize);

  /// Load precalc project file
  char* json = System::ReadFile(L"loading.zen");
  shared_ptr<Document> loading = FromJSON(string(json));
  ASSERT(loading);
  delete json;

  /// Show loading screen
  for (int i = 0; i < 10; i++) {
    loading->mMovie.GetNode()->Draw(renderTarget, 0);
    wglSwapLayerBuffers(hDC, WGL_SWAP_MAIN_PLANE);
  }
  Sleep(3000);
  //glClearColor(0, 1, 0, 1);
  //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //wglSwapLayerBuffers(hDC, WGL_SWAP_MAIN_PLANE);

  /// Load demo file
  json = System::ReadFile(L"demo.zen");
  shared_ptr<Document> doc = FromJSON(string(json));
  ASSERT(doc);
  delete json;

  /// Compile shaders, upload resources
  vector<shared_ptr<Node>> nodes;
  doc->GenerateTransitiveClosure(nodes, false);
  for (const auto& node : nodes) node->Update();

  /// No more OpenGL resources should be allocated after this point
  PleaseNoNewResources = true;

  /// Calculate demo length
  shared_ptr<MovieNode> movieNode = doc->mMovie.GetNode();
  float movieLength = movieNode->CalculateMovieLength();
  float beatsPerSecond = doc->mProperties.GetNode()->mBPM.Get() / 60.0f;;

  /// Start music
  BASS_ChannelSetPosition(chan, BASS_ChannelSeconds2Bytes(chan, 0), BASS_POS_BYTE);
  BASS_ChannelPlay(chan, FALSE);

  /// Play demo
  DWORD startTime = timeGetTime();
  while (running) {
    /// Handle Win32 events
    MSG msg;
    if (PeekMessage(&msg, hwnd, 0, 0, PM_NOREMOVE)) {
      GetMessage(&msg, 0, 0, 0);
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    if (GetAsyncKeyState(VK_ESCAPE)) break;

    /// Measure elapsed time
    float time = beatsPerSecond * float(timeGetTime() - startTime) / 1000.0f;
    if (time > movieLength) break;
    GlobalTimeNode::OnTimeChanged(time);

    /// Render demo frame
    movieNode->Draw(renderTarget, time);
    wglSwapLayerBuffers(hDC, WGL_SWAP_MAIN_PLANE);
  };

  /// Köszön olvasó.
  CloseZengine();
  BASS_Free();
}
