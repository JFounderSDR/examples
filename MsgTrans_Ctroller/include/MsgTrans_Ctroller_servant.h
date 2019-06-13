/***************************************************************************//**
* @file     MsgTrans_Ctroller_servant.h
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

#if !defined __MSGTRANS_CTROLLER_INCLUDED__
#define __MSGTRANS_CTROLLER_INCLUDED__

#include <pthread.h>
#include <vector>

#include "debug.h"
#include "Resource_impl.h"
#include "openscaSupport.h"
#include "utils.h"
#include "Boost_utils.h"

#include "Resource_u.h"

using namespace std;

#define BLOCK_ERROR_RATE 	("DCE:8b91015a-b0cd-47eb-b475-c44c70bb56a4:1")
#define LOCAL_LD		 	("DCE:1b26d09b-472d-4515-b872-4063e9d39062:1")
#define TARGET_LD		 	("DCE:59d888d5-15e6-413b-aa5c-bd344f040998:1")
#define CONNECTION		 	("DCE:6cb56ba8-0bb9-455e-b34f-cba88b481bff:1")
#define START_STATUS	 	("DCE:085d86cb-1d1c-45fd-94ed-afa8c0e98414:1")
#define BUSINESS_TYPE		("DCE:04a9d833-be0c-4fe6-a5c6-a67e1ca7dcfa:1")
#define CRCCOMP_ID			("DCE:9daf8086-91dc-4406-9d89-3d48565c5c1a:1")

class MsgTrans_Ctroller_servant: 
public virtual POA_CF::Resource,
public virtual PortableServer::RefCountServantBase,
public virtual Resource_impl
{

public:

	virtual ~MsgTrans_Ctroller_servant();
	
#ifdef __SDS_OS_VXWORKS__	
	MsgTrans_Ctroller_servant(
		const char * _id, 
		const char * _cosNaming, 
		const char * _appName, 
		const char * _sftwfl, 
		const char * _fsroot, 
		pthread_cond_t * _shutdownCond);
#elif defined __SDS_OS_LINUX__										
	MsgTrans_Ctroller_servant(
		const char * _id, 
		const char * _cosNaming, 
		const char * _appName, 
		const char * _sftwfl, 
		const char * _fsroot);
#endif
	
	/**
	 * @brief 	The start operation is provided to command the resource implementing 
	 *         	this interface to start internal processing.The start operation puts 
	 *         	the resource in an operating condition.
	 *
	 * @exception 	StartError The start operation shall raise the StartError 
	 *             	exception if an error occurs while starting the resource.
	 */
	virtual void 
	start() 
	throw (
		CORBA::SystemException, 
		CF::Resource::StartError);

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
	virtual void 
	stop() 
	throw (
		CORBA::SystemException, 
		CF::Resource::StopError);

	/**
	 * @brief 	The purpose of the initialize operation is to provide a mechanism to 
	 *          set a component to a known initial state. For example, data structures 
	 *          may be set to initial values, memory may be allocated, hardware 
	 *          devices may be configured to some state, etc.
	 *
	 * @exception 	The initialize operation shall raise an InitializeError 
	 *             	exception when an initialization error occurs.
	 */
	virtual void 
	initialize() 
	throw (
		CF::LifeCycle::InitializeError, 
		CORBA::SystemException);

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
	virtual void 
	releaseObject() 
	throw (
		CORBA::SystemException, 
		CF::LifeCycle::ReleaseError);

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
	virtual void 
	runTest(
		CORBA::ULong TestID, 
		CF::Properties & testValues)
	throw (
		CF::UnknownProperties, 
		CF::TestableObject::UnknownTest,
		CORBA::SystemException);

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
	virtual void 
	configure( 
		const CF::Properties & )
	throw (
		CORBA::SystemException,
		CF::PropertySet::InvalidConfiguration,
		CF::PropertySet::PartialConfiguration);

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
	virtual void 
	query( 
		CF::Properties & ) 
	throw (
		CORBA::SystemException, 
		CF::UnknownProperties);

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
	virtual CORBA::Object * 
	getPort( 
		const char * name )
	throw (
		CORBA::SystemException, 
		CF::PortSupplier::UnknownPort);
		
	void
	getConfigPropsFromPRF();
		
	
protected:
	MsgTrans_Ctroller_servant();

private:

	///define port here: use_port and provide_port
    StandardInterfaces_i::Resource_u* control_out_uport; 
	
	
	
	openscaSupport::ORB_Wrap * m_orbWrap;
	
	/// synchronosize to configure and query device attribute simultaneously
	pthread_mutex_t 			m_attrMtx;
	/// control to release device servant
	pthread_cond_t *			m_shutdownCond;
	
	std::vector<std::string> 	m_mhalPortNames;
	
	std::string 				m_fsroot;
	std::string 				m_spdRelPath;
	std::string 				m_appName;
	std::string 				m_cosNaming;
	
	bool						m_initConfig;
	CF::Properties 				m_prfConfigProps;
	
	/// control thread start for sending and receiving
	bool m_isStarted;

	CF::Device_var m_mhalDev;

	
	/**
 	 * @brief	This function used to set Mhal port's logical address.
 	 * 			The logical address is read from component's SPD file.
 	 */
	void setMhalPortLD();
};
#endif /// !defined(MsgTrans_Ctroller_servant_INCLUDED_)
