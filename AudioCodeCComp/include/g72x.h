/*
 * This source code is a product of Sun Microsystems, Inc. and is provided
 * for unrestricted use.  Users may copy or modify this source code without
 * charge.
 *
 * SUN SOURCE CODE IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING
 * THE WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 *
 * Sun source code is provided with no support and without any obligation on
 * the part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 *
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY THIS SOFTWARE
 * OR ANY PART THEREOF.
 *
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 *
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */
//#include "g72x.h"
/*
 * g72x.h
 *
 * Header file for CCITT conversion routines.
 *
 */
#ifndef _G72X_H
#define	_G72X_H

#define	AUDIO_ENCODING_ULAW	(1)	/* ISDN u-law */
#define	AUDIO_ENCODING_ALAW	(2)	/* ISDN A-law */
#define	AUDIO_ENCODING_LINEAR	(3)	/* PCM 2's-complement (0-center) */

#ifdef __cplusplus 
extern "C" {
#endif
/*
 * The following is the definition of the state structure
 * used by the G.721/G.723 encoder and decoder to preserve their internal
 * state between successive calls.  The meanings of the majority
 * of the state structure fields are explained in detail in the
 * CCITT Recommendation G.721.  The field names are essentially indentical
 * to variable names in the bit level description of the coding algorithm
 * included in this Recommendation.
 */
struct g72x_state {
	long yl; /* Locked or steady state step size multiplier. 锁定或稳定状态步长乘数。*/
	short yu; /* Unlocked or non-steady state step size multiplier. 未锁定或非稳定状态步长乘数。*/
	short dms; /* Short term energy estimate. 短期能源估算*/
	short dml; /* Long term energy estimate. 长期能源估算*/
	short ap; /* Linear weighting coefficient of 'yl' and 'yu'. 线性加权系数   锁定 非锁定*/

	short a[2]; /* Coefficients of pole portion of prediction filter. 预测滤波器的极点部分的系数*/
	short b[6]; /* Coefficients of zero portion of prediction filter. 预测滤波器的零部分的系数*/
	short pk[2]; /*
	 * Signs of previous two samples of a partially 前两个部分样本的记号
	 * reconstructed signal. 重建信号
	 */
	short dq[6]; /*
	 * Previous 6 samples of the quantized difference 前6个样本的量化差异
	 * signal represented in an internal floating point 信号以内部浮点表示
	 * format.
	 */
	short sr[2]; /*
	 * Previous 2 samples of the quantized difference 前2个样本的量化差异
	 * signal represented in an internal floating point
	 * format.
	 */
	char td; /* delayed tone detect, new in 1988 version */
};

/* External function definitions. */

extern void g72x_init_stat(struct g72x_state *);
extern int g721_encoder(int sample, int in_coding, struct g72x_state *state_ptr);
extern int g721_decoder(int code, int out_coding, struct g72x_state *state_ptr);
extern int g723_24_encoder(int sample, int in_coding,
		struct g72x_state *state_ptr);
extern int g723_24_decoder(int code, int out_coding,
		struct g72x_state *state_ptr);
extern int g723_40_encoder(int sample, int in_coding,
		struct g72x_state *state_ptr);
extern int g723_40_decoder(int code, int out_coding,
		struct g72x_state *state_ptr);

#ifdef __cplusplus 
}
#endif

#endif /* !_G72X_H */
