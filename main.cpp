// main.cpp - single-file minimal app
// Build: see instructions below
#include <SDL3/SDL.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#define S(x) ([]() { x; }())
#define LOG(x) std::cerr << x << "\n"
using u32 = uint32_t;

// tiny helpers
static bgfx::Memory *loadFile(const char *path) {
  std::ifstream ifs(path, std::ios::binary | std::ios::ate);
  if (!ifs)
    return nullptr;
  auto sz = (size_t)ifs.tellg();
  ifs.seekg(0);
  auto mem = (bgfx::Memory *)BGFX_ALLOC(sizeof(bgfx::Memory) + sz + 1);
  // we'll use bgfx::copy normally, but keep minimal: read then create copy mem
  std::vector<uint8_t> buf(sz);
  ifs.read((char *)buf.data(), sz);
  auto m = bgfx::alloc((uint32_t)sz + 1);
  memcpy(m->data, buf.data(), sz);
  m->data[sz] = 0;
  return m;
}

// simple vertex
struct Vert {
  float x, y, z;
  float nx, ny, nz;
};

int main(int argc, char **argv) {
  const char *filename = "t.fbx";
  S(LOG("Starting minimal viewer"););

  // SDL init
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
    LOG("SDL_Init failed: " << SDL_GetError());
    return 1;
  }

  int ww = 1280, wh = 720;
  SDL_Window *win = SDL_CreateWindow(
      "fbx-bgfx-min", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ww, wh,
      SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
  if (!win) {
    LOG("CreateWindow failed");
    return 1;
  }

  // get native handle for bgfx
  SDL_SysWMinfo wminfo;
  SDL_VERSION(&wminfo.version);
  if (!SDL_GetWindowWMInfo(win, &wminfo)) {
    LOG("SDL_GetWindowWMInfo failed");
    return 1;
  }
  bgfx::PlatformData pd{};
#if defined(_WIN32)
  pd.nwh = wminfo.info.win.window;
#elif defined(__linux__)
// X11 or Wayland - try X11 first
#if defined(SDL_VIDEO_DRIVER_X11)
  pd.nwh = (void *)(wminfo.info.x11.window);
  pd.context = nullptr;
  pd.backBuffer = nullptr;
#else
  pd.nwh = (void *)(wminfo.info.wl.surface);
#endif
#else
  pd.nwh = wminfo.info.win.window;
#endif

  // bgfx init
  bgfx::Init init;
  init.platformData = pd;
  init.type = bgfx::RendererType::Count; // auto
  init.resolution.width = ww;
  init.resolution.height = wh;
  init.resolution.reset = BGFX_RESET_VSYNC; // **enable vsync**
  if (!bgfx::init(init)) {
    LOG("bgfx::init failed");
    return 1;
  }

  bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x808080ff, 1.0f,
                     0);
  bgfx::setViewRect(0, 0, 0, ww, wh);

  // simple shaders - user must precompile them into shaders/vs_simple.bin and
  // fs_simple.bin
  auto vsMem = loadFile("shaders/vs_simple.bin");
  auto fsMem = loadFile("shaders/fs_simple.bin");
  if (!vsMem || !fsMem) {
    LOG("Missing compiled shaders in shaders/*.bin - build shaders (see "
        "comments)");
    return 1;
  }
  bgfx::ShaderHandle vsh = bgfx::createShader(vsMem);
  bgfx::ShaderHandle fsh = bgfx::createShader(fsMem);
  bgfx::ProgramHandle program = bgfx::createProgram(vsh, fsh, true);

  // vertex layout
  bgfx::VertexLayout layout;
  layout.begin()
      .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
      .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
      .end();

  // load model with Assimp
  Assimp::Importer importer;
  const aiScene *scene =
      importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_GenNormals |
                                      aiProcess_JoinIdenticalVertices);
  if (!scene) {
    LOG("Assimp load failed: " << importer.GetErrorString());
    return 1;
  }

  struct MeshData {
    bgfx::VertexBufferHandle vb;
    bgfx::IndexBufferHandle ib;
    u32 idxCount;
  };
  std::vector<MeshData> meshes;
  meshes.reserve(scene->mNumMeshes);

  for (unsigned m = 0; m < scene->mNumMeshes; ++m) {
    aiMesh *mesh = scene->mMeshes[m];
    std::vector<Vert> verts;
    verts.reserve(mesh->mNumVertices);
    for (unsigned i = 0; i < mesh->mNumVertices; ++i) {
      aiVector3D p = mesh->mVertices[i];
      aiVector3D n =
          mesh->HasNormals() ? mesh->mNormals[i] : aiVector3D(0, 1, 0);
      verts.push_back({p.x, p.y, p.z, n.x, n.y, n.z});
    }
    std::vector<uint32_t> idx;
    idx.reserve(mesh->mNumFaces * 3);
    for (unsigned f = 0; f < mesh->mNumFaces; ++f) {
      aiFace &face = mesh->mFaces[f];
      if (face.mNumIndices == 3) {
        idx.push_back(face.mIndices[0]);
        idx.push_back(face.mIndices[1]);
        idx.push_back(face.mIndices[2]);
      }
    }
    bgfx::VertexBufferHandle vb = bgfx::createVertexBuffer(
        bgfx::makeRef(verts.data(), (uint32_t)verts.size() * sizeof(Vert)),
        layout);
    bgfx::IndexBufferHandle ib = bgfx::createIndexBuffer(
        bgfx::makeRef(idx.data(), (uint32_t)idx.size() * sizeof(uint32_t)));
    meshes.push_back({vb, ib, (u32)idx.size()});
  }

  // camera state - camera at (cx,cy,cz), look toward -Z, up +Y
  float cx = 0.f, cy = 0.f, cz = 3.0f; // cz controls zoom (distance)
  float yaw = 0.f, pitch = 0.f;
  const float moveSpeed = 2.0f; // units/sec
  const float zoomSpeed = 2.0f;

  bool running = true;
  uint32_t last = SDL_GetTicks();
  while (running) {
    // time
    uint32_t now = SDL_GetTicks();
    float dt = (now - last) / 1000.0f;
    last = now;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_EVENT_QUIT)
        running = false;
      if (e.type == SDL_EVENT_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
        running = false;
    }

    const Uint8 *state = SDL_GetKeyboardState(NULL);
    // WASD: move XY in camera plane
    float dx = 0, dy = 0, dz = 0;
    if (state[SDL_SCANCODE_W])
      dy += moveSpeed * dt;
    if (state[SDL_SCANCODE_S])
      dy -= moveSpeed * dt;
    if (state[SDL_SCANCODE_A])
      dx -= moveSpeed * dt;
    if (state[SDL_SCANCODE_D])
      dx += moveSpeed * dt;
    if (state[SDL_SCANCODE_Q])
      cz -= zoomSpeed * dt; // zoom in
    if (state[SDL_SCANCODE_E])
      cz += zoomSpeed * dt; // zoom out

    // apply camera XY movement in world coords (simple)
    cx += dx;
    cy += dy;
    // clamp zoom
    if (cz < 0.1f)
      cz = 0.1f;

    // handle window resize
    int neww, newh;
    SDL_GetWindowSize(win, &neww, &newh);
    if (neww != ww || newh != wh) {
      ww = neww;
      wh = newh;
      bgfx::reset(ww, wh, BGFX_RESET_VSYNC);
      bgfx::setViewRect(0, 0, 0, ww, wh);
    }

    // view/proj
    float view[16];
    float proj[16];
    // look from (cx,cy,cz) to (cx,cy,0)
    float at[3] = {cx, cy, 0.f};
    float eye[3] = {cx, cy, cz};
    float up[3] = {0.f, 1.f, 0.f};
    bx::mtxLookAt(view, eye, at, up);
    bx::mtxProj(proj, 60.0f, float(ww) / float(wh), 0.1f, 1000.0f,
                bgfx::getCaps()->homogeneousDepth);

    bgfx::setViewTransform(0, view, proj);

    bgfx::touch(0); // clear/view

    // submit meshes
    for (auto &msh : meshes) {
      float mtx[16];
      bx::mtxIdentity(mtx);
      // translate by 0,0,0 (model space as loaded)
      bgfx::setTransform(mtx);
      bgfx::setVertexBuffer(0, msh.vb);
      bgfx::setIndexBuffer(msh.ib);
      bgfx::setState(BGFX_STATE_DEFAULT);
      bgfx::submit(0, program);
    }

    bgfx::frame(); // let bgfx render - no fps lock (vsync on)
  }

  // cleanup
  for (auto &m : meshes) {
    bgfx::destroy(m.vb);
    bgfx::destroy(m.ib);
  }
  bgfx::destroy(program);
  bgfx::shutdown();

  SDL_DestroyWindow(win);
  SDL_Quit();
  return 0;
}
