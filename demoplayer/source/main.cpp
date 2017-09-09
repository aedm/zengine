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
  0, // Size Of This Pixel Format Descriptor... BAD coding, nothing new, saves 6 bytes
  1, PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, 32, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0
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

  DWORD chan;
  QWORD pos;
  BASS_DEVICEINFO di;
  for (int a = 1; BASS_GetDeviceInfo(a, &di); a++) {
    if (di.flags&BASS_DEVICE_ENABLED) // enabled output device
      INFO("dev %d: %s\n", a, di.name);
  }

  if (!BASS_Init(1, 44100, 0, 0, NULL)) ERR("Can't initialize BASS");
  chan = BASS_StreamCreateFile(FALSE, L"demo.mp3", 0, 0, BASS_STREAM_AUTOFREE);
  //chan = BASS_StreamCreateFile(FALSE, L"013b.wav", 0, 0, BASS_SAMPLE_LOOP);
  pos = BASS_ChannelGetLength(chan, BASS_POS_BYTE);

  WNDCLASS wc = {0, gdi01_WindowProc, 0, 0, hInstance, LoadIcon(NULL, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_WINDOW + 1), NULL, L"GDI01"};
  RegisterClass(&wc);


#ifdef FULLSCREEN
  int windowWidth = GetSystemMetrics(SM_CXSCREEN);
  int windowHeight = GetSystemMetrics(SM_CYSCREEN);
  //static DEVMODEW dmScreenSettings = {
  //  L"", 0, 0, sizeof(dmScreenSettings), 0, DM_PELSWIDTH | DM_PELSHEIGHT,
  //  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, L"", 0, 0, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  //};
  //dmScreenSettings.dmPelsWidth = windowWidth;
  //dmScreenSettings.dmPelsHeight = windowHeight;
  static DEVMODEW dmScreenSettings = {
    L"", 0, 0, sizeof(dmScreenSettings), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    L"", 0, 0, DWORD(-1), DWORD(-1), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };
  ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
  HWND hwnd = CreateWindowW(L"edit", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0);
  HDC hDC = GetDC(hwnd);
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
#endif 

  SetPixelFormat(hDC, ChoosePixelFormat(hDC, &pfd), &pfd);
  wglMakeCurrent(hDC, wglCreateContext(hDC));

  /// Initialize Zengine
  InitZengine();
  OpenGL->OnContextSwitch();
  OpenGL->Clear(true, true, 0x80008000);
  SwapBuffers(hDC);

  LoadEngineShaders();
  Vec2 windowSize = Vec2(float(windowWidth), float(windowHeight));
  RenderTarget* renderTarget = new RenderTarget(windowSize);

  /// Load demo file
  char* json = System::ReadFile(L"demo.zen");
  Document* doc = FromJSON(string(json));
  ASSERT(doc);
  delete json;

  /// Compile shaders
  vector<Node*> nodes;
  doc->GenerateTransitiveClosure(nodes, false);
  for (Node* node : nodes) node->Update();

  /// Calculate demo length
  MovieNode* movieNode = doc->mMovie.GetNode();
  float movieLength = movieNode->CalculateMovieLength();

  float bps = doc->mProperties.GetNode()->mBPM.Get() / 60.0f;;

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
    float time = bps * float(timeGetTime() - startTime) / 1000.0f;
    if (time > movieLength) break;
    GlobalTimeNode::OnTimeChanged(time);

    /// Render demo frame
    movieNode->Draw(renderTarget, time);
    SwapBuffers(hDC);
  };

  /// Köszön olvasó.
  CloseZengine();
  BASS_Free();
}
