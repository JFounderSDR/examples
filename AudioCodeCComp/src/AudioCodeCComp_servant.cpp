/***************************************************************************//**
* @file     AudioCodeCComp_servant.cpp
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

#include "../include/AudioCodeCComp_servant.h"
#include "../include/libu1alg.h"
#include "../include/g72x.h"
#include "FileSystem_impl.h"
#include "SPDParser.h"
#include "utils.h"
#ifdef __SDS_OS_VXWORKS__
#include <usrLib.h>
#endif

#define DATA_LENGTH		8000	// Maximum length of transmitted data
#define AUDIOHEAD 		12		// Length of data header receiving data from PC

JTRS::OctetSequence g_recvSeqFromPC(20000);
JTRS::OctetSequence g_sendSeqFromCRC(20000);
JTRS::OctetSequence g_recvSeqFromCRC(20000);

AudioCodeCComp_servant::AudioCodeCComp_servant()
{
}

/**
 * @brief	AudioCodeCComp_servant Constructor.
 * 
 * @param[in] _id		component name.
 * @param[in] _sftwf1 	component SPD file name.
 * @param[in] _fsroot	file system root path.
 */
#ifdef __SDS_OS_VXWORKS__
AudioCodeCComp_servant::AudioCodeCComp_servant(
	const char * _id,
	const char * _cosNaming,
	const char * _appName,
	const char * _sftwfl,
	const char * _fsroot,
	pthread_cond_t * _shutdownCond):
Resource_impl(_id)
{
	DEBUG(6, [AudioCodeCComp_servant], "In constructor.")
	
	
	m_orbWrap = new openscaSupport::ORB_Wrap::ORB_Wrap();
	m_appName = _appName;
	m_cosNaming = _cosNaming;
	m_spdRelPath = _sftwfl;
	m_fsroot = _fsroot;
	m_shutdownCond = _shutdownCond;
	pthread_mutex_init(&m_attrMtx, NULL);


	if(m_mhalPortNames.size() > 0) {
		setMhalPortLD();
    }
/**************************OPENSCA-USERREGION-BEGIN*******************************/


/**************************OPENSCA-USERREGION-END*********************************/
}

#elif defined __SDS_OS_LINUX__

AudioCodeCComp_servant::AudioCodeCComp_servant(
	const char * _id,
	const char * _cosNaming,
	const char * _appName,
	const char * _sftwfl,
	const char * _fsroot):
Resource_impl(_id)
{
	DEBUG(6, [AudioCodeCComp_servant], "In constructor.")
	
	
	m_orbWrap = new openscaSupport::ORB_Wrap();
	m_appName = _appName;
	m_cosNaming = _cosNaming;
	m_spdRelPath = _sftwfl;
	m_fsroot = _fsroot;
	pthread_mutex_init(&m_attrMtx, NULL);


	if(m_mhalPortNames.size() > 0) {
		setMhalPortLD();
    }
/**************************OPENSCA-USERREGION-BEGIN*******************************/
	user_data_out_uport = NULL;
	data_out_uport = NULL;
	user_data_in_pport = NULL;
	data_in_pport = NULL;
/**************************OPENSCA-USERREGION-END*********************************/
}
#endif

AudioCodeCComp_servant::~AudioCodeCComp_servant()
{
	DEBUG(6, [AudioCodeCComp_servant], "in destructor")
/**************************OPENSCA-USERREGION-BEGIN*******************************/


/**************************OPENSCA-USERREGION-END*********************************/
}

/**
 * @brief 	Received data from PC and compressed data, then send them to
 * 			the CRC component.
 * 			The data encoding format to be compressed is PCM, Sampling bit
 * 			depth is 16 bits and mono channel. The data 2:1 is compressed.
 *
 * @param[in]	p  an examples of this class
 */
void
AudioCodeCComp_servant::sendDataToCRC()
{
	COMPDEBUG(7, [AudioCodeCComp_servant::sendDataToCRC], " enter in ...")
	
	user_data_in_pport->getData(g_recvSeqFromPC);
	static unsigned char audioHeader[AUDIOHEAD];
	memset(audioHeader, 0, AUDIOHEAD);
	memcpy(audioHeader, g_recvSeqFromPC.get_buffer(), AUDIOHEAD);

	g_sendSeqFromCRC.length(g_recvSeqFromPC.length() - AUDIOHEAD);
	for (CORBA::ULong i = 0; i < g_sendSeqFromCRC.length(); ++i) {
		g_sendSeqFromCRC[i] = g_recvSeqFromPC[AUDIOHEAD + i];
	}
	int actualLength = g_sendSeqFromCRC.length();
	COMPDEBUG(3, [AudioCodeCComp_servant::sendDataToCRC],
		" g_sendSeqFromCRC.length():" << actualLength)

	JTRS::OctetSequence enCodeSeq;
	enCodeSeq.length(DATA_LENGTH);
	int enCodecLen = u1lag_audio_codec_encode(m_encodecHandle, (unsigned char*)enCodeSeq.get_buffer(),
		(short*)g_sendSeqFromCRC.get_buffer(), actualLength);
	COMPDEBUG(3, [AudioCodeCComp_servant::sendDataToCRC],
		" enCodeSeq.length():" << enCodeSeq.length())

	JTRS::OctetSequence sendSeq;
	sendSeq.length(enCodecLen + AUDIOHEAD);
	memcpy(sendSeq.get_buffer(), audioHeader, AUDIOHEAD);
	memcpy(sendSeq.get_buffer() + AUDIOHEAD, enCodeSeq.get_buffer(), 
		enCodecLen);

	data_out_uport->pushPacket(sendSeq);
}

/**
 * @brief 	Received data from CRC component and decompressed data, then send them to
 * 			the PC.
 * 			The data encoding format to be decompressed is PCM, Sampling bit
 * 			depth is 16 bits and mono channel. The data 1:2 is decompressed.
 *
 * @param[in]	p  an examples of this class
 */
void
AudioCodeCComp_servant::receiveDataFromCRC()
{
	COMPDEBUG(7, [AudioCodeCComp_servant::receiveDataFromCRC],
			" enter in ...")

	m_recvData.length(DATA_LENGTH);
	memset(m_recvData.get_buffer(), 0, DATA_LENGTH);

	data_in_pport->getData(g_recvSeqFromCRC);

	unsigned char audioHeader[AUDIOHEAD];
	memset(audioHeader, 0, AUDIOHEAD);
	memcpy(audioHeader, g_recvSeqFromCRC.get_buffer(), AUDIOHEAD);

	int actualLength = g_recvSeqFromCRC.length() - AUDIOHEAD;
	COMPDEBUG(3, [AudioCodeCComp_servant::receiveDataFromCRC],
		" actualLength:" << actualLength)

	JTRS::OctetSequence deCodeSeq;
	deCodeSeq.length(DATA_LENGTH);

	char recvData[actualLength];
	memset(recvData, 0, actualLength);
	memcpy(recvData, g_recvSeqFromCRC.get_buffer() + AUDIOHEAD, actualLength);

	int decodeLen = u1lag_audio_codec_decode(m_decodecHandle, (unsigned char*)recvData,
		(short*)deCodeSeq.get_buffer(), actualLength);
	COMPDEBUG(3, [AudioCodeCComp_servant::receiveDataFromCRC],
		" deCodeSeq.length():" << deCodeSeq.length())

	JTRS::OctetSequence deCodeSeqWithHeader;
	deCodeSeqWithHeader.length(AUDIOHEAD + decodeLen);
	memcpy(deCodeSeqWithHeader.get_buffer(), audioHeader, AUDIOHEAD);
	memcpy(deCodeSeqWithHeader.get_buffer() + AUDIOHEAD, deCodeSeq.get_buffer(), 
		decodeLen);
	COMPDEBUG(3, [AudioCodeCComp_servant::receiveDataFromCRC],
		" deCodeSeqWithHeader.length():" << deCodeSeqWithHeader.length())

	user_data_out_uport->pushPacket(deCodeSeqWithHeader);

	COMPDEBUG(3, [AudioCodeCComp_servant::receiveDataFromCRC],
		" send finish." << deCodeSeqWithHeader.length())
}

/**
 * @brief	This function used to set Mhal port logical address.
 * 			The logical address is read from component SPD file.
 */
void 
AudioCodeCComp_servant::setMhalPortLD()
{
	FileSystem_impl * fs_i = new FileSystem_impl(m_fsroot.c_str());
	if(!fs_i->exists(m_spdRelPath.c_str())) {
		DEBUG(0, [AudioCodeCComp_servant::setMhalPortLD],
			"SPD file is not existing: " << m_spdRelPath);
	}

	std::string spdAbsPath = m_fsroot + "/" + m_spdRelPath;
	SPDParser spdParser(spdAbsPath);
	size_t pos = m_spdRelPath.find_last_of("/");
	std::string prfRelPath =
		m_spdRelPath.substr(0, pos + 1) + spdParser.getPRFFile();
	if(!fs_i->exists(prfRelPath.c_str())) {
		DEBUG(0, [AudioCodeCComp_servant::setMhalPortLD],
			"prf file is not existing: " << prfRelPath);
	}

	std::string prfAbsPath = m_fsroot + "/" + prfRelPath;
	PRFParser prfParser(prfAbsPath);
	std::vector<PRFProperty *> allProps = *(prfParser.getProperties());
	for(int i = 0; i < allProps.size(); i++) {
		std::string propName = allProps[i]->getName();
		std::vector <std::string> value = allProps[i]->getValue();
		
        //There is no mhal ports.
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
AudioCodeCComp_servant::getPort(
	const char * portName) 
throw (
	CORBA::SystemException, 
	CF::PortSupplier::UnknownPort)
{
    DEBUG(6, [AudioCodeCComp_servant::getPort], "In getPort.")
		
	CORBA::Object_var _port;
	
	std::string portFullName = "OpenSCA_Domain/" + m_appName + "/" + portName;

    _port = user_data_out_uport->getPort( portFullName.c_str() );
    if (!CORBA::is_nil(_port))
       return _port._retn();
    _port = data_out_uport->getPort( portFullName.c_str() );
    if (!CORBA::is_nil(_port))
       return _port._retn();
    if( 0 == strcmp(portName, "control_in") )
        return _this();
    _port = user_data_in_pport->getPort( portFullName.c_str() );
    if (!CORBA::is_nil(_port))
       return _port._retn();
    _port = data_in_pport->getPort( portFullName.c_str() );
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
AudioCodeCComp_servant::start()
throw (
	CORBA::SystemException, 
	CF::Resource::StartError)
{
	DEBUG(6, [AudioCodeCComp_servant::start], "In start.")
/**************************OPENSCA-USERREGION-BEGIN*******************************/
#ifdef __SDS_OS_LINUX__
	if(!m_isStarted){
		user_data_in_pport->connectSlot(
				boost::bind(&AudioCodeCComp_servant::sendDataToCRC, this));
		data_in_pport->connectSlot(
				boost::bind(&AudioCodeCComp_servant::receiveDataFromCRC, this));

		m_isStarted = true;
	}
#endif
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
AudioCodeCComp_servant::stop() 
throw (
	CORBA::SystemException, 
	CF::Resource::StopError) 
{  
	DEBUG(6, [AudioCodeCComp_servant::stop], "In stop.")
/**************************OPENSCA-USERREGION-BEGIN*******************************/
	if(m_isStarted){

		user_data_in_pport->disconnectSlot();
		data_in_pport->disconnectSlot();

		m_isStarted = false;
	}
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
AudioCodeCComp_servant::releaseObject() 
throw (
	CORBA::SystemException, 
	CF::LifeCycle::ReleaseError)
{
	DEBUG(6, [AudioCodeCComp_servant::releaseObject], " In releaseObject.")

#ifdef __SDS_OS_LINUX__
	stop();
#endif
	
    if(user_data_out_uport) {
        delete user_data_out_uport;
        user_data_out_uport = NULL;
    }
    if(data_out_uport) {
        delete data_out_uport;
        data_out_uport = NULL;
    }
    if(user_data_in_pport) {
        delete user_data_in_pport;
        user_data_in_pport = NULL;
    }
    if(data_in_pport) {
        delete data_in_pport;
        data_in_pport = NULL;
    }

    if(NULL != m_encodecHandle)
		u1lag_audio_codec_delete(m_encodecHandle);
	if(NULL != m_decodecHandle)
		u1lag_audio_codec_delete(m_decodecHandle);
	
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
		DEBUG(0, [AudioCodeCComp_servant::releaseObject],
			" occure InvalidName Exception.")
		throw CF::LifeCycle::ReleaseError();
	} catch (...) {
		DEBUG(0, [AudioCodeCComp_servant::releaseObject],
			" occure Unknown Exception.")
		throw CF::LifeCycle::ReleaseError();
	}
	
	try {
		m_orbWrap->destory_context( nc );
	} catch(CosNaming::NamingContext::NotEmpty) {
		DEBUG(0, [AudioCodeCComp_servant::releaseObject],
			" NamingContext to be destroy is not empty.")
		throw CF::LifeCycle::ReleaseError();
	} catch(...) {
		DEBUG(0, [AudioCodeCComp_servant::releaseObject],
			" Unknown  Exception.")
		throw CF::LifeCycle::ReleaseError();
	}
	
	///ubind name from domain
	try {
		m_orbWrap->unbind_string(contextName.c_str());
	} catch(...) {
		DEBUG(0, [AudioCodeCComp_servant::releaseObject],
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
AudioCodeCComp_servant::initialize()
throw (
	CF::LifeCycle::InitializeError, 
	CORBA::SystemException)
{
	DEBUG(6, [AudioCodeCComp_servant::initialize], " In initialize.")
	
    user_data_out_uport = new StandardInterfaces_i::RealOctet_u(
    	("OpenSCA_Domain/" + m_appName + "/AudioCodeCComp/user_data_out").c_str());
    data_out_uport = new StandardInterfaces_i::RealOctet_u(
    	("OpenSCA_Domain/" + m_appName + "/AudioCodeCComp/data_out").c_str());
    user_data_in_pport = new StandardInterfaces_i::RealOctet_p(
    	("OpenSCA_Domain/" + m_appName + "/AudioCodeCComp/user_data_in").c_str());
    data_in_pport = new StandardInterfaces_i::RealOctet_p(
    	("OpenSCA_Domain/" + m_appName + "/AudioCodeCComp/data_in").c_str());
	m_initConfig = false;
	getConfigPropsFromPRF();
/**************************OPENSCA-USERREGION-BEGIN*******************************/
	m_encodecHandle = NULL;
	m_decodecHandle = NULL;

	m_audioCodecType = 0;

	m_isStarted = false;
/**************************OPENSCA-USERREGION-END*********************************/	
}

void 
AudioCodeCComp_servant::getConfigPropsFromPRF()
{
	FileSystem_impl * fs_i = new FileSystem_impl( m_fsroot.c_str() );
	if(!fs_i->exists(m_spdRelPath.c_str())) {
		DEBUG(0, [AudioCodeCComp_servant::getConfigPropsFromPRF], 
			"SPD file is not existing: " << m_spdRelPath);
	}

	std::string spdAbsPath = m_fsroot + "/" + m_spdRelPath;
	SPDParser spdParser(spdAbsPath);
	size_t pos = m_spdRelPath.find_last_of("/");
	std::string prfRelPath = m_spdRelPath.substr(0, pos + 1) +
								spdParser.getPRFFile();
	if(!fs_i->exists(prfRelPath.c_str())) {
		DEBUG(0, [AudioCodeCComp_servant::getConfigPropsFromPRF], 
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
AudioCodeCComp_servant::query (
	CF::Properties & configProperties)
throw (
	CORBA::SystemException, 
	CF::UnknownProperties)
{
	DEBUG(6, [AudioCodeCComp_servant::query], "In query.")
	pthread_mutex_lock(&m_attrMtx);
	PropertySet_impl::query(configProperties);
/**************************OPENSCA-USERREGION-BEGIN*******************************/


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
AudioCodeCComp_servant::configure(
	const CF::Properties & configProperties)
throw (
	CORBA::SystemException,
	CF::PropertySet::InvalidConfiguration,
	CF::PropertySet::PartialConfiguration)
{
	DEBUG(7, [AudioCodeCComp_servant::configure], "In configure.")

	pthread_mutex_lock(&m_attrMtx);
	CF::Properties props = configProperties;
	if(!m_initConfig) {
		CORBA::Short oldLen = configProperties.length();
		props.length(oldLen + m_prfConfigProps.length());
		for(CORBA::UShort i = 0; i < m_prfConfigProps.length(); ++i) {
			props[oldLen + i] = m_prfConfigProps[i];
		}
		m_initConfig = true;
	}

	int propLen = props.length();
	COMPDEBUG(3, [AudioCodeCComp_servant::configure],
		" propLen:" << propLen)
	if(1 == propLen){
		if(0 == strcmp(props[0].id, AUDIO_COMPRESSION_RATIO)){
			CORBA::UShort audioCodecType;
			props[0].value >>= audioCodecType;
			COMPDEBUG(3, [AudioCodeCComp_servant::configure],
				" audioCodecType:" << audioCodecType)

			if(audioCodecType != 1 && audioCodecType != 2){
				COMPDEBUG(0, [AudioCodeCComp_servant::configure],
					" audioCodecType error.")
				pthread_mutex_unlock(&m_attrMtx);
				char *errText = "参数超出范围1或2，请重新设置！";
				throw CF::PropertySet::InvalidConfiguration(
					errText, props);
			}	
		}
	}
	
	try {
		PropertySet_impl::configure(props);
	} catch (CF::PropertySet::PartialConfiguration & e) {
		DEBUG(0, [AudioCodeCComp_servant::configure], 
			"partial configuration exception.")
		pthread_mutex_unlock(&m_attrMtx);
		throw e;
	} catch (CF::PropertySet::InvalidConfiguration & e) {
		DEBUG(0, [AudioCodeCComp_servant::configure], 
			"invalid configuration exception.")
		pthread_mutex_unlock(&m_attrMtx);
		throw e;
	} catch (...) {
		DEBUG(0, [AudioCodeCComp_servant::configure], 
			"occur unkown exception when config." )
		pthread_mutex_unlock(&m_attrMtx);
		throw CF::PropertySet::InvalidConfiguration();
	}
/**************************OPENSCA-USERREGION-BEGIN*******************************/
	
	for(CORBA::UShort i=0; i<propLen; ++i){
		if(0 == strcmp(props[i].id, AUDIO_COMPRESSION_RATIO)){
			CORBA::UShort audioCodecType;
			props[i].value >>= audioCodecType;
			m_audioCodecType = audioCodecType;
			COMPDEBUG(3, [AudioCodeCComp_servant::configure],
				" m_audioCodecType:" << m_audioCodecType)
		}
	}

	if(NULL != m_encodecHandle){
		u1lag_audio_codec_delete(m_encodecHandle);
	}

	if(NULL != m_decodecHandle){
		u1lag_audio_codec_delete(m_decodecHandle);
	}

	m_encodecHandle = u1lag_audio_codec_create( \
		(u1alg_audio_codec_type_t)m_audioCodecType);
	m_decodecHandle = u1lag_audio_codec_create( \
		(u1alg_audio_codec_type_t)m_audioCodecType);

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
AudioCodeCComp_servant::runTest(
	CORBA::ULong TestID, 
	CF::Properties & testValues)
throw (
	CF::UnknownProperties, 
	CF::TestableObject::UnknownTest,
	CORBA::SystemException)
{
	DEBUG(6, [AudioCodeCComp_servant::runTest], "In runTest.")
/**************************OPENSCA-USERREGION-BEGIN*******************************/


/**************************OPENSCA-USERREGION-END*********************************/	
}

