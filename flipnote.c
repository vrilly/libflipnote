#include "flipnote.h"

static void unpack_lc(uint8_t *dst, uint8_t *src)
{
	uint8_t byte;
	for (int i = 0; i < 48; i++)
	{
		byte = src[i];
		for (int ii = 0; ii < 8; ii += 2)
			dst[i * 4 + ii / 2] = (uint8_t)((byte >> ii) & 0x03);
	}
}

static void render_layer(t_layer *layer, FILE *fh)
{
	uint32_t *fb;

	fb = calloc(256 * 192, 4);
	for (int line = 0; line < 192; line++)
	{
		if (layer->line_encoding[line])
		{
			uint32_t chunkflags;
			int x = 0;
			if (layer->line_encoding[line] == 3)
				chunkflags = 0xFFFFFFFF;
			else
				fread(&chunkflags, 4, 1, fh);
			chunkflags = ((chunkflags >> 24) & 0xff) |
						 ((chunkflags << 8) & 0xff0000) |
						 ((chunkflags >> 8) & 0xff00) |
						 ((chunkflags << 24) & 0xff000000);
			while (chunkflags != 0)
			{
				if ((chunkflags & 0x80000000) == 0x80000000)
				{
					uint8_t chunk;
					fread(&chunk, 1, 1, fh);
					for (int bit = 0; bit < 8; bit++)
					{
						if ((chunk & 0x01) == 0x01)
							fb[line * 256 + x] = 0xFFFFFFFF;
						x++;
						chunk >>= 1;
					}
				}
				else
					x += 8;
				chunkflags <<= 1;
			}
		}
	}
	layer->framebuffer = fb;
}

t_frame *get_next_frame(t_flipnote *flipnote)
{
	flipnote->current_frame++;
	return (flipnote->frames + flipnote->current_frame);
}

void	conv_utf16_ansi(uint16_t *src, char *dst)
{
	for (int i = 0; i < 11; i++)
	{
		if (src[i] > 31 && src[i] < 128)
			dst[i] = (char)(src[i]);
		else
			dst[i] = ' ';
	}
	dst[11] = 0;
}

t_flipnote *read_ppm_from_file(char *path)
{
	t_flipnote *flipnote;
	FILE *fh;
	long base_offst;

	flipnote = calloc(1, sizeof(t_flipnote));
	fh = fopen(path, "rb");
	if (!flipnote || !fh)
		return NULL;
	flipnote->current_frame = -1;
	if (!fread(&flipnote->header, sizeof(t_ppmheader), 1, fh) ||
		strncmp(flipnote->header.magic, "PARA", 4) != 0 ||
		fseek(fh, 1536, SEEK_CUR) == -1 ||
		!fread(&flipnote->anim_hdr, 8, 1, fh))
	{
		perror("uwu woopsie");
		free(flipnote);
		fclose(fh);
		return NULL;
	}
	flipnote->header.frame_count++;
	flipnote->frames = calloc(flipnote->header.frame_count * sizeof(t_frame),
							  1);
	if (!flipnote->frames)
		return (NULL);
	flipnote->anim_hdr.frametable =
			malloc(flipnote->anim_hdr.frametable_size);
	if (!flipnote->anim_hdr.frametable ||
		!fread(flipnote->anim_hdr.frametable,
			   flipnote->anim_hdr.frametable_size, 1, fh))
		return NULL;
	base_offst = ftell(fh);
	for (int i = 0; i < flipnote->header.frame_count; i++)
	{
		t_frame *fr;
		uint8_t buff[48];

		fr = flipnote->frames + i;
		fseek(fh, base_offst + flipnote->anim_hdr.frametable[i], SEEK_SET);
		if (!fread(&fr->frame_header, 1, 1, fh))
			break;
		if (fr->frame_translate)
		{
			fread(&fr->fd_x, 1, 1, fh);
			fread(&fr->fd_y, 1, 1, fh);
		}
		fread(&buff, 48, 1, fh);
		unpack_lc(fr->layer_1.line_encoding, buff);
		fread(&buff, 48, 1, fh);
		unpack_lc(fr->layer_2.line_encoding, buff);
		render_layer(&fr->layer_1, fh);
		render_layer(&fr->layer_2, fh);
	}
	fclose(fh);
	return (flipnote);
}
