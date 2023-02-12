#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL.h>

#define OPEN_RENDERER_IMPLEMENTATION
#include "../datastructures-c/libs/open_renderer.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "./stb_image_write.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "./stb_truetype.h"

#define STB_IMAGE_IMPLEMENTATION
#include "./stb_image.h"

#define WIDTH 800
#define HEIGHT 600

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

#define FONT_HEIGHT 94
#define ASCII_VALS (127-32)

unsigned char ascii2[ASCII_VALS * FONT_HEIGHT * FONT_HEIGHT] = {0};
int ascii_w[ASCII_VALS];
int ascii_h[ASCII_VALS];
int ascii_x[ASCII_VALS];
int ascii_y[ASCII_VALS];

stbtt_fontinfo font = {0};

Open_Texture font_texture = {0};
unsigned char ttf_buffer[1<<25] = {0};

void prepare_font() {
  //
  // /usr/share/fonts/truetype/iosevka/iosevka-regular.ttf
  fread(ttf_buffer, 1, 1<<25, fopen("/usr/share/fonts/truetype/freefont/FreeSans.ttf", "rb"));
  
  stbtt_InitFont(&font, ttf_buffer, 0);

  float scale = stbtt_ScaleForPixelHeight(&font, (float) FONT_HEIGHT);

  for(int c=32;c<=126;c++) {
    size_t glyph_off = (c-32)*FONT_HEIGHT;
    size_t ascii_width = ASCII_VALS * FONT_HEIGHT;
    
    int w, h, x ,y;
    unsigned char *bitmap =
      stbtt_GetCodepointSDF(&font, scale, c, 0, 128, 128.0, &w, &h, &x, &y);
    //stbtt_GetCodepointBitmap(&font, 0, scale, c, &w, &h, &x, &y);

    if(bitmap == NULL) {
      continue;
    }

    ascii_x[c - 32] = x;
    ascii_y[c - 32] = y;
    
    ascii_w[c-32] = w;
    ascii_h[c-32] = h;
    
    
    for (int j=0; j < FONT_HEIGHT; ++j) {
      for (int i=0; i < FONT_HEIGHT; ++i) {
	//ascii2[j*ascii_width+glyph_off+i] = 0x00;
	if(i<w && j<h) {
	  unsigned char d = bitmap[j*w+i];	  
	  ascii2[j*ascii_width+glyph_off+i] = d;
	}
      }
    }
    
    stbtt_FreeBitmap(bitmap, font.userdata);
  }

  /*
  if(!stbi_write_png("font.png", ASCII_VALS * FONT_HEIGHT, FONT_HEIGHT, 1, ascii2, ASCII_VALS * FONT_HEIGHT)) {
    fprintf(stderr, "Failed to save as png\n");
    exit(1);
  }
  */
}

void render_line(Open_Renderer *or, const char *word, Vec2f pos) {
  float factor = 4.f;

  size_t word_len = strlen(word);
  for(size_t i=0;i<word_len;i++) {
    char c = word[i];
    if(c == ' ') {
      pos.x += factor * FONT_HEIGHT / 2.f;
      continue;
    }
    
    float char_width = (float) ascii_w[c - 32] / (float) font_texture.width;
    float char_off = (float) (c - 32) * (float) FONT_HEIGHT / (float) font_texture.width;	
    
    Vec2f c_size = vec2f(factor * (float) ascii_w[c-32], factor * FONT_HEIGHT);

    pos.x += factor * (float) ascii_x[c-32];
    if(pos.x >= WIDTH) break;
	
    open_renderer_text_rect(or, vec2f(pos.x, pos.y + factor * (float) ascii_y[c -32]), c_size,
			    vec2f(char_off, 0), vec2f(char_width, 1),
			    vec4f(1, 0.7, 0.3, 1));
    if(i!=word_len - 1) pos.x += c_size.x;
  }
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
  size_t monka = open_renderer_push_texture(&or, monka_texture);
  (void) monka;
  
  //POGGERS
  Open_Texture poggers_texture = load_texture_from_file("poggers.png");
  size_t poggers = open_renderer_push_texture(&or, poggers_texture);
  (void) poggers;  

  stbi_image_free(monka_texture.data);
  stbi_image_free(poggers_texture.data);

  prepare_font();  
  font_texture.width = ASCII_VALS * FONT_HEIGHT;
  font_texture.height = FONT_HEIGHT;
  font_texture.data = (char *) ascii2;
  font_texture.grey = true;
  size_t font_index = open_renderer_push_texture(&or, font_texture);

  SDL_Event event;
  bool quit = false;

  Vec2f pos = vec2f(0.f, 0.f);
  Vec2f vel = vec2f(200, 100);
  Vec2f size = vec2fs(150);
  (void) pos;
  (void) vel;
  (void) size;
  
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
      //RES
      int w, h;
      SDL_GetWindowSize(window, &w, &h);      
      or.resolution = vec2i(w, h);

      //TIME
      float now = (float) SDL_GetTicks() / 1000.0f;
      float dt = now - or.time;
      (void) dt;
      or.time = now;

      pos = vec2f_add(pos, vec2f_mul(vel, vec2fs(dt)));
      if(pos.x + size.x >= (float) w || pos.x < 0.f) {
	vel.x *= -1;
      }
      if(pos.y + size.y>= (float) h || pos.y < 0.f) {
	vel.y *= -1;
      }

      /*
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

      */
      or.image = font_index;
      open_renderer_set_shader(&or, SHADER_FOR_TEXT);
      render_line(&or, "Hello, World!", vec2f(0, HEIGHT/2));
      open_renderer_flush(&or);
    }

    //END RENDER
    SDL_GL_SwapWindow(window);
  }  

  SDL_DestroyWindow(window);
  SDL_Quit();
  
  return 0;
}
