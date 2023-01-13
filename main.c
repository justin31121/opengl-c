#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL.h>

#define OPEN_RENDERER_IMPLEMENTATION
#include "../datastructures-c/libs/open_renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "./stb_image.h"

#define WIDTH 800
#define HEIGHT 600

#define VERTICIES_CAP 1024

typedef struct{
  Vec2f position;
  Vec4f color;
  Vec2f uv;
}Vertex;

int main() {
  if(SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "SDL_Error: %s\n", SDL_GetError());
    exit(1);
  }

  SDL_Window *window = SDL_CreateWindow("Rainbow Triangle", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT,
					SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  if(!window) {
    fprintf(stderr, "SDL_Error: %s\n", SDL_GetError());
    exit(1);    
  }
  
  {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    int major;
    int minor;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
    printf("GL version %d.%d\n", major, minor);
  }

  if(SDL_GL_CreateContext(window) == NULL) {
    fprintf(stderr, "SDL_Error: %s\n", SDL_GetError());
    return 1;
  }

  //BETTER INIT
  Open_Renderer or;
  if(!open_renderer_init(&or, "./simple.vert", "./simple_color.frag", "./simple_image.frag")) {
    panic("open_renderer_init");
  }

  //MONKAS
  int x, y, comp;
  char *data = (char *) stbi_load("./monka.png", &x, &y, &comp, 0);
  if(!data) {
    panic("Failed to load monka.png");
  }
  
  GLuint texture = 0;
  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(
	       GL_TEXTURE_2D,
	       0,
	       GL_RGBA,
	       x,
	       y,
	       0,
	       GL_RGBA,
	       GL_UNSIGNED_INT_8_8_8_8_REV,
	       data);

  SDL_Event event;
  bool quit = false;
  
  while(!quit) {
    SDL_PollEvent(&event);

    switch(event.type) {
    case SDL_QUIT: {
      quit = true;
      break;
    }
    case SDL_KEYDOWN: {
      switch(event.key.keysym.sym) {
      case SDLK_q:
	quit = true;
	break;
      }
      break;
    }
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    {
      int w, h;
      SDL_GetWindowSize(window, &w, &h);
      
      or.resolution = vec2i(w, h);
      open_renderer_set_shader(&or, SHADER_FOR_IMAGE);     

      Vec2f uvp = vec2fs(0);
      Vec2f uvs = vec2f(1, 1);
      open_renderer_image_rect(&or, vec2f(0, 0), vec2f(WIDTH, HEIGHT), uvp, uvs);

      open_renderer_flush(&or);
    }

    //END RENDER
    SDL_GL_SwapWindow(window);
  }

  stbi_image_free(data);
  SDL_DestroyWindow(window);
  SDL_Quit();
  
  return 0;
}
