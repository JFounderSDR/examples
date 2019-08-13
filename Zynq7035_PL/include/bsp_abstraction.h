/***************************************************************************//**
* @file    bsp_abstraction.h
* @author  maketure
* @version V1.0.0
* @date    2018-09-22
* @brief   the file declaration interface serve for system manage software.
* @remarks modification history
* @verbatim
* ==============================================================================
* <Date> | <Version> | <Author> | <Description>
* ==============================================================================
* 2018-09-22 | 1.0.0.0 | maketure | Create file
* ==============================================================================
* @endverbatim
* ******************************************************************************
* @attention
*
* <h2><center>&copy; Copyright(c)2017-2022 JFounder Info Tech Co.,Ltd</center></h2>
*
* All rights reserved. The right to copy, distribute, modify or otherwise make use
* of this software may be licensed only pursuant to the terms
* of an applicable JFounder license agreement.\n
*
*//****************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_ABSTRACTION_H
#define __BSP_ABSTRACTION_H

/* Includes ------------------------------------------------------------------*/
#include <assert.h>
#include <vxWorks.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief  写入实例号到FPGA SRIO组件
 * @param  instno [写入的实例号]
 * @return        [OK if success，ERRROR otherwise]
 */
STATUS fpga_srio_comp_ins_write(UINT8 instno);


/*
 * @brief  写入LD-PD映射区域的信息标识字段
 * @param  msgtype [信息类型]
 * @return         [OK if success，ERRROR otherwise]
 */
STATUS fpga_srio_map_identify_writre(UINT32 msgtype);


/*
 * @brief   读取LD-PD映射区域的信息类型
 *
 * @onte    无实际作用
 *
 * @return  消息类型
 */
UINT32 fpga_srio_map_identify_read(void);

/*
 * @brief  写入表格项
 * @param  items   [表格项数]
 * @return         [OK if success，ERRROR otherwise]
 */
STATUS fpga_srio_items_write(UINT32 items);


/*
 * @brief   读取LD-PD映射区域的表格大小
 *
 * @return  LD-PD表格大小
 */
UINT32 fpga_srio_items_read(void);


/*
 * @brief  写入LD-PD的映射关系到映射表格
 * @param  LD     [逻辑地址]
 * @param  PD     [物理地址]
 * @param  itemNo [表格项范围1-254]
 * @return        [OK if success，ERRROR otherwise]
 */
STATUS fpga_srio_LD_PD_write(UINT16 LD, UINT16 PD, int itemno);


/*
 * @brief  读取item项LD-PD的映射关系
 * @param  itemNo [表格项范围1-254]
 * @return        [返回LD-PD的映射关系]
 */
UINT32 fpga_srio_LD_PD_read(int itemno);


/*
 * 擦去所有FPGA SRIO LD-PD映射关系,包括区域标识等
 */
STATUS fpga_srio_map_area_erase(void);


/*
 * @brief 		写入以太网接收到的波形参数到FPGA,通过AXI接口
 * @param[in]	写入数据的起始地址
 * @param[in]   写入数据的长度
 */
STATUS eth_axi_param_area_write(UINT8 *data, int size);

/*
 * @brief 		读取FPGA需要发送的波形参数
 * @param[out]	读取数据的起始地址
 * @param[in]   读取数据的长度
 */
STATUS eth_axi_param_area_read(UINT8 *data, int size);

/*
 * @brief 		写入以太网接收到的波形数据到FPGA,通过AXI接口
 * @param[in]	写入数据的起始地址
 * @param[in]   写入数据的长度
 */
STATUS eth_axi_data_area_write(UINT8 *data, int size);

/*
 * @brief 		读取FPGA需要发送的波形数据
 * @param[out]	读取数据的起始地址
 * @param[in]   读取数据的长度
 */
STATUS eth_axi_data_area_read(UINT8 *data, int size);


#ifdef __cplusplus
}
#endif

#endif /* __BSP_ABSTRACTION_H */

/************************ (C) COPYRIGHT JieFang Info Tech *****END OF FILE****/
