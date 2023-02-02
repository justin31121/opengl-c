#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>

#include "./sdl_c.h"

#define PEN_IMPLEMENTATION
#include "pen.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define WIDTH 800
#define HEIGHT 600

uint32_t *source;

#define BUFFER_CAP 1024

char buffer[BUFFER_CAP] = {0};
size_t buffer_size = 0;

SDL_Renderer *rend;
SDL_Texture *texture;
bool running = true;

void render() {
  SDL_RenderClear(rend);
  SDL_RenderCopy(rend, texture, NULL, NULL);
  SDL_RenderPresent(rend);
}

unsigned char ttf_buffer[1<<25];

#define FONT_HEIGHT 22
#define FONT_SPACE_WIDTH 12
#define ASCII_VALS 127-32

uint32_t ascii[ASCII_VALS][FONT_HEIGHT * FONT_HEIGHT];
int ascii_w[ASCII_VALS];
int ascii_h[ASCII_VALS];

float estimate_word_len(stbtt_fontinfo *font, const char*word, size_t word_len) {
  float xpos = 0.f;
  float scale = stbtt_ScaleForPixelHeight(font, FONT_HEIGHT);

  for(size_t k=0;k<word_len;k++) {
    float x_shift = xpos - (float) floor(xpos);
    int advance, lsb;
    stbtt_GetCodepointHMetrics(font, word[k], &advance, &lsb);

    int x0, y0, x1, y1;
    stbtt_GetCodepointBitmapBoxSubpixel(font, word[k], scale, scale, x_shift, 0, &x0, &y0, &x1, &y1);

    xpos += (advance * scale);
    if(word[k+1]<=126) {
      xpos += scale*stbtt_GetCodepointKernAdvance(font, word[k], word[k+1]);
    }
  }

  return xpos;
}

void render_word_len(stbtt_fontinfo *font, const char *word, size_t word_len, float x, int y) {
  float xpos = x;
  float scale = stbtt_ScaleForPixelHeight(font, FONT_HEIGHT);

  for(size_t k=0;k<word_len;k++) {
    float x_shift = xpos - (float) floor(xpos);
    int advance, lsb;
    stbtt_GetCodepointHMetrics(font, word[k], &advance, &lsb);

    int x0, y0, x1, y1;
    stbtt_GetCodepointBitmapBoxSubpixel(font, word[k], scale, scale, x_shift, 0, &x0, &y0, &x1, &y1);

    int w = ascii_w[word[k] - 32];
    int h = ascii_h[word[k] - 32];
    
    copy(source, WIDTH, HEIGHT, ascii[word[k] - 32], w, h, (int) xpos + x0, y + y0, w, h);

    xpos += (advance * scale);
    if(word[k+1]<=126) {
      xpos += scale*stbtt_GetCodepointKernAdvance(font, word[k], word[k+1]);
    }
  }  
} 

void render_text_len(stbtt_fontinfo *font, const char* text, size_t text_len,
		     int x, int y,
		     int *_x, int *_y) {
  (void) _x;
  (void) _y;
  float xpos = (float) x;
  size_t i=0;
  size_t last = 0;
  while(i<text_len) {
    while(i<text_len && text[i]!=' ') i++;
    if(i<=text_len) {
      //RENDER
      printf("'%.*s'\n", (int) (i - last), buffer + last);
      float __x = estimate_word_len(font, buffer+last, i - last);
      xpos += __x;
      if((int) xpos >= WIDTH) {
	xpos = 0.f;
	y+=2*FONT_HEIGHT;
      }
      render_word_len(font, buffer+last, i - last, xpos==0 ? 0 : xpos-__x, y);
      last = i;
    }
    while(i<text_len && text[i]==' ') {
      i++;
      last++;
      xpos += (float) FONT_SPACE_WIDTH;
      if((int) xpos >= WIDTH) {
	xpos = 0.f;
	y+=2*FONT_HEIGHT;
      }
    }
  }
}

void render_text(stbtt_fontinfo *font, const char* text, int x, int y, int *_x, int *_y) {
  render_text_len(font, text, strlen(text), x, y, _x, _y);
}

void* loop(void *ptr) {
  (void) ptr;

  fill(source, WIDTH, HEIGHT, 0xff181818);

  stbtt_fontinfo font;
  fread(ttf_buffer, 1, 1<<25, fopen("c:/windows/fonts/consola.ttf", "rb"));
  
  stbtt_InitFont(&font, ttf_buffer, 0);

  float scale = stbtt_ScaleForPixelHeight(&font, FONT_HEIGHT);

  for(int c=32;c<=126;c++) {
    int w, h;
    unsigned char *bitmap =
      stbtt_GetCodepointBitmap(&font, 0, scale, c, &w, &h, 0, 0);

    for (int j=0; j < FONT_HEIGHT; ++j) {
      for (int i=0; i < FONT_HEIGHT; ++i) {
	if(i<w && j<h) {
	  unsigned char d = bitmap[j*w+i];
	  if(d) {
	    ascii[c-32][j*w+i] = 0xff000000 | (d<<0*8) | (d<<1*8) | (d<<2*8);
	    continue;
	  }
	}
	ascii[c-32][j*FONT_HEIGHT+i] = 0xff181818;
      }
    }
    ascii_w[c-32] = w;
    ascii_h[c-32] = h;
    
    stbtt_FreeBitmap(bitmap, font.userdata);
  }



  while(running) {
    fill(source, WIDTH, HEIGHT, 0x181818);
    int x, y;
    render_text_len(&font,
		  buffer, buffer_size,
		  0, 1*FONT_HEIGHT, &x, &y); 
    render();
    SDL_Delay(100);
  }
  
  return NULL;
}

int main(int argc, char** argv) {
  (void) argc;
  (void) argv;

  scc(SDL_Init(SDL_INIT_VIDEO));

  SDL_Window *wind = scp(SDL_CreateWindow(
					  "Image",
					  SDL_WINDOWPOS_CENTERED,
					  SDL_WINDOWPOS_CENTERED,
					  WIDTH, HEIGHT, 0));

  rend = scp(SDL_CreateRenderer(
				wind,
				-1,
				SDL_RENDERER_SOFTWARE));

  texture = scp(SDL_CreateTexture(
				  rend,
				  SDL_PIXELFORMAT_RGBA32,
				  SDL_TEXTUREACCESS_STREAMING,
				  WIDTH,
				  HEIGHT));

  int texture_pitch;
  void* texture_pixels = NULL;
  scc(SDL_LockTexture(texture, NULL, &texture_pixels, &texture_pitch));

  source = (uint32_t *) texture_pixels;

  SDL_Event event;
  size_t len;

  pthread_t thread;
  if(pthread_create(&thread, NULL, loop, NULL) < 0 ) {
    fprintf(stderr, "ERROR: Can not create thread");
    exit(1);
  }

  while(running) {
    SDL_WaitEvent(&event);

    switch(event.type) {
    case SDL_KEYDOWN:
      switch(event.key.keysym.sym) {
      case SDLK_BACKSPACE:
	buffer_size = buffer_size ? buffer_size-1 : buffer_size;
      }
      break;
    case SDL_TEXTINPUT:
      len = strlen(event.text.text);
      if(buffer_size + len >= BUFFER_CAP) {
	fprintf(stderr, "ERROR: Buffer-Overflow\n");
	exit(1);
      }
      memcpy(buffer + buffer_size, event.text.text, len);
      buffer_size += len;
      break;
    case SDL_QUIT:
      running = false;
      break;
    }

    render();
  }

  SDL_UnlockTexture(texture);
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(rend);
  SDL_DestroyWindow(wind);

  SDL_Quit();

  pthread_join(thread, NULL);
  
  return 0;
}
