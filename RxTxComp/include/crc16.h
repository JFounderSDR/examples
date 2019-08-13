//! 计算crc16
 /*!
    \param [in] data 需要计算crc16的数据
    \param [in] data_length 需要计算crc16数据的长度
    \param [in] last_crc16 固定参数0
    \return 返回crc16值
    \details 无
*/
#ifdef __cplusplus 
extern "C" {
#endif

unsigned short get_crc16(	void *data,	unsigned int data_length, unsigned short last_crc16);

#ifdef __cplusplus
}
#endif