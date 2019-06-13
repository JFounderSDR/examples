/***************************************************************************//**
* @file     MsgTrans_Ctroller_servant.cpp
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

#include "../include/MsgTrans_Ctroller_servant.h"
#include "FileSystem_impl.h"
#include "SPDParser.h"
#include "utils.h"
#ifdef __SDS_OS_VXWORKS__
#include <usrLib.h>
#endif
MsgTrans_Ctroller_servant::MsgTrans_Ctroller_servant()
{
}

/**
 * @brief	MsgTrans_Ctroller_servant Constructor.
 * 
 * @param[in] _id		component's name.
 * @param[in] _sftwf1 	component's SPD file name.
 * @param[in] _fsroot	file system root path.
 */
#ifdef __SDS_OS_VXWORKS__
MsgTrans_Ctroller_servant::MsgTrans_Ctroller_servant(
	const char * _id,
	const char * _cosNaming,
	const char * _appName,
	const char * _sftwfl,
	const char * _fsroot,
	pthread_cond_t * _shutdownCond):
Resource_impl(_id)
{
	DEBUG(6, [MsgTrans_Ctroller_servant], "In constructor.")
	
	
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

MsgTrans_Ctroller_servant::MsgTrans_Ctroller_servant(
	const char * _id,
	const char * _cosNaming,
	const char * _appName,
	const char * _sftwfl,
	const char * _fsroot):
Resource_impl(_id)
{
	DEBUG(6, [MsgTrans_Ctroller_servant], "In constructor.")
	
	
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


/**************************OPENSCA-USERREGION-END*********************************/
}
#endif

MsgTrans_Ctroller_servant::~MsgTrans_Ctroller_servant()
{
	DEBUG(6, [MsgTrans_Ctroller_servant], "in destructor")
/**************************OPENSCA-USERREGION-BEGIN*******************************/


/**************************OPENSCA-USERREGION-END*********************************/
}

/**
 * @brief	This function used to set Mhal port's logical address.
 * 			The logical address is read from component's SPD file.
 */
void 
MsgTrans_Ctroller_servant::setMhalPortLD()
{
	FileSystem_impl * fs_i = new FileSystem_impl(m_fsroot.c_str());
	if(!fs_i->exists(m_spdRelPath.c_str())) {
		DEBUG(0, [MsgTrans_Ctroller_servant::setMhalPortLD],
			"SPD file is not existing: " << m_spdRelPath);
	}

	std::string spdAbsPath = m_fsroot + "/" + m_spdRelPath;
	SPDParser spdParser(spdAbsPath);
	size_t pos = m_spdRelPath.find_last_of("/");
	std::string prfRelPath =
		m_spdRelPath.substr(0, pos + 1) + spdParser.getPRFFile();
	if(!fs_i->exists(prfRelPath.c_str())) {
		DEBUG(0, [MsgTrans_Ctroller_servant::setMhalPortLD],
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
 *         	named port as stated in the component's SCD.
 *
 * @param[in] 	portName-name references to the port user want to get.
 *
 * @return		the references of the port object user want to get.
 *
 * @exception  	The getPort operation shall raise an UnknownPort exception 
 *              if the port name is invalid.
 */
CORBA::Object_ptr 
MsgTrans_Ctroller_servant::getPort(
	const char * portName) 
throw (
	CORBA::SystemException, 
	CF::PortSupplier::UnknownPort)
{
    DEBUG(6, [MsgTrans_Ctroller_servant::getPort], "In getPort.")
		
	CORBA::Object_var _port;
	
	std::string portFullName = "OpenSCA_Domain/" + m_appName + "/" + portName;
	DEBUG(5, [MsgTrans_Ctroller_servant::getPort], " portName:" << portName)
	if ((0 == strcmp(portName, "CRCComp/data_in")) ||
		(0 == strcmp(portName, "CRCComp/data_out"))) {
		std::vector <CF::Resource_ptr> comps;
		comps = control_out_uport->getProvidesPorts();
		_port = comps[0]->getPort(portName);
		return _port._retn();
	}

    _port = control_out_uport->getPort( portFullName.c_str());
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
MsgTrans_Ctroller_servant::start()
throw (
	CORBA::SystemException, 
	CF::Resource::StartError)
{
	DEBUG(6, [MsgTrans_Ctroller_servant::start], "In start.")
/**************************OPENSCA-USERREGION-BEGIN*******************************/
#ifdef __SDS_OS_LINUX__
	if(!m_isStarted){
		m_mhalDev->start();

		std::vector <CF::Resource_ptr> comps;
		comps = control_out_uport->getProvidesPorts();
		DEBUG(7, [MsgTrans_Ctroller_servant::start],
			" comps.size(): " << comps.size());

		for(int i = 0; i < comps.size(); i++) {
			if(CORBA::is_nil(comps[i])) {
				DEBUG(0, [MsgTrans_Ctroller_servant::start],
					" get component failed. ");
				break;
			}
			comps[i]->start();
		}

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
MsgTrans_Ctroller_servant::stop() 
throw (
	CORBA::SystemException, 
	CF::Resource::StopError) 
{  
	DEBUG(6, [MsgTrans_Ctroller_servant::stop], "In stop.")
/**************************OPENSCA-USERREGION-BEGIN*******************************/
#ifdef __SDS_OS_LINUX__
	if(m_isStarted){
		try{
			m_mhalDev->stop();
		}catch(...){
			DEBUG(0, [MsgTrans_Ctroller_servant::stop], " stop error.")
		}

		std::vector <CF::Resource_ptr> comps;
		comps = control_out_uport->getProvidesPorts();
		DEBUG(7, MsgTrans_Ctroller_servant::stop,
			" comps.size(): " << comps.size());

		for(int i = 0; i < comps.size(); i++) {
			if(CORBA::is_nil(comps[i])) {
				DEBUG(0, [MsgTrans_Ctroller_servant::stop],
					" get component failed. ");
				break;
			}

			DEBUG(7, [MsgTrans_Ctroller_servant::stop], " call stop. ");
			comps[i]->stop();
		}

		m_isStarted = false;
	}
#endif
	DEBUG(7, [MsgTrans_Ctroller_servant::stop], " leave.")
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
MsgTrans_Ctroller_servant::releaseObject() 
throw (
	CORBA::SystemException, 
	CF::LifeCycle::ReleaseError)
{
	DEBUG(6, [MsgTrans_Ctroller_servant::releaseObject], " In releaseObject.")
	
	if (m_isStarted) {
		stop();
	}

	if (control_out_uport) {
		delete control_out_uport;
		control_out_uport = NULL;
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
		DEBUG(0, [MsgTrans_Ctroller_servant::releaseObject],
			" occure InvalidName Exception.")
		throw CF::LifeCycle::ReleaseError();
	} catch (...) {
		DEBUG(0, [MsgTrans_Ctroller_servant::releaseObject],
			" occure Unknown Exception.")
		throw CF::LifeCycle::ReleaseError();
	}
	
	try {
		m_orbWrap->destory_context( nc );
	} catch(CosNaming::NamingContext::NotEmpty) {
		DEBUG(0, [MsgTrans_Ctroller_servant::releaseObject],
			" NamingContext to be destroy is not empty.")
		throw CF::LifeCycle::ReleaseError();
	} catch(...) {
		DEBUG(0, [MsgTrans_Ctroller_servant::releaseObject],
			" Unknown  Exception.")
		throw CF::LifeCycle::ReleaseError();
	}
	
	///ubind name from domain
	try {
		m_orbWrap->unbind_string(contextName.c_str());
	} catch(...) {
		DEBUG(0, [MsgTrans_Ctroller_servant::releaseObject],
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
MsgTrans_Ctroller_servant::initialize()
throw (
	CF::LifeCycle::InitializeError, 
	CORBA::SystemException)
{
	DEBUG(6, [MsgTrans_Ctroller_servant::initialize], " In initialize.")
	
    control_out_uport = new StandardInterfaces_i::Resource_u(
    	("OpenSCA_Domain/" + m_appName + "/MsgTrans_Ctroller/control_out").c_str());
	m_initConfig = false;
	getConfigPropsFromPRF();
/**************************OPENSCA-USERREGION-BEGIN*******************************/
	/// getting the SPM_Zynq7045_PS device through the naming service
	/// and then getting the Mhal subdevice.
	std::string contextName = 
		"OpenSCA_Domain/Single_Node/Device_Manager/U1_Zynq7035_PS";
	CORBA::Object_ptr obj = 
		m_orbWrap->get_object_from_string(contextName.c_str());
	if (CORBA::is_nil(obj)) {
		DEBUG(0, [MsgTrans_Ctroller_servant::initialize], " obtain obj fail.")
		return;
	}

	CF::ExecutableDevice_var dev;
	try {
		dev = CF::ExecutableDevice::_narrow(obj);
	} catch (...) {
		DEBUG(0, [MsgTrans_Ctroller_servant::initialize],
			"\"CF::ExecutableDevice::_narrow\" failed with Unknown Exception.")
	}

	try {
		CF::DeviceSequence_var deviceSeq = dev->compositeDevice()->devices();
		for (CORBA::ULong i = 0; i < deviceSeq->length(); ++i) {
			if (0 == strcmp("MHAL_Device", deviceSeq[i]->label())) {
				m_mhalDev = CF::Device::_duplicate(deviceSeq[i]);
				break;
			}
		}
	} catch (...) {
		DEBUG(0, [MsgTrans_Ctroller_servant::initialize],
			" get MHAL device failed.")
	}

	if (CORBA::is_nil(m_mhalDev)) {
		DEBUG(0, [MsgTrans_Ctroller_servant::initialize],
			" get child device failed.")
		return;
	}

	m_isStarted = false;
/**************************OPENSCA-USERREGION-END*********************************/	
}

void 
MsgTrans_Ctroller_servant::getConfigPropsFromPRF()
{
	FileSystem_impl * fs_i = new FileSystem_impl( m_fsroot.c_str() );
	if(!fs_i->exists(m_spdRelPath.c_str())) {
		DEBUG(0, [MsgTrans_Ctroller_servant::getConfigPropsFromPRF], 
			"SPD file is not existing: " << m_spdRelPath);
	}

	std::string spdAbsPath = m_fsroot + "/" + m_spdRelPath;
	SPDParser spdParser(spdAbsPath);
	size_t pos = m_spdRelPath.find_last_of("/");
	std::string prfRelPath = m_spdRelPath.substr(0, pos + 1) +
								spdParser.getPRFFile();
	if(!fs_i->exists(prfRelPath.c_str())) {
		DEBUG(0, [MsgTrans_Ctroller_servant::getConfigPropsFromPRF], 
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
 * 			(simple properties whose kind element's kindtype attribute is 
 * 			"configure" whose mode attribute is "readwrite" or "readonly" and any
 * 			allocation properties with an action value of "external" as referenced 
 * 			in the component's SPD.
 *
 * @param[inout]	props	properties need to be queried.
 *
 * @exception The query operation shall raise the CF UnknownProperties exception 
 *            when one or more properties being requested are not known by the 
 *            component.
 */
void 
MsgTrans_Ctroller_servant::query (
	CF::Properties & configProperties)
throw (
	CORBA::SystemException, 
	CF::UnknownProperties)
{
	DEBUG(6, [MsgTrans_Ctroller_servant::query], "In query.")
	pthread_mutex_lock(&m_attrMtx);
/**************************OPENSCA-USERREGION-BEGIN*******************************/
	CF::Properties totalProperties;
	totalProperties.length(0);

	std::vector <CF::Resource_ptr> comps;
	comps = control_out_uport->getProvidesPorts();
	CORBA::Short compLen = comps.size();
	DEBUG(6, [MsgTrans_Ctroller_servant::query], " compLen: " << compLen);

	if (0 == configProperties.length()){
		for(CORBA::Short i = 0; i < compLen; i++) {
			if(CORBA::is_nil(comps[i])) {
				DEBUG(0, [MsgTrans_Ctroller_servant::start], " get component failed. ");
				pthread_mutex_unlock(&m_attrMtx);
				break;
			}
			CF::Properties properties;
			comps[i]->query(properties);

			CORBA::Short len = properties.length();
			DEBUG(6, [MsgTrans_Ctroller_servant::query], " len: " << len);
			CORBA::Short totalLen = totalProperties.length();
			DEBUG(6, [MsgTrans_Ctroller_servant::query], " totalLen: " << totalLen);
			totalProperties.length(totalLen + len);
			for(CORBA::Short i = 0; i < len; ++i) {
				totalProperties[totalLen + i] = properties[i];
			}
		}

		PropertySet_impl::query(configProperties);
		CORBA::UShort len = configProperties.length();
		CORBA::UShort oldLen = totalProperties.length();
		totalProperties.length(oldLen + len);
		for(CORBA::UShort i = 0; i < len; ++i) {
			totalProperties[oldLen + i] = configProperties[i];
		}

		configProperties.length(0);
		configProperties = totalProperties;
	} 

	if(1 == configProperties.length()){
		if(0 == strcmp(configProperties[0].id, CONNECTION) || 
			0 == strcmp(configProperties[0].id, START_STATUS) ||
			0 == strcmp(configProperties[0].id, BUSINESS_TYPE)){

			PropertySet_impl::query(configProperties);

		} else if (0 == strcmp(configProperties[0].id, BLOCK_ERROR_RATE) || 
			0 == strcmp(configProperties[0].id, LOCAL_LD) ||
			0 == strcmp(configProperties[0].id, TARGET_LD)) {

			for(int i = 0; i < compLen; i++) {
				if(CORBA::is_nil(comps[i])) {
					DEBUG(0, [MsgTrans_Ctroller_servant::start], " get component failed. ")
					pthread_mutex_unlock(&m_attrMtx);
					break;
				}
				if(0 == strcmp(comps[i]->identifier(), CRCCOMP_ID)){
					comps[i]->query(configProperties);
				}
			}
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
 *			readwrite and writeonly properties referenced in the component's SPD.
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
MsgTrans_Ctroller_servant::configure(
	const CF::Properties & configProperties)
throw (
	CORBA::SystemException,
	CF::PropertySet::InvalidConfiguration,
	CF::PropertySet::PartialConfiguration)
{
	DEBUG(6, [MsgTrans_Ctroller_servant::configure], "In configure.")

	pthread_mutex_lock(&m_attrMtx);
	
	CF::Properties props = configProperties;

	if(props.length() > 0){
		if( 0 == strcmp( props[0].id, 
				"DCE:1b26d09b-472d-4515-b872-4063e9d39062:1") ||
			0 == strcmp( props[0].id, 
				"DCE:96acf915-e92f-4065-8050-1982a9bf790b:1")) {

			std::vector <CF::Resource_ptr> comps;
			comps = control_out_uport->getProvidesPorts();

			DEBUG(7, [MsgTrans_Ctroller_servant::configure],
				" comps.size(): " << comps.size());

			for(int i = 0; i < comps.size(); i++) {
				if(CORBA::is_nil(comps[i])) {
					DEBUG(1, [MsgTrans_Ctroller_servant::configure],
						" get component failed. ");
					break;
				}

				if( 0 == strcmp(comps[i]->identifier(),
						"DCE:36d656f4-023f-45ae-8575-857f49870d57:1")) {
					CF::Properties crcProp;
					crcProp.length(1);
					crcProp[0] = props[0];
					comps[i]->configure(crcProp);
				}
			}
		}
	}

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
	} catch (CF::PropertySet::PartialConfiguration) {
		DEBUG(0, [MsgTrans_Ctroller_servant::configure], 
			"partial configuration exception.")
		pthread_mutex_unlock(&m_attrMtx);
		throw CF::PropertySet::PartialConfiguration();
	} catch (CF::PropertySet::InvalidConfiguration) {
		DEBUG(0, [MsgTrans_Ctroller_servant::configure], 
			"invalid configuration exception.")
		pthread_mutex_unlock(&m_attrMtx);
		throw CF::PropertySet::InvalidConfiguration();
	} catch (...) {
		DEBUG(0, [MsgTrans_Ctroller_servant::configure], 
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
 *					CF DataTypes that are not known by the component's test
 *					implementation or any values that are out of range for the 
 *					requested test. The exception parameter invalidProperties 
 *					shall contain the invalid testValues properties id(s) that are
 *					not known by the component or the value(s) are out of range.
 */
void 
MsgTrans_Ctroller_servant::runTest(
	CORBA::ULong TestID, 
	CF::Properties & testValues)
throw (
	CF::UnknownProperties, 
	CF::TestableObject::UnknownTest,
	CORBA::SystemException)
{
	DEBUG(6, [MsgTrans_Ctroller_servant::runTest], "In runTest.")
/**************************OPENSCA-USERREGION-BEGIN*******************************/


/**************************OPENSCA-USERREGION-END*********************************/	
}

