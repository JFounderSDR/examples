
#include "../include/libu1alg.h"
#include "../include/g72x.h"


struct u1lag_audio_codec_s
{
	struct g72x_state * codec_handle;
	u1alg_audio_codec_type_t codec_type;
};
//---------------------------------------------------------------------------------------------------------

struct u1alg_crc16_s
{
	unsigned short crc16_table[256];
};
//---------------------------------------------------------------------------------------------------------

u1lag_audio_codec_handle_t u1lag_audio_codec_create(u1alg_audio_codec_type_t type)
{
	struct g72x_state * codec_handle = (struct g72x_state *)malloc(sizeof(struct g72x_state));
	struct u1lag_audio_codec_s * handle = (struct u1lag_audio_codec_s *)malloc(sizeof(struct u1lag_audio_codec_s));
	handle->codec_type = type;
	handle->codec_handle = codec_handle;
	if (handle)
	{
		g72x_init_state(handle->codec_handle);
	}
	return handle;
}

//---------------------------------------------------------------------------------------------------------

void u1lag_audio_codec_delete(u1lag_audio_codec_handle_t handle)
{
	free(handle->codec_handle);
	free(handle);
}

//---------------------------------------------------------------------------------------------------------

int u1lag_audio_codec_encode(u1lag_audio_codec_handle_t handle, unsigned char * code, short * pcm, int length)
{
	int in_coding;
	int(*enc_routine)();
	int i, resid;
	int enc_bits = 5;
	int bytes = 0;
	unsigned char  encode;
	int count = length / 2;
	in_coding = AUDIO_ENCODING_LINEAR;
	enc_routine = g723_40_encoder;
	if (NORMAL == handle->codec_type)
	{
		for (i = 0; i < count; i++)
		{
			code[i] = (*enc_routine)(pcm[i], in_coding, (struct g72x_state*)handle->codec_handle);
		}
		return count;
	}
	if (RAPID == handle->codec_type)
	{
		for (i = 0; i < count; i++)
		{
			encode = (*enc_routine)(pcm[i], in_coding, (struct g72x_state*)handle->codec_handle);
			resid = pack_output(encode, code, enc_bits, &bytes);
		}
		while (resid)
		{
			resid = pack_output(0, code, enc_bits, &bytes);
		}
		return bytes;
	}
	
}

//---------------------------------------------------------------------------------------------------------

int u1lag_audio_codec_decode(u1lag_audio_codec_handle_t handle, unsigned char * code, short * pcm, int length)
{
	unsigned char decode = { 0 };
	int out_coding;
	int(*dec_routine)();
	int i;
	out_coding = AUDIO_ENCODING_LINEAR;
	dec_routine = g723_40_decoder;

	if (NORMAL == handle->codec_type)
	{
		for (i = 0; i < length; i++)
		{
			pcm[i] = (*dec_routine)(code[i], out_coding, (struct g72x_state*)handle->codec_handle);
		}
		return length * 2;
	}

	if (RAPID == handle->codec_type)
	{
		int index = 0, pcm_len = 0, dec_bits = 5, i;
		for (i = 0; unpack_input(&decode, code, &index, dec_bits, length) >= 0; i++)
		{
			pcm[i] = (*dec_routine)(decode, out_coding, (struct g72x_state*)handle->codec_handle);
			pcm_len += 2;
		}
		return pcm_len;
	}
}

//---------------------------------------------------------------------------------------------------------

int pack_output(unsigned code, unsigned char * buffer ,int bits, int * bytes)
{
	static unsigned int out_buffer = 0;
	static int out_bits = 0;
	unsigned char out_byte;
	out_buffer |= (code << out_bits);
	out_bits += bits;
	if (out_bits >= 8)
	{
		out_byte = out_buffer & 0xff;
		out_bits -= 8;
		out_buffer >>= 8;
		buffer[*bytes] = out_byte;
		*bytes+= 1;
	}
	return (out_bits > 0);
}

//---------------------------------------------------------------------------------------------------------

int unpack_input(unsigned char *code, unsigned char * buffer, int * index,int bits, int buf_len )
{
	static unsigned int in_buffer = 0;
	static int in_bits = 0;
	unsigned char in_byte;
	if (in_bits < bits)
	{
		if(*index >= buf_len)
		{
			*code = 0;
			return (-1);
		}
		in_byte = buffer[*index];
		(*index)++;
		in_buffer |= (in_byte << in_bits);
		in_bits += 8;
	}
	*code = in_buffer & ((1 << bits) - 1);
	in_buffer >>= bits;
	in_bits -= bits;
	
	return (in_bits > 0);
}

//---------------------------------------------------------------------------------------------------------

short poly_to_gener(unsigned int poly[])
{
	unsigned short gener = 0;
	unsigned short bit = 1;
	unsigned int  i;
	for (i = 16; i > 0; i--)
	{
		gener += poly[i] * bit;
		bit = bit << 1;
	}
	return gener;
}

//---------------------------------------------------------------------------------------------------------

void crc16_make_table(unsigned short table[256], const unsigned short gener)
{
	unsigned short c;
	int i, j, t1;
	for (i = 0; i < 256; ++i)
	{
		c = 0xFF00 & (i << 8);
		for (j = 0; j < 8; ++j)
		{
			t1 = c & 0x8000;
			c <<= 1;
			if (t1 != 0)
				c ^= gener;
		}
		table[i] = c;
	}
}

//---------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include <stddef.h>

u1alg_crc16_handle_t u1alg_crc16_create(unsigned int poly[])
{
	u1alg_crc16_handle_t handle = NULL;
	handle = (u1alg_crc16_handle_t)malloc(sizeof(struct u1alg_crc16_s));
	if (NULL != handle && NULL != poly)
	{
		unsigned short gener = poly_to_gener(poly);
		crc16_make_table(handle->crc16_table, gener);
		return handle;
	}
}

//---------------------------------------------------------------------------------------------------------

unsigned short u1alg_crc16_figure(u1alg_crc16_handle_t handle, void *data, unsigned int size, unsigned short last)
{
	unsigned short crc16_reg = ~last;
	unsigned char *byte_data = (unsigned char *)data;
	int i;
	for (i = 0; i < size; i++)
	{
		crc16_reg = (crc16_reg >> 8) ^ handle->crc16_table[(crc16_reg ^ byte_data[i]) & 0xff];
	}
	return ~crc16_reg;
}

//---------------------------------------------------------------------------------------------------------

void u1alg_crc16_delete(u1alg_crc16_handle_t handle)
{
	free(handle);
}