//! ����crc16
 /*!
    \param [in] data ��Ҫ����crc16������
    \param [in] data_length ��Ҫ����crc16���ݵĳ���
    \param [in] last_crc16 �̶�����0
    \return ����crc16ֵ
    \details ��
*/
#ifdef __cplusplus 
extern "C" {
#endif

unsigned short get_crc16(	void *data,	unsigned int data_length, unsigned short last_crc16);

#ifdef __cplusplus
}
#endif