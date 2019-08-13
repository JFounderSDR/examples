#pragma once
/*
 * libu1a1.h
 *
 *  Created on: 2019��5��20��
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
	NORMAL = 1,	//ѹ���� 2:1
	RAPID			//ѹ���� 16:5
}u1alg_audio_codec_type_t;


//! ������Ƶ�����״̬��
 /*!
    \param [in] type ѹ��������
	\return ����״̬��ָ��
	\details ��Ƶѹ�����롢��ѹ������Ҫ���Բ�ͬ��״̬��
*/
u1lag_audio_codec_handle_t u1lag_audio_codec_create(u1alg_audio_codec_type_t type);

//! �ͷ���Ƶ�����״̬��
 /*!
	\param [in] handle ״̬��ָ��
	\return ��
	\details ��
*/
void u1lag_audio_codec_delete(u1lag_audio_codec_handle_t handle);


//! PCM��Ƶѹ������
 /*!
	\param [in] handle ������״̬��ָ��
	\param [in] code ���ѹ����������
	\param [in] pcm ��Ҫѹ����������
	\param [in] length ��Ҫѹ���������ݵĳ���
	\return ѹ����������ݳ���
	\details �ú���ֻ��ѹ��������СΪ16bit��PCM��Ƶ���ݣ�ѹ�����ɴ�����������
*/
int u1lag_audio_codec_encode(u1lag_audio_codec_handle_t handle, unsigned char * code, short * pcm, int length);


//! PCM��Ƶ��ѹ����
 /*!
	\param [in] handle ����״̬��ָ��
    \param [in] code ��Ҫ��ѹ��������
	\param [in] pcm ��ɽ�ѹ��������
	\param [in] length ��Ҫ��ѹ�������ݵĳ���
	\return ��
	\details �ú�������Ƶѹ������ķ�������ѹ�����ɴ�����������
*/
int u1lag_audio_codec_decode(u1lag_audio_codec_handle_t handle, unsigned char * code, short * pcm, int length);

//! ����crc16������
 /*!
    \param [in] poly[] ����ʽ���� x^16 x^15 ... x^1 x^0
    \return Return crc16������
    \details More ����ʽ����Ԫ����0��1
*/
u1alg_crc16_handle_t u1alg_crc16_create(unsigned int poly[]);

//! ʹ��crc16������
 /*!
    \param [in] handle crc16������
    \param [in] data ��Ҫ�������ݻ�ַ
    \param [in] size ���ݳ���
    \param [in] last ��һ��crc16������
    \return Return crc16������
    \details ��
*/
unsigned short u1alg_crc16_figure(u1alg_crc16_handle_t handle, void * data, unsigned int size, unsigned short last);

//! �ͷ�crc16������
 /*!
    \param [in] handle crc16������
    \return Return ��
    \details ��
*/
void u1alg_crc16_delete(u1alg_crc16_handle_t handle);

#ifdef __cplusplus 
}
#endif

#endif /* LIBU1A1_H_ */
