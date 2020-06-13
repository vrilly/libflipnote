#ifndef LIBFLIPNOTE_FLIPNOTE_H
#define LIBFLIPNOTE_FLIPNOTE_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

typedef struct __attribute__((packed))
{
	char magic[4]; // PARA

	uint32_t anim_size;
	uint32_t snd_size;

	uint16_t frame_count;
	uint16_t unknown; // 0x2400
	uint16_t locked;
	uint16_t thumbnail_frame;
	uint16_t root_author_name[11];
	uint16_t parent_author_name[11];
	uint16_t current_author_name[11];
	uint64_t parent_author_id;
	uint64_t current_author_id;

	char parent_filename[18];
	char current_filename[18];
	uint64_t root_author_id;
	char partial_filename[8];
	uint32_t timestamp;
	uint16_t reserved; // 0x0
} t_ppmheader;

typedef struct __attribute__((packed))
{
	uint16_t frametable_size;
	uint16_t unknown; // 0
	uint32_t flags;

	uint32_t *frametable;
} t_animheader;

typedef struct
{
	uint8_t line_encoding[192];
	uint32_t *framebuffer;
} t_layer;

typedef struct
{
	union
	{
		struct
		{
			uint frame_type:1;
			uint frame_translate:2;
			uint frame_ci2:2;
			uint frame_ci1:2;
			uint frame_paper_color:1;
		};
		int frame_header;
	};

	// translation position for frame diffing
	uint8_t fd_x;
	uint8_t fd_y;

	t_layer layer_1;
	t_layer layer_2;
} t_frame;

typedef struct
{
	t_ppmheader header;
	t_animheader anim_hdr;
	t_frame *frames;

	int current_frame;
} t_flipnote;

t_flipnote *read_ppm_from_file(char *path);

t_frame *get_next_frame(t_flipnote *flipnote);
void	conv_utf16_ansi(uint16_t *src, char *dst);

#endif //LIBFLIPNOTE_FLIPNOTE_H
