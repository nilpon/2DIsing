#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <cmath>
#include <ctime>
#include <fstream>
#include <vector>
#include <string>
#include <utility>
#include "XoshiroCpp.hpp"

int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;
int IMAGE_WIDTH = 200;
int IMAGE_HEIGHT = 150;
#define SCREEN_SHOT_PATH "Ising.bmp"

const std::vector<std::pair<int, int> > window_size_list = {{320, 240}, {640, 480}, {800, 600}, {1280, 960}, {1440, 1080}, {1600, 1200}, {1920, 1440}, {2560, 1920}};
const std::vector<std::pair<int, int> > image_size_list = {{4, 3}, {20, 15}, {40, 30}, {60, 45}, {120, 80}, {200, 150}, {320, 240}, {400, 300}, {640, 480}, {800, 600}, {1280, 960}, {1440, 1080}, {1600, 1200}, {1920, 1440}, {2560, 1920}};
int window_size_i = 2;
int image_size_i = 5;
bool window_size_changed = false;
bool image_size_changed = false;

unsigned char * image = NULL;
XoshiroCpp::Xoshiro128PlusPlus rng(std::time(nullptr));
int temp = 200;
int speed = 15;
bool is_pause = true;

class UnionFind {
public:
  std::vector<int> parent;
  std::vector<int> rank;
  std::vector<int> relative_weight;
  std::vector<int> component_size;
  
  UnionFind(int n) {
    parent.resize(n);
    rank.resize(n);
    relative_weight.resize(n);
    component_size.resize(n);
    for(int i = 0; i < n; i++) {
      parent[i] = i;
      rank[i] = 0;
      relative_weight[i] = 0;
      component_size[i] = 1;
    }
  }

  int root(int x) {
    if(parent[x] == x) {
      return x;
    }
    else {
      int r = root(parent[x]);
      relative_weight[x] += relative_weight[parent[x]];
      return parent[x] = r;
    }
  }
  
  bool same(int x, int y) {
    return root(x) == root(y);
  }

  // weight(y) - weight(x) = w
  bool merge(int x, int y, int w = 0) {
    w += weight(x) - weight(y);
    x = root(x); y = root(y);
    if(x == y) return false;
    int compsize = component_size[x] + component_size[y];
    if(rank[x] < rank[y]) {
      parent[x] = y;
      relative_weight[x] = -w;
      component_size[y] = compsize;
    }
    else {
      parent[y] = x;
      relative_weight[y] = w;
      component_size[x] = compsize;
      if(rank[x] == rank[y]) rank[x]++;
    }
    return true;
  }

  int get_component_size(int x) {
    return component_size[root(x)];
  }

  int weight(int x) {
    root(x);
    return relative_weight[x];
  }
};

int count_connected_component(int color)
{
  UnionFind uf(IMAGE_HEIGHT * IMAGE_WIDTH);
  for(int i = 0; i < IMAGE_HEIGHT; i++) {
    for(int j = 0; j < IMAGE_WIDTH; j++) {
      if(image[4*i*IMAGE_WIDTH + 4*j    ] == color) {
        int ni, nj;
        ni = i; nj = j+1;
        if(j == IMAGE_WIDTH-1) nj = 0;
        if(image[4*i*IMAGE_WIDTH + 4*j] == image[4*ni*IMAGE_WIDTH + 4*nj]) {
          uf.merge(i*IMAGE_WIDTH + j, ni*IMAGE_WIDTH + nj);
        }
        ni = i+1; nj = j;
        if(i == IMAGE_HEIGHT-1) ni = 0;
        if(image[4*i*IMAGE_WIDTH + 4*j] == image[4*ni*IMAGE_WIDTH + 4*nj]) {
          uf.merge(i*IMAGE_WIDTH + j, ni*IMAGE_WIDTH + nj);
        }
      }
    }
  }

  int ct = 0;
  for(int id = 0; id < IMAGE_HEIGHT * IMAGE_WIDTH; id++) {
    if(uf.root(id) == id && image[4*id] == color) ct++;
  }
  
  return ct;
}

void Screenshot(int x, int y, int w, int h, const char * filename)
{
    unsigned char * pixels = new unsigned char[w*h*4]; // 4 bytes for RGBA
    glReadPixels(x,y,w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    SDL_Surface * surf = SDL_CreateRGBSurfaceFrom(pixels, w, h, 8*4, w*4, 0,0,0,0);
    SDL_SaveBMP(surf, filename);

    SDL_FreeSurface(surf);
    delete [] pixels;
}

bool input()
{
  SDL_Event event;
  
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      return true;
      break;
    case SDL_KEYDOWN:
      switch (event.key.keysym.sym) {
      case SDLK_UP:
        temp++;
        break;
      case SDLK_RIGHT:
        speed++;
        if(speed > 24) speed = 24;
        break;
      case SDLK_DOWN:
        temp--;
        if(temp < 0) temp = 0;
        break;
      case SDLK_LEFT:
        speed--;
        if(speed < 0) speed = 0;
        break;
      case SDLK_s:
        Screenshot(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SCREEN_SHOT_PATH);
        break;
      case SDLK_r:
        for(int i = 0; i < IMAGE_HEIGHT; i++) {
          for(int j = 0; j < IMAGE_WIDTH; j++) {
            if(rng() & 1) {
              image[4*i*IMAGE_WIDTH + 4*j    ] = 0xff;
              image[4*i*IMAGE_WIDTH + 4*j + 1] = 0xff;
              image[4*i*IMAGE_WIDTH + 4*j + 2] = 0xff;
              image[4*i*IMAGE_WIDTH + 4*j + 3] = 0x00;
            }
            else {
              image[4*i*IMAGE_WIDTH + 4*j    ] = 0x00;
              image[4*i*IMAGE_WIDTH + 4*j + 1] = 0x00;
              image[4*i*IMAGE_WIDTH + 4*j + 2] = 0x00;
              image[4*i*IMAGE_WIDTH + 4*j + 3] = 0x00;
            }
          }
        }
        break;
      case SDLK_p:
        if(is_pause) is_pause = false;
        else is_pause = true;
        break;
      case SDLK_z:
        if(is_pause && image_size_i) {
          image_size_i--;
          if(image_size_i < 0) image_size_i = 0;
          image_size_changed = true;
        }
        break;
      case SDLK_x:
        if(is_pause && image_size_i < image_size_list.size() - 1) {
          image_size_i++;
          if(image_size_i >= image_size_list.size()) image_size_i = image_size_list.size() - 1;
          image_size_changed = true;
        }
        break;
      case SDLK_q:
        if(is_pause) {
          window_size_i--;
          if(window_size_i < 0) window_size_i = 0;
          window_size_changed = true;
        }
        break;
      case SDLK_w:
        if(is_pause) {
          window_size_i++;
          if(window_size_i >= window_size_list.size()) window_size_i = window_size_list.size() - 1;
          window_size_changed = true;
        }
        break;
      default:
        break;
      }
      break;
    default:
      break;
    }
  }
  return false;
}

std::uint64_t calc_pval(int temp)
{
  double p = std::pow(2, 64 - (double)10000.0/temp);
  if(p >= std::pow((double)2.0, 64) - 1) p = 0xffffffffffffffff;
  return p;
}

bool random_raise(int energy_jump, std::uint64_t pv)
{
  if(energy_jump > 0) {
    for(int i = 0; i < energy_jump; i++) {
      if(rng() >= pv) return false;
    }
    return true;
  }
  return true;
}

void draw() {
  glClear(GL_COLOR_BUFFER_BIT);
  if(!is_pause){
    std::uint64_t pv = calc_pval(temp);
    for(int k = 0; k < (1 << speed); k++) {
      int j = rng() % IMAGE_WIDTH, i = rng() % IMAGE_HEIGHT;
      int energy_jump = 0;
      auto ptstate = image[4*i*IMAGE_WIDTH + 4*j];
      
      int adji = ((i > 0) ? i-1 : IMAGE_HEIGHT-1), adjj = j;
      auto adjstate = image[4*adji*IMAGE_WIDTH + 4*adjj];
      if(ptstate == adjstate) energy_jump++;
      else energy_jump--;
      
      adji = ((i < IMAGE_HEIGHT-1) ? i+1 : 0); adjj = j;
      adjstate = image[4*adji*IMAGE_WIDTH + 4*adjj];
      if(ptstate == adjstate) energy_jump++;
      else energy_jump--;
      
      adji = i; adjj = ((j > 0) ? j-1 : IMAGE_WIDTH-1);
      adjstate = image[4*adji*IMAGE_WIDTH + 4*adjj];
      if(ptstate == adjstate) energy_jump++;
      else energy_jump--;
      
      adji = i; adjj = ((j < IMAGE_WIDTH-1) ? j+1 : 0);
      adjstate = image[4*adji*IMAGE_WIDTH + 4*adjj];
      if(ptstate == adjstate) energy_jump++;
      else energy_jump--;

      energy_jump /= 2;
      
      if(random_raise(energy_jump, pv)) {
        image[4*i*IMAGE_WIDTH + 4*j   ] ^= 0xff;
        image[4*i*IMAGE_WIDTH + 4*j + 1] ^= 0xff;
        image[4*i*IMAGE_WIDTH + 4*j + 2] ^= 0xff;
      }
    }
  }
  
  glDrawPixels(IMAGE_WIDTH, IMAGE_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, image);
  glFlush();
}

#ifdef __cplusplus
extern "C"
#endif

int main(int argc, char *argv[])
{
  image = new unsigned char[IMAGE_WIDTH * IMAGE_HEIGHT * 4];
  if(!image) return false;
  for(int i = 0; i < IMAGE_HEIGHT; i++) {
    for(int j = 0; j < IMAGE_WIDTH; j++) {
      if(rng() & 1) {
        image[4*i*IMAGE_WIDTH + 4*j    ] = 0xff;
        image[4*i*IMAGE_WIDTH + 4*j + 1] = 0xff;
        image[4*i*IMAGE_WIDTH + 4*j + 2] = 0xff;
        image[4*i*IMAGE_WIDTH + 4*j + 3] = 0x00;
      }
      else {
        image[4*i*IMAGE_WIDTH + 4*j    ] = 0x00;
        image[4*i*IMAGE_WIDTH + 4*j + 1] = 0x00;
        image[4*i*IMAGE_WIDTH + 4*j + 2] = 0x00;
        image[4*i*IMAGE_WIDTH + 4*j + 3] = 0x00;
      }
    }
  }
  int n_component_black = count_connected_component(0x00);
  int n_component_white = count_connected_component(0xff);
  int frame_count = 0;
  
  SDL_Init(SDL_INIT_VIDEO);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  SDL_Window* window = SDL_CreateWindow("",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              WINDOW_WIDTH, WINDOW_HEIGHT,
                              SDL_WINDOW_OPENGL);

  SDL_GLContext context = SDL_GL_CreateContext(window);
  if (!context) return false;

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glOrtho(0.0, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0, -1000.0, 1000.0);

  glClear(GL_COLOR_BUFFER_BIT);
  glDrawPixels(IMAGE_WIDTH, IMAGE_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, image);
  
  SDL_GL_SwapWindow(window);

  while (true) {
    if (input())
      break;
    if(window_size_changed) {
      WINDOW_WIDTH = window_size_list[window_size_i].first;
      WINDOW_HEIGHT = window_size_list[window_size_i].second;
      SDL_SetWindowSize(window, WINDOW_WIDTH, WINDOW_HEIGHT);
      window_size_changed = false;
    }
    if(image_size_changed) {
      IMAGE_WIDTH = image_size_list[image_size_i].first;
      IMAGE_HEIGHT = image_size_list[image_size_i].second;
      if(image) delete [] image;
      image = new unsigned char[IMAGE_WIDTH * IMAGE_HEIGHT * 4];
      if(!image) return false;
      for(int i = 0; i < IMAGE_HEIGHT; i++) {
        for(int j = 0; j < IMAGE_WIDTH; j++) {
          if(rng() & 1) {
            image[4*i*IMAGE_WIDTH + 4*j    ] = 0xff;
            image[4*i*IMAGE_WIDTH + 4*j + 1] = 0xff;
            image[4*i*IMAGE_WIDTH + 4*j + 2] = 0xff;
            image[4*i*IMAGE_WIDTH + 4*j + 3] = 0x00;
          }
          else {
            image[4*i*IMAGE_WIDTH + 4*j    ] = 0x00;
            image[4*i*IMAGE_WIDTH + 4*j + 1] = 0x00;
            image[4*i*IMAGE_WIDTH + 4*j + 2] = 0x00;
            image[4*i*IMAGE_WIDTH + 4*j + 3] = 0x00;
          }
        }
      }
      n_component_black = count_connected_component(0x00);
      n_component_white = count_connected_component(0xff);
      
      image_size_changed = false;
    }

    glPixelZoom((float)WINDOW_WIDTH/IMAGE_WIDTH, (float)WINDOW_HEIGHT/IMAGE_HEIGHT);
    draw();
    std::string titlebar = "2D Ising " + std::to_string(IMAGE_WIDTH) + "x" + std::to_string(IMAGE_HEIGHT);
    if(is_pause) titlebar += " *PAUSE*";
    titlebar += " T=" + std::to_string(temp) + ", speed=" + std::to_string(speed) + " (pv=" + std::to_string(calc_pval(temp)) + ")";
    if(!is_pause && (frame_count % 10) == 0) {
      n_component_black = count_connected_component(0x00);
      n_component_white = count_connected_component(0xff);
    }
    titlebar += " [" + std::to_string(n_component_black) + ", " + std::to_string(n_component_white) + "]";
    SDL_SetWindowTitle(window, titlebar.c_str());
    
    SDL_GL_SwapWindow(window);

    frame_count++;
    SDL_Delay(16);
  }
  
  SDL_Quit();
  delete [] image;
  return EXIT_SUCCESS;
}
