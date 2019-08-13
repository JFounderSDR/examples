#pragma once
/*
 * libu1a1.h
 *
 *  Created on: 2019年5月20日
 *      Author: ziggy
 */

#ifndef LIBU1A1_H_
#define LIBU1A1_H_

#ifdef __cplusplus 
extern "C" {
#endif

struct u1lag_audio_codec_s;
typedef struct u1lag_audio_codec_s   *u1lag_audio_codec_handle_t;

struct u1alg_crc16_s;
typedef struct u1alg_crc16_s * u1alg_crc16_handle_t;


typedef enum codec_type_t
{
	NORMAL = 1,	//压缩比 2:1
	RAPID			//压缩比 16:5
}u1alg_audio_codec_type_t;


//! 创建音频编解码状态机
 /*!
    \param [in] type 压缩比类型
	\return 返回状态机指针
	\details 音频压缩编码、解压解码需要各自不同的状态机
*/
u1lag_audio_codec_handle_t u1lag_audio_codec_create(u1alg_audio_codec_type_t type);

//! 释放音频编解码状态机
 /*!
	\param [in] handle 状态机指针
	\return 无
	\details 无
*/
void u1lag_audio_codec_delete(u1lag_audio_codec_handle_t handle);


//! PCM音频压缩编码
 /*!
	\param [in] handle 编码码状态机指针
	\param [in] code 完成压缩编码数据
	\param [in] pcm 需要压缩编码数据
	\param [in] length 需要压缩编码数据的长度
	\return 压缩编码后数据长度
	\details 该函数只能压缩采样大小为16bit的PCM音频数据，压缩比由创建函数决定
*/
int u1lag_audio_codec_encode(u1lag_audio_codec_handle_t handle, unsigned char * code, short * pcm, int length);


//! PCM音频解压解码
 /*!
	\param [in] handle 解码状态机指针
    \param [in] code 需要解压解码数据
	\param [in] pcm 完成解压解码数据
	\param [in] length 需要解压解码数据的长度
	\return 无
	\details 该函数是音频压缩编码的反操作，压缩比由创建函数决定
*/
int u1lag_audio_codec_decode(u1lag_audio_codec_handle_t handle, unsigned char * code, short * pcm, int length);

//! 创建crc16计算器
 /*!
    \param [in] poly[] 多项式数组 x^16 x^15 ... x^1 x^0
    \return Return crc16计算器
    \details More 多项式数组元素填0或1
*/
u1alg_crc16_handle_t u1alg_crc16_create(unsigned int poly[]);

//! 使用crc16计算器
 /*!
    \param [in] handle crc16计算器
    \param [in] data 需要计算数据基址
    \param [in] size 数据长度
    \param [in] last 上一次crc16计算结果
    \return Return crc16计算结果
    \details 无
*/
unsigned short u1alg_crc16_figure(u1alg_crc16_handle_t handle, void * data, unsigned int size, unsigned short last);

//! 释放crc16计算器
 /*!
    \param [in] handle crc16计算器
    \return Return 无
    \details 无
*/
void u1alg_crc16_delete(u1alg_crc16_handle_t handle);

#ifdef __cplusplus 
}
#endif

#endif /* LIBU1A1_H_ */
