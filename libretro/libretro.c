#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "osint.h"
#include "vecx.h"
#include "e8910.h"
#include "e6809.h"
#include "libretro.h"

#define STANDARD_BIOS

#ifdef STANDARD_BIOS
#include "bios/system.h"
#else
#ifdef FAST_BIOS
#include "bios/fast.h"
#else
#include "bios/skip.h"
#endif
#endif

#define WIDTH 330
#define HEIGHT 410
#define BUFSZ 135300

static retro_video_refresh_t video_cb;
static retro_input_poll_t poll_cb;
static retro_input_state_t input_state_cb;
static retro_environment_t environ_cb;
static retro_audio_sample_t audio_cb;

static unsigned char point_size;
static unsigned short framebuffer[BUFSZ];

/* Empty stubs */
void retro_set_controller_port_device(unsigned port, unsigned device){}
void retro_cheat_reset(void){}
void retro_cheat_set(unsigned index, bool enabled, const char *code){}
void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) {}
unsigned retro_get_region(void){ return RETRO_REGION_PAL; }
unsigned retro_api_version(void){ return RETRO_API_VERSION; }
bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info){ return false; }

void retro_deinit(void)
{
}

void *retro_get_memory_data(unsigned id){ return NULL; }
size_t retro_get_memory_size(unsigned id){ return 0; }

/* Emulator states */
extern unsigned snd_regs[16];

/* setters */
void retro_set_environment(retro_environment_t cb) { environ_cb = cb; }
void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t cb) { audio_cb = cb; }
void retro_set_input_poll(retro_input_poll_t cb) { poll_cb = cb; }
void retro_set_input_state(retro_input_state_t cb) { input_state_cb = cb; }

void retro_get_system_info(struct retro_system_info *info)
{
	memset(info, 0, sizeof(*info));
	info->library_name = "VecX";
#ifndef GIT_VERSION
#define GIT_VERSION ""
#endif
	info->library_version = "1.2" GIT_VERSION;
	info->need_fullpath = false;
	info->valid_extensions = "bin|vec";
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
	memset(info, 0, sizeof(*info));
	info->timing.fps            = 50.0;
	info->timing.sample_rate    = 44100;
	info->geometry.base_width   = WIDTH;
	info->geometry.base_height  = HEIGHT;
	info->geometry.max_width    = WIDTH;
	info->geometry.max_height   = HEIGHT;
	info->geometry.aspect_ratio = WIDTH / HEIGHT;
}

void retro_init(void)
{
   unsigned level = 5; 
   environ_cb(RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, &level);
}

size_t retro_serialize_size(void)
{
	return vecx_statesz();
}

bool retro_serialize(void *data, size_t size)
{
	return vecx_serialize((char*)data, size);
}

bool retro_unserialize(const void *data, size_t size)
{
	return vecx_deserialize((char*)data, size);
}

unsigned b;
unsigned char cart[65536];

bool retro_load_game(const struct retro_game_info *info)
{
   size_t cart_sz;
   struct retro_input_descriptor desc[] = {
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "Left" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "Up" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "Down" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "2" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "1" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,     "3" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,     "4" },

      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "Left" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "Up" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "Down" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "2" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "1" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,     "3" },
      { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,     "4" },
   };

   if (!info)
      return false;

   environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);

   e8910_init_sound();
   memset(framebuffer, 0, sizeof(framebuffer) / sizeof(framebuffer[0]));
   point_size = 1;

   /* start with a fresh BIOS copy */
	memcpy(rom, bios_data, bios_data_size);

   /* just memcpy buffer, ROMs are so tiny on Vectrex */
	cart_sz = sizeof(cart) / sizeof(cart[0]);

	if (info->data && info->size > 0 && info->size <= cart_sz)
   {
      memset(cart, 0, cart_sz);
      memcpy(cart, info->data, info->size);
      for(b = 0; b < sizeof(cart); b++)
      {
	   set_cart(b, cart[b]);
      }

      vecx_reset();
      e8910_init_sound();

      return true;
   }

	return false;
}

void retro_unload_game(void)
{
	memset(cart, 0, sizeof(cart) / sizeof(cart[0]));
	for(b = 0; b < sizeof(cart); b++)
	   set_cart(b, 0);
	vecx_reset();
}

void retro_reset(void)
{
	vecx_reset();
	e8910_init_sound();
}

#define RGB1555(col) ( (col) << 10 | (col) << 5 | (col) )

static inline void draw_point(int x, int y, unsigned char col)
{
	int psz = point_size;
	int sy, ey, sx, ex;
	
	if (psz == 1)
   {
		framebuffer[ (y * WIDTH) + x ] = RGB1555(col);
      return;
   }

	sy = y - psz > 0        ? y - psz : 0;
	ey = y + psz<= HEIGHT-1 ? y + psz : HEIGHT - 1;
	sx = x - psz > 0        ? x - psz : 0;
	ex = x + psz<= WIDTH -1 ? x + psz : WIDTH  - 1;
	
	for (y = sy; y <= ey; y++)
		for (x = sx; x <= ex; x++)
			if ( (x-sx) * (x-sx) + (y - sy) * (y - sy) <= psz * psz)
				framebuffer[ (y * WIDTH) + x ] = RGB1555(col);
}

/* plain old bresenham, AA etc. is up to the FE */
static inline void draw_line(unsigned x0, unsigned y0, unsigned x1, unsigned y1, unsigned char col)
{
	int dx = abs(x1-x0);
	int dy = abs(y1-y0);
	int sx = x0 < x1 ? 1 : -1;
	int sy = y0 < y1 ? 1 : -1;
	int err = dx - dy;
	int e2;

	while(1)
   {
      draw_point(x0, y0, col);

      if (x0 == x1 && y0 == y1)
         break;

      e2 = 2 * err;
      if (e2 > -dy)
      {
         err = err - dy;
         x0 = x0 + sx;
      }

      if (e2 < dx)
      {
         err = err + dx;
         y0 = y0 + sy;
      }
   }
}

void osint_render(void)
{
	int i;
	unsigned char intensity;
	unsigned x0, x1, y0, y1;

   (void)intensity;

	memset(framebuffer, 0, BUFSZ * sizeof(unsigned short));

   /* rasterize list of vectors */
	for (i = 0; i < vector_draw_cnt; i++)
   {
		unsigned char intensity = vectors_draw[i].color;
		x0 = (float)vectors_draw[i].x0 / (float)ALG_MAX_X * (float)WIDTH;
		x1 = (float)vectors_draw[i].x1 / (float)ALG_MAX_X * (float)WIDTH;
		y0 = (float)vectors_draw[i].y0 / (float)ALG_MAX_Y * (float)HEIGHT;
		y1 = (float)vectors_draw[i].y1 / (float)ALG_MAX_Y * (float)HEIGHT;

		if (intensity == 128)
			continue;
	    
		if (x0 - x1 == 0 && y0 - y1 == 0)
			draw_point(x0, y0, intensity);
		else
			draw_line(x0, y0, x1, y1, intensity);
	}
}

/* NOTE: issue with this core atm. (and thus, emulation) is partly input
 * (lightpens, analog axes etc. plugged into the different ports)
 * and statemanagement (as in, there is none currently) */

void retro_run(void)
{
	int i;
	unsigned char asamples[882] = {0};
	uint8_t buffer[882] = {0};

   (void)asamples;
	
   /* poll input and update states;
      buttons (snd_regs[14], 4 buttons/pl => 4 bits starting from LSB, |= for rel. &= ~ for push)
      analog stick (alg_jch0, alg_jch1, => -1 (0x00) .. 0 (0x80) .. 1 (0xff)) */
	poll_cb();

   /* Player 1 */

	if      (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT ))
      alg_jch0 = 0x00;
	else if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT))
      alg_jch0 = 0xff;
	else
      alg_jch0 = 0x80;
	
	if      (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP   ))
      alg_jch1 = 0xff;
	else if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN ))
      alg_jch1 = 0x00;
	else
      alg_jch1 = 0x80;

	
	if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A ))
      snd_regs[14] &= ~1;
	else
      snd_regs[14] |= 1;
	
	if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B ))
      snd_regs[14] &= ~2;
	else
      snd_regs[14] |= 2;

	if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X ))
      snd_regs[14] &= ~4;
	else
      snd_regs[14] |= 4;

	if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y ))
      snd_regs[14] &= ~8;
	else
      snd_regs[14] |= 8;

   /* Player 2 */
	if      (input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT ))
      alg_jch2 = 0x00;
	else if (input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT))
      alg_jch2 = 0xff;
	else
      alg_jch2 = 0x80;
	
	if      (input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP   ))
      alg_jch3 = 0xff;
	else if (input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN ))
      alg_jch3 = 0x00;
	else
      alg_jch3 = 0x80;

	if (input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A ))
      snd_regs[14] &= ~16;
	else
      snd_regs[14] |= 16;
	
	if (input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B ))
      snd_regs[14] &= ~32;
	else
      snd_regs[14] |= 32;

	if (input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X ))
      snd_regs[14] &= ~64;
	else
      snd_regs[14] |= 64;
	
	if (input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y ))
      snd_regs[14] &= ~128;
	else
      snd_regs[14] |= 128;
		
	vecx_emu(30000); /* 1500000 / 1000 * 20 */
	
	e8910_callback(NULL, buffer, 882);
	
	for (i = 0; i < 882; i++)
   {
		short convs = (buffer[i] << 8) - 0x7ff;
		audio_cb(convs, convs);
	}

	video_cb(framebuffer, WIDTH, HEIGHT, WIDTH * sizeof(unsigned short));
}
