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
  if(!open_renderer_init(&or)) {
    panic("open_renderer_init");
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
  
  //MONKAS
  int x, y;
  char *data = (char *) stbi_load("./monka2.jpg", &x, &y, NULL, 4);
  if(!data) {
    panic("Failed to load monka2.jpg");
  }

  printf("Loaded monka2.jpg (%d, %d)\n", x, y);

  
  int x2, y2;
  char *data2 = (char *) stbi_load("./poggers.png", &x2, &y2, NULL, 4);
  if(!data2) {
    panic("Failed to load poggers.png");
  }

  printf("Loaded poggers.png (%d, %d)\n", x2, y2);

  

  /*
  for(int i=0;i<x;i++) {
    for(int j=0;j<y;j++) {
      uint32_t pixel = data[j*y+i];
      uint8_t r = (pixel & 0xff) >> 6;
      uint8_t g = (pixel & 0x00ff) >> 4;
      uint8_t b = (pixel & 0x0000ff) >> 2;
      uint8_t a = pixel & 0x000000ff;

      //printf("%u %u %u %u\n", r, g, b, a);
    }
  }
  */
  
  glTexImage2D(
	       GL_TEXTURE_2D,
	       0,
	       GL_RGBA,
	       x + x2,
	       (y > y2 ? y : y2),
	       0,
	       GL_RGBA,
	       GL_UNSIGNED_INT_8_8_8_8_REV,
	       NULL);
  
  glTexSubImage2D(GL_TEXTURE_2D,
	0,
	0,
	0,
	x,
	y,
	GL_RGBA,
	GL_UNSIGNED_INT_8_8_8_8_REV,
	data);

  glTexSubImage2D(GL_TEXTURE_2D,
	0,
	x,
	0,
	x2,
	y2,
	GL_RGBA,
	GL_UNSIGNED_INT_8_8_8_8_REV,
	data2);

  stbi_image_free(data);
  stbi_image_free(data2);

  SDL_Event event;
  bool quit = false;

  Vec2f pos = vec2f(0.f, 0.f);
  Vec2f vel = vec2f(200, 100);
  Vec2f size = vec2fs(150); 
  
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

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable( GL_BLEND );

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    {
      int w, h;
      SDL_GetWindowSize(window, &w, &h);
      
      or.resolution = vec2i(w, h);
      float now = (float) SDL_GetTicks() / 1000.0f;
      float dt = now - or.time;
      or.time = now;

      pos = vec2f_add(pos, vec2f_mul(vel, vec2fs(dt)));

      if(pos.x + size.x >= (float) w || pos.x < 0.f) {
	vel.x *= -1;
      }
      if(pos.y + size.y>= (float) h || pos.y < 0.f) {
	vel.y *= -1;
      }

      // -- IMAGE
      open_renderer_set_shader(&or, SHADER_FOR_IMAGE);
      open_renderer_image_rect(&or, vec2fs(0), vec2f(w, h), vec2fs(0), vec2f((float) x / (float) (x+x2), (float) 1));
      open_renderer_image_rect(&or, pos, size, vec2f((float) x / (float) (x+x2), 0),vec2f((float) x2 / (float) (x+x2), (float) y2 / (float) y));
      open_renderer_flush(&or);

      // -- COLOR
      open_renderer_set_shader(&or, SHADER_FOR_COLOR);      
      open_renderer_solid_rect(&or, vec2f(100, 100), vec2f(100, 100), vec4f(1.0, 0.0, 0.0, 1.0));
      open_renderer_flush(&or);
    }

    //END RENDER
    SDL_GL_SwapWindow(window);
  }

  SDL_DestroyWindow(window);
  SDL_Quit();
  
  return 0;
}
