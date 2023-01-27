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

Open_Texture load_texture_from_file(const char *file_path) {
  Open_Texture result = {0};
  result.data = (char *) stbi_load(file_path, &result.width, &result.height, NULL, 4);
  if(!result.data) {
    panic("Failed to load image");
  }
  return result;
}

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
  
  //MONKAS
  Open_Texture monka_texture = load_texture_from_file("monka2.jpg");
  size_t monka = open_renderer_push_image(&or, monka_texture);
  
  //POGGERS
  Open_Texture poggers_texture = load_texture_from_file("poggers.png");
  size_t poggers = open_renderer_push_image(&or, poggers_texture); 

  stbi_image_free(monka_texture.data);
  stbi_image_free(poggers_texture.data);

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

      or.image = monka;
      open_renderer_set_shader(&or, SHADER_FOR_RIPPLE);
      open_renderer_image_rect(&or, vec2fs(0), vec2f(w, h), vec2fs(0), vec2f(1, 1));
      open_renderer_flush(&or);

      or.image = poggers;
      open_renderer_set_shader(&or, SHADER_FOR_IMAGE);
      open_renderer_image_rect(&or, pos, size, vec2fs(0), vec2f(1, 1));
      open_renderer_flush(&or);

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
