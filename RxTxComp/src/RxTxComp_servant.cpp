/***************************************************************************//**
* @file     RxTxComp_servant.cpp
* @author   open Team
* @version  1
* @date     2019-05-05
* @brief
* @Details
* @Remark : <Description>
* @verbatim
* ==============================================================================
* <Date>     | <Version> | <Author>       | <Description>
* ==============================================================================
*  2019-05-05 | 1       | open Team       | Create file
* ==============================================================================
* @endverbatim
* ******************************************************************************
* <h2><center>&copy; Copyright(c)2017-2022 JFounder Info Tech Co.,Ltd</center></h2>
* All rights reserved. The right to copy, distribute, modify or otherwise make use
* of this software may be licensed only pursuant to the terms
* of an applicable JFounder license agreement.
*//****************************************************************************/

#include "../include/RxTxComp_servant.h"
#include "FileSystem_impl.h"
#include "SPDParser.h"
#include "utils.h"
#ifdef __SDS_OS_VXWORKS__
#include <usrLib.h>
#endif

#define SYNCHRO_HEAD	("@MAC")	// CRC header identification
#define TRANSFER_DATA_MAX_LEN		25000	// Maximum length of transmitted data

JTRS::OctetSequence g_recvSeqFromPC(20000);
JTRS::OctetSequence g_recvSeqFromMHAL(20000);

struct ErrorRate{
	unsigned long long checkSum;
	unsigned long long checkErrorNum;
} errorRate;

RxTxComp_servant::RxTxComp_servant()
{
}

/**
 * @brief	RxTxComp_servant Constructor.
 * 
 * @param[in] _id		component name.
 * @param[in] _sftwf1 	component SPD file name.
 * @param[in] _fsroot	file system root path.
 */
#ifdef __SDS_OS_VXWORKS__
RxTxComp_servant::RxTxComp_servant(
	const char * _id,
	const char * _cosNaming,
	const char * _appName,
	const char * _sftwfl,
	const char * _fsroot,
	pthread_cond_t * _shutdownCond):
Resource_impl(_id)
{
	DEBUG(6, [RxTxComp_servant], "In constructor.")
	
	
	m_orbWrap = new openscaSupport::ORB_Wrap::ORB_Wrap();
	m_appName = _appName;
	m_cosNaming = _cosNaming;
	m_spdRelPath = _sftwfl;
	m_fsroot = _fsroot;
	m_shutdownCond = _shutdownCond;
	pthread_mutex_init(&m_attrMtx, NULL);

    m_mhalPortNames.push_back("data_mhal_axi_out");
    m_mhalPortNames.push_back("data_mhal_axi_in");

	if(m_mhalPortNames.size() > 0) {
		setMhalPortLD();
    }
/**************************OPENSCA-USERREGION-BEGIN*******************************/


/**************************OPENSCA-USERREGION-END*********************************/
}

#elif defined __SDS_OS_LINUX__

RxTxComp_servant::RxTxComp_servant(
	const char * _id,
	const char * _cosNaming,
	const char * _appName,
	const char * _sftwfl,
	const char * _fsroot):
Resource_impl(_id)
{
	DEBUG(6, [RxTxComp_servant], "In constructor.")
	
	
	m_orbWrap = new openscaSupport::ORB_Wrap();
	m_appName = _appName;
	m_cosNaming = _cosNaming;
	m_spdRelPath = _sftwfl;
	m_fsroot = _fsroot;
	pthread_mutex_init(&m_attrMtx, NULL);

    m_mhalPortNames.push_back("data_mhal_axi_out");
    m_mhalPortNames.push_back("data_mhal_axi_in");

    data_mhal_axi_in_pport_local_LD = 0x0520;
    data_mhal_axi_out_uport_target_LD = 0x04a0;

	if(m_mhalPortNames.size() > 0) {
		setMhalPortLD();
    }
/**************************OPENSCA-USERREGION-BEGIN*******************************/
	data_out_uport = NULL;
	data_mhal_axi_out_uport = NULL;
	data_in_pport = NULL;
	data_mhal_axi_in_pport = NULL;
/**************************OPENSCA-USERREGION-END*********************************/
}
#endif

RxTxComp_servant::~RxTxComp_servant()
{
	DEBUG(6, [RxTxComp_servant], "in destructor")
/**************************OPENSCA-USERREGION-BEGIN*******************************/


/**************************OPENSCA-USERREGION-END*********************************/
}

/**
 * @brief 	Received data from AudioCodec component and adding CRC header to
 * 			facilitate CRC verification after receiving data, then send them to
 * 			the MHAL device.
 *
 * @param[in]	p  an examples of this class
 */
void
RxTxComp_servant::sendDataToMHAL()
{
	COMPDEBUG(7, [RxTxComp_servant::sendDataToMHAL],
		" enter in ...")

	static unsigned char data[TRANSFER_DATA_MAX_LEN];

	data_in_pport->getData(g_recvSeqFromPC);
	COMPDEBUG(4, [RxTxComp_servant::sendDataToMHAL],
		" g_recvSeqFromPC.length():" << g_recvSeqFromPC.length())

	unsigned short validLength = g_recvSeqFromPC.length();
	unsigned short crcNum = get_crc16(g_recvSeqFromPC.get_buffer(), validLength, 0);

	if (g_recvSeqFromPC.get_buffer() && (0 < validLength) &&
		(validLength <= TRANSFER_DATA_MAX_LEN)) {
		memset(data, 0, TRANSFER_DATA_MAX_LEN);
		memcpy(data, SYNCHRO_HEAD, 4);
		memcpy(data + 4, &validLength, 2);
		memcpy(data + 6, &crcNum, 2);
		memcpy(data + 8, g_recvSeqFromPC.get_buffer(), validLength);

		int packNum = 1 + (validLength + 7) / 8;
		memcpySequence(m_sendData, data, packNum * 8);

		data_mhal_axi_out_uport->pushPacket(
			data_mhal_axi_out_uport_target_LD, m_sendData);
		COMPDEBUG(3, [RxTxComp_servant::sendDataToMHAL],
			" m_sendData.length():" << m_sendData.length())
	}
}

/**
 * @brief 	Received data from MHAL device and conducting CRC verification,
 * 			then send them to the AudioCodec component.
 *
 * @param[in]	p  an examples of this class
 */
void
RxTxComp_servant::receiveDataFromMHAl()
{
	COMPDEBUG(7, [RxTxComp_servant::receiveDataFromMHAL],
			" enter in ...")

	CORBA::UShort targetLD;
	COMPDEBUG(3, [RxTxComp_servant::receiveDataFromMHAL],
			" enter...")
	data_mhal_axi_in_pport->getData(targetLD, g_recvSeqFromMHAL);
	COMPDEBUG(3, [RxTxComp_servant::receiveDataFromMHAL],
			" leave...")
	if (!checkTargetLD(targetLD)) {
		COMPDEBUG(0, [RxTxComp_servant::receiveDataFromMHAL],
			" LD error.")
		return;
	}

	int curBufLen = m_recvData.length();
	int newPackLen = g_recvSeqFromMHAL.length();
	COMPDEBUG(3, [RxTxComp_servant::receiveDataFromMHAL],
		" curBufLen:" << curBufLen)
	COMPDEBUG(3, [RxTxComp_servant::receiveDataFromMHAL],
		" newPackLen:" << newPackLen)

	m_recvData.length(curBufLen + newPackLen);
	memcpy(m_recvData.get_buffer() + curBufLen,
		g_recvSeqFromMHAL.get_buffer(), newPackLen);

	processData();
}

void
RxTxComp_servant::processData()
{
	unsigned char *recvData = m_recvData.get_buffer();

	if(m_recvData.length() <= 8){
		COMPDEBUG(1, [RxTxComp_servant::processData],
			" m_recvData.length():" << m_recvData.length())
		return;
	}
	if ((*(unsigned int*) recvData) == (*(unsigned int*) SYNCHRO_HEAD)) {
		if (errorRate.checkSum < ULLONG_MAX) {
			errorRate.checkSum += 1;
		} else {
			errorRate.checkErrorNum = 0;
			errorRate.checkSum = 1;
		}

		unsigned short len = *(short*) (recvData + strlen(SYNCHRO_HEAD));

		if (len > TRANSFER_DATA_MAX_LEN || 0 >= len) {
			COMPDEBUG(1, [RxTxComp_servant::processData],
				" len is more than 8000.")
			splitValidData(m_recvData, 8);
			if (adjustData(m_recvData)) {
				processData();
			}
			return;
		}

		unsigned short recvCrc = *(short*) (recvData + strlen(SYNCHRO_HEAD)
				+ sizeof(len));

		int dataLength = m_recvData.length() - 8;
		COMPDEBUG(3, [RxTxComp_servant::processData], " len:" << len)
		COMPDEBUG(3, [RxTxComp_servant::processData], " dataLength:" << dataLength)

		if (dataLength >= len) {
			if (checkCRC(recvData + 8, len, recvCrc)) {
				COMPDEBUG(3, [RxTxComp_servant::processData], " checkCRC success.")
				JTRS::OctetSequence dataSeq;
				memcpySequence(dataSeq, recvData + 8, len);
				data_out_uport->pushPacket(dataSeq);
				COMPDEBUG(3, [RxTxComp_servant::processData], " send finish.")
				splitValidData(m_recvData, 8+len);
				if (adjustData(m_recvData)) {
					processData();
				}
			} else {
				if (errorRate.checkErrorNum < ULLONG_MAX) {
					errorRate.checkErrorNum += 1;
				} else {
					errorRate.checkErrorNum = 1;
					errorRate.checkSum = 1;
				}

				COMPDEBUG(0, [RxTxComp_servant::processData], " checkCRC error.")
				JTRS::OctetSequence dataSeq;
				int offset = 0;
				if (findHeader(m_recvData.get_buffer() + 8, m_recvData.length() - 8, &offset)) {
					memcpySequence(dataSeq, m_recvData.get_buffer() + 8, offset);
					data_out_uport->pushPacket(dataSeq);

					splitValidData(m_recvData, offset + 8);
					processData();
				} else {
					memcpySequence(dataSeq, m_recvData.get_buffer() + 8, len);
					data_out_uport->pushPacket(dataSeq);

					splitValidData(m_recvData, len + 8);
				}
			}
		}
	} else {
		COMPDEBUG(1, [RxTxComp_servant::processData], " head error.")
		if (adjustData(m_recvData)) {
			processData();
		}
	}
}

void
RxTxComp_servant::splitValidData(
		JTRS::OctetSequence &sequence,
		int offset)
{
	if(0 >= offset){
		return;
	}
	int len = sequence.length();

	if (len < offset) {
		return;
	} else if (len == offset) {
		sequence.release();
		sequence.length(len - offset);
	} else {
		memcpySequence(sequence, sequence.get_buffer() + offset, len - offset);
	}
}

bool
RxTxComp_servant::adjustData(
		JTRS::OctetSequence &sequence)
{
	unsigned char *recvData = sequence.get_buffer();
	int length = sequence.length();
	int offset = 0;
	if (findHeader(recvData, length, &offset)) {
		splitValidData(sequence, offset);
		COMPDEBUG(3, [RxTxComp_servant::adjustData],
			" head offset:" << offset)
		return true;
	} else {
		COMPDEBUG(3, [RxTxComp_servant::adjustData],
			" no head offset:" << offset)
		splitValidData(sequence, offset);
	}

	COMPDEBUG(3, [RxTxComp_servant::adjustData],
			" m_recvData:" << m_recvData.length())
	return false;
}

bool
RxTxComp_servant::findHeader(
		unsigned char * recvData,
		int dataLength,
		int * offset)
{
	COMPDEBUG(3, [RxTxComp_servant::findHeader],
		" dataLength:" << dataLength)
	assert(offset);
	if(dataLength < 4){
		*offset = 0;
		COMPDEBUG(3, [RxTxComp_servant::findHeader],
			" offset:" << *offset)
		return false;
	}
	short packNum = dataLength / 8;
	for (int i = 0; i < packNum; i++) {
		if ((*(unsigned int*) (recvData + i * 8))
				== (*(unsigned int*) SYNCHRO_HEAD)) {
			*offset = i * 8;
			return true;
		}
	}

	packNum = dataLength - 4;
	for (int i = 0; i <= packNum; i++) {
		if ((*(unsigned int*) (recvData + i))
				== (*(unsigned int*) SYNCHRO_HEAD)) {
			*offset = i;
			return true;
		}else{
			if(i == packNum){
				*offset = dataLength;
			}
		}
	}
	return false;
}

bool
RxTxComp_servant::checkTargetLD(
		int targetLD)
{
	if (targetLD == data_mhal_axi_in_pport_local_LD){
		return true;
	}
	return false;
}

bool
RxTxComp_servant::checkCRC(
		unsigned char *data,
		int length,
		unsigned short crc)
{
	unsigned short crcNum = get_crc16(data, length, 0);
	if (crcNum == crc) {
		return true;
	}
	return false;
}

void
RxTxComp_servant::memcpySequence(
		JTRS::OctetSequence &sequence,
		const unsigned char *data,
		int length)
{
	if (0 != length && NULL != data) {
		JTRS::OctetSequence dataSeq;
		dataSeq.release();
		dataSeq.length(length);
		memcpy(dataSeq.get_buffer(), data, length);

		sequence.release();
		sequence.length(length);
		memcpy(sequence.get_buffer(), dataSeq.get_buffer(), length);
	}
}

/**
 * @brief	This function used to set Mhal port logical address.
 * 			The logical address is read from component SPD file.
 */
void 
RxTxComp_servant::setMhalPortLD()
{
	FileSystem_impl * fs_i = new FileSystem_impl(m_fsroot.c_str());
	if(!fs_i->exists(m_spdRelPath.c_str())) {
		DEBUG(0, [RxTxComp_servant::setMhalPortLD],
			"SPD file is not existing: " << m_spdRelPath);
	}

	std::string spdAbsPath = m_fsroot + "/" + m_spdRelPath;
	SPDParser spdParser(spdAbsPath);
	size_t pos = m_spdRelPath.find_last_of("/");
	std::string prfRelPath =
		m_spdRelPath.substr(0, pos + 1) + spdParser.getPRFFile();
	if(!fs_i->exists(prfRelPath.c_str())) {
		DEBUG(0, [RxTxComp_servant::setMhalPortLD],
			"prf file is not existing: " << prfRelPath);
	}

	std::string prfAbsPath = m_fsroot + "/" + prfRelPath;
	PRFParser prfParser(prfAbsPath);
	std::vector<PRFProperty *> allProps = *(prfParser.getProperties());
	for(int i = 0; i < allProps.size(); i++) {
		std::string propName = allProps[i]->getName();
		std::vector <std::string> value = allProps[i]->getValue();
		
        if(0 == strcmp(allProps[i]->getName(),"data_mhal_axi_out_target_LD")) {
            if(checkHexFormat(value[0].c_str())) {
                hexConvertToDec(value[0].c_str(),data_mhal_axi_out_uport_target_LD);
            } else {
                data_mhal_axi_out_uport_target_LD = openscaSupport::strings_to_unsigned_short(allProps[i]->getValue());
            }
            continue;
        }
        if(0 == strcmp(allProps[i]->getName(),"data_mhal_axi_in_local_LD")) {
            if(checkHexFormat(value[0].c_str())) {
                hexConvertToDec(value[0].c_str(),data_mhal_axi_in_pport_local_LD);
            } else {
                data_mhal_axi_in_pport_local_LD = openscaSupport::strings_to_unsigned_short(allProps[i]->getValue());
            }
            continue;
        }
	}

	delete fs_i;
}

/**
 * @brief 	The getPort operation provides a mechanism to obtain a specific 
 *         	consumer or producer port, returns the object reference to the 
 *         	named port as stated in the component SCD.
 *
 * @param[in] 	portName-name references to the port user want to get.
 *
 * @return		the references of the port object user want to get.
 *
 * @exception  	The getPort operation shall raise an UnknownPort exception 
 *              if the port name is invalid.
 */
CORBA::Object_ptr 
RxTxComp_servant::getPort(
	const char * portName) 
throw (
	CORBA::SystemException, 
	CF::PortSupplier::UnknownPort)
{
    DEBUG(6, [RxTxComp_servant::getPort], "In getPort.")
		
	CORBA::Object_var _port;
	
	std::string portFullName = "OpenSCA_Domain/" + m_appName + "/" + portName;

    _port = data_out_uport->getPort( portFullName.c_str() );
    if (!CORBA::is_nil(_port))
       return _port._retn();
    _port = data_mhal_axi_out_uport->getPort( portFullName.c_str() );
    if (!CORBA::is_nil(_port))
       return _port._retn();
    if( 0 == strcmp(portName, "control_in") )
        return _this();
    _port = data_in_pport->getPort( portFullName.c_str() );
    if (!CORBA::is_nil(_port))
       return _port._retn();
    _port = data_mhal_axi_in_pport->getPort( portFullName.c_str() );
    if (!CORBA::is_nil(_port))
       return _port._retn();
	
	//don't find any port named by protName
	throw CF::PortSupplier::UnknownPort();
}

/**
 * @brief 	The start operation is provided to command the resource implementing 
 *         	this interface to start internal processing.The start operation puts 
 *         	the resource in an operating condition.
 *
 * @exception 	StartError The start operation shall raise the StartError 
 *             	exception if an error occurs while starting the resource.
 */
void 
RxTxComp_servant::start()
throw (
	CORBA::SystemException, 
	CF::Resource::StartError)
{
	DEBUG(6, [RxTxComp_servant::start], "In start.")
/**************************OPENSCA-USERREGION-BEGIN*******************************/
	if(!m_isStarted){
		m_recvData.length(0);

		data_in_pport->connectSlot(
			boost::bind(&RxTxComp_servant::sendDataToMHAL, this));
		data_mhal_axi_in_pport->connectSlot(
			boost::bind(&RxTxComp_servant::receiveDataFromMHAl, this));

		m_isStarted = true;
	}
/**************************OPENSCA-USERREGION-END*********************************/
}

/**
 * @brief 	The stop operation is provided to command the resource implementing 
 *         	this interface to stop internal processing.
 * 			The stop operation shall disable all current operations and put the 
 * 			resource in a non-operating condition. The stop operation shall not 
 * 			inhibit subsequent configure, query, and start operations.
 *
 * @exception 	StopError The start operation shall raise the StopError exception 
 *             	if an error occurs while stopping the device.
 */
void 
RxTxComp_servant::stop() 
throw (
	CORBA::SystemException, 
	CF::Resource::StopError) 
{  
	DEBUG(6, [RxTxComp_servant::stop], "In stop.")
/**************************OPENSCA-USERREGION-BEGIN*******************************/
#ifdef __SDS_OS_LINUX__
	if(m_isStarted){

		data_in_pport->disconnectSlot();
		data_mhal_axi_in_pport->disconnectSlot();

		m_isStarted = false;
	}
#endif
/**************************OPENSCA-USERREGION-END*********************************/
}

/**
 * @brief 	Release this device object and releavant resource
 *
 * 			The following behavior is in addition to the LifeCycle::releaseObject 
 * 			operation behavior.
 *
 * 			The releaseObject operation shall assign the LOCKED state to the Device 
 * 			adminState attribute, when the Device adminState attribute is UNLOCKED.
 *
 * 			The releaseObject operation shall call the releaseObject operation on 
 * 			all those devices that are contained within the AggregateDevice 
 * 			devices attribute, when this device is a parent device.
 *
 * 			The releaseObject operation shall cause the removal of the device 
 * 			from the Device compositeDevice attribute, when this device is a 
 * 			child device.
 *
 * 			The releaseObject operation shall cause the device to be unavailable 
 * 			and released from the CORBA environment when the Device adminState 
 * 			attribute transitions to LOCKED. The transition to the LOCKED state 
 * 			signifies that the Device usageState attribute is IDLE and, if the 
 * 			device is a parent device, that its child devices have been removed.
 *
 * 			The releaseObject operation shall unregister its device from its 
 * 			device manager.
 *
 * @exception 	The releaseObject operation shall raise the ReleaseError 
 *             	exception when releaseObject is not successful in releasing a 
 *             	logical device due to internal processing errors that occurred
 *			  	within the device being released.
 */
void 
RxTxComp_servant::releaseObject() 
throw (
	CORBA::SystemException, 
	CF::LifeCycle::ReleaseError)
{
	DEBUG(6, [RxTxComp_servant::releaseObject], " In releaseObject.")

#ifdef __SDS_OS_LINUX__
	if(m_isStarted){

		data_in_pport->disconnectSlot();
		data_mhal_axi_in_pport->disconnectSlot();

		errorRate.checkErrorNum =0;
		errorRate.checkSum = 0;

		m_isStarted = false;
	}
#endif

	if (data_out_uport) {
		delete data_out_uport;
		data_out_uport = NULL;
	}

	if (data_mhal_axi_out_uport) {
		delete data_mhal_axi_out_uport;
		data_mhal_axi_out_uport = NULL;
	}

	if (data_in_pport) {
		delete data_in_pport;
		data_in_pport = NULL;
	}

	if (data_mhal_axi_in_pport) {
		delete data_mhal_axi_in_pport;
		data_mhal_axi_in_pport = NULL;
	}
	
	//unbind name from domain
	std::string contextName = 
			"OpenSCA_Domain/" + m_appName + "/" + m_cosNaming;
	CORBA::Object_ptr obj = 
			m_orbWrap->get_object_from_string(contextName.c_str());
	CosNaming::NamingContext_ptr nc = 
			CosNaming::NamingContext::_nil();
	
	try {
		nc = CosNaming::NamingContext::_narrow(obj);
	} catch (CosNaming::NamingContext::InvalidName& ex) {
		DEBUG(0, [RxTxComp_servant::releaseObject],
			" occure InvalidName Exception.")
		throw CF::LifeCycle::ReleaseError();
	} catch (...) {
		DEBUG(0, [RxTxComp_servant::releaseObject],
			" occure Unknown Exception.")
		throw CF::LifeCycle::ReleaseError();
	}
	
	try {
		m_orbWrap->destory_context( nc );
	} catch(CosNaming::NamingContext::NotEmpty) {
		DEBUG(0, [RxTxComp_servant::releaseObject],
			" NamingContext to be destroy is not empty.")
		throw CF::LifeCycle::ReleaseError();
	} catch(...) {
		DEBUG(0, [RxTxComp_servant::releaseObject],
			" Unknown  Exception.")
		throw CF::LifeCycle::ReleaseError();
	}
	
	///ubind name from domain
	try {
		m_orbWrap->unbind_string(contextName.c_str());
	} catch(...) {
		DEBUG(0, [RxTxComp_servant::releaseObject],
			" unbing_string with Unknown  Exception.")
		throw CF::LifeCycle::ReleaseError();
	}
/**************************OPENSCA-USERREGION-BEGIN*******************************/


/**************************OPENSCA-USERREGION-END*********************************/	
#ifdef __SDS_OS_VXWORKS__
	pthread_cond_signal(m_shutdownCond);
#elif defined __SDS_OS_LINUX__
	m_orbWrap->orb->shutdown();
#endif
 }

/**
 * @brief 	The purpose of the initialize operation is to provide a mechanism to 
 *          set a component to a known initial state. For example, data structures 
 *          may be set to initial values, memory may be allocated, hardware 
 *          devices may be configured to some state, etc.
 *
 * @exception 	The initialize operation shall raise an InitializeError 
 *             	exception when an initialization error occurs.
 */
void 
RxTxComp_servant::initialize()
throw (
	CF::LifeCycle::InitializeError, 
	CORBA::SystemException)
{
	DEBUG(6, [RxTxComp_servant::initialize], " In initialize.")
	
    data_out_uport = new StandardInterfaces_i::RealOctet_u(("OpenSCA_Domain/" + m_appName + "/RxTxComp/data_out").c_str());
    data_mhal_axi_out_uport = new StandardInterfaces_i::MHAL_WF_u(("OpenSCA_Domain/" + m_appName + "/RxTxComp/data_mhal_axi_out").c_str());
    data_in_pport = new StandardInterfaces_i::RealOctet_p(("OpenSCA_Domain/" + m_appName + "/RxTxComp/data_in").c_str());
    data_mhal_axi_in_pport = new StandardInterfaces_i::MHAL_WF_p(("OpenSCA_Domain/" + m_appName + "/RxTxComp/data_mhal_axi_in").c_str());
	m_initConfig = false;
	getConfigPropsFromPRF();
/**************************OPENSCA-USERREGION-BEGIN*******************************/
    m_isStarted = false;

    errorRate.checkSum = 0;
    errorRate.checkErrorNum = 0;

/**************************OPENSCA-USERREGION-END*********************************/	
}

void 
RxTxComp_servant::getConfigPropsFromPRF()
{
	FileSystem_impl * fs_i = new FileSystem_impl( m_fsroot.c_str() );
	if(!fs_i->exists(m_spdRelPath.c_str())) {
		DEBUG(0, [RxTxComp_servant::getConfigPropsFromPRF], 
			"SPD file is not existing: " << m_spdRelPath);
	}

	std::string spdAbsPath = m_fsroot + "/" + m_spdRelPath;
	SPDParser spdParser(spdAbsPath);
	size_t pos = m_spdRelPath.find_last_of("/");
	std::string prfRelPath = m_spdRelPath.substr(0, pos + 1) +
								spdParser.getPRFFile();
	if(!fs_i->exists(prfRelPath.c_str())) {
		DEBUG(0, [RxTxComp_servant::getConfigPropsFromPRF], 
			"prf file is not existing: " << prfRelPath);
	}

	std::string prfAbsPath = m_fsroot + "/" + prfRelPath;
	PRFParser prfParser(prfAbsPath);
	std::vector <PRFProperty *> prfConfigProps = 
				*(prfParser.getConfigureProperties());
	m_prfConfigProps.length(prfConfigProps.size());
	for(CORBA::ULong i = 0; i < prfConfigProps.size(); ++i) {
		m_prfConfigProps[i] = *(prfConfigProps[i]->getDataType());
	}
}

/**
 * @brief 	The query operation allows a component to be queried to retrieve its 
 *         	properties.
 * 			The query operation shall return all component properties when the 
 * 			inout parameter configProperties is zero size. The query operation 
 * 			shall return only those id/value pairs specified in the
 * 			configProperties parameter if the parameter is not zero size. Valid 
 * 			properties for the query operation shall be all configure properties 
 * 			(simple properties whose kind element kindtype attribute is 
 * 			"configure" whose mode attribute is "readwrite" or "readonly" and any
 * 			allocation properties with an action value of "external" as referenced 
 * 			in the component SPD.
 *
 * @param[inout]	props	properties need to be queried.
 *
 * @exception The query operation shall raise the CF UnknownProperties exception 
 *            when one or more properties being requested are not known by the 
 *            component.
 */
void 
RxTxComp_servant::query (
	CF::Properties & configProperties)
throw (
	CORBA::SystemException, 
	CF::UnknownProperties)
{
	DEBUG(6, [RxTxComp_servant::query], "In query.")
	pthread_mutex_lock(&m_attrMtx);
	PropertySet_impl::query(configProperties);
/**************************OPENSCA-USERREGION-BEGIN*******************************/

	for(int i=0; i<configProperties.length(); ++i){
		if(0 == strcmp(configProperties[i].id, BLOCK_ERROR_RATE)
				&& 0 != errorRate.checkSum){
			float errorRateValue = errorRate.checkErrorNum*1.0/errorRate.checkSum;
			errorRateValue = processDouble(errorRateValue, 5);
			configProperties[i].value <<= errorRateValue;
		}
	}
/**************************OPENSCA-USERREGION-END*********************************/	
	pthread_mutex_unlock(&m_attrMtx);
}

/**
 * @brief 	The configure operation allows id/value pair configuration properties 
 *         	to be assigned to components implementing this interface.
 *			The configure operation shall assign values to the properties as 
 *			indicated in the input configProperties parameter. Valid properties 
 *			for the configure operation shall at a minimum be the configure 
 *			readwrite and writeonly properties referenced in the component SPD.
 *
 * @param[in]	configProperties properties need to be configured.
 *
 * @exception 	The configure operation shall raise a PartialConfiguration 
 *             	exception when some configuration properties were successfully 
 *             	set and some configuration properties were not successfully set.
 *				The configure operation shall raise an InvalidConfiguration 
 *				exception when a configuration error occurs and no configuration 
 *				properties were successfully set.
 */
void 
RxTxComp_servant::configure(
	const CF::Properties & configProperties)
throw (
	CORBA::SystemException,
	CF::PropertySet::InvalidConfiguration,
	CF::PropertySet::PartialConfiguration)
{
	DEBUG(6, [RxTxComp_servant::configure], "In configure.")

	pthread_mutex_lock(&m_attrMtx);
	
	CF::Properties props = configProperties;

	if(!m_initConfig) {
		CORBA::UShort oldLen = configProperties.length();
		props.length(oldLen + m_prfConfigProps.length());
		for(CORBA::UShort i = 0; i < m_prfConfigProps.length(); ++i) {
			props[oldLen + i] = m_prfConfigProps[i];
		}
		m_initConfig = true;
	}
	
	try {
		PropertySet_impl::configure(props);
	} catch (CF::PropertySet::PartialConfiguration & e) {
		DEBUG(0, [RxTxComp_servant::configure], 
			"partial configuration exception.")
		pthread_mutex_unlock(&m_attrMtx);
		throw e;
	} catch (CF::PropertySet::InvalidConfiguration & e) {
		DEBUG(0, [RxTxComp_servant::configure], 
			"invalid configuration exception.")
		pthread_mutex_unlock(&m_attrMtx);
		throw e;
	} catch (...) {
		DEBUG(0, [RxTxComp_servant::configure], 
			"occur unkown exception when config." )
		pthread_mutex_unlock(&m_attrMtx);
		throw CF::PropertySet::InvalidConfiguration();
	}
/**************************OPENSCA-USERREGION-BEGIN*******************************/


/**************************OPENSCA-USERREGION-END*********************************/
	pthread_mutex_unlock(&m_attrMtx);
}

/**
 * @brief 	The runTest operation shall use the input testId parameter to 
 *         	determine which of its predefined test implementations should be 
 *         	performed.
 *
 * @param[in] 		TestID id of the predefined test implementations should be 
 * 					performed.
 * @parma[inout]	testValues	provide additional information to the 
 * 					implementation-specific test to be run,
 * 					and store the result(s) of the test in the testValues 
 * 					parameter.
 *
 * @Exception		The runTest operation shall raise the UnknownTest exception 
 * 					when there is no underlying test implementation that is 
 * 					associated with the input testId given.
 *					The runTest operation shall raise the CF UnknownProperties 
 *					exception when the input parameter testValues contains any 
 *					CF DataTypes that are not known by the component test
 *					implementation or any values that are out of range for the 
 *					requested test. The exception parameter invalidProperties 
 *					shall contain the invalid testValues properties id(s) that are
 *					not known by the component or the value(s) are out of range.
 */
void 
RxTxComp_servant::runTest(
	CORBA::ULong TestID, 
	CF::Properties & testValues)
throw (
	CF::UnknownProperties, 
	CF::TestableObject::UnknownTest,
	CORBA::SystemException)
{
	DEBUG(6, [RxTxComp_servant::runTest], "In runTest.")
/**************************OPENSCA-USERREGION-BEGIN*******************************/


/**************************OPENSCA-USERREGION-END*********************************/	
}

