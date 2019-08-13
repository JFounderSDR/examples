/***************************************************************************//**
* @file     Zynq7035_servant.cpp
* @author   open Team
* @version  1
* @date     2019-04-30
* @brief
* @Details
* @Remark : <Description>
* @verbatim
* ==============================================================================
* <Date>     | <Version> | <Author>       | <Description>
* ==============================================================================
*  2019-04-30 | 1       | open Team       | Create file
* ==============================================================================
* @endverbatim
* ******************************************************************************
* <h2><center>&copy; Copyright(c)2017-2022 JFounder Info Tech Co.,Ltd</center></h2>
* All rights reserved. The right to copy, distribute, modify or otherwise make use
* of this software may be licensed only pursuant to the terms
* of an applicable JFounder license agreement.
*//****************************************************************************/

#include "../include/Zynq7035_servant.h"

Zynq7035_servant::Zynq7035_servant()
{
	DEBUG(7, Zynq7035_servant, " In default contructor.")
}

#ifdef __SDS_OS_VXWORKS__
Zynq7035_servant::Zynq7035_servant(
	const char * id,
	const char * lbl,
	const char * sftwfrl,
	const char * nodeName,
	pthread_cond_t * _shutdownCond)
:Resource_impl(id)
{
	DEBUG(7, Zynq7035_servant,
		" In constructor with three input parameters")
	
	m_label = lbl;
	m_softwareProfile = sftwfrl;
	m_nodeName = nodeName;
	
	DEBUG(3, Zynq7035_servant, "In constructor with label is: " << m_label)
	DEBUG(3, Zynq7035_servant, "In constructor with softwareProfile is: "
		<< m_softwareProfile)
	
	m_orbWrap = new openscaSupport::ORB_Wrap::ORB_Wrap();
	
	m_usageState = CF::Device::IDLE;
	m_operationalState = CF::Device::ENABLED;
	m_adminState = CF::Device::UNLOCKED;
	const std::string STACK_SIZE_ID("STACK_SIZE");
	const std::string PRIORITY_ID("PRIORITY");
	
	m_shutdownCond = _shutdownCond;
	pthread_mutex_init(&m_stateMtx, NULL);
	pthread_mutex_init(&m_attrMtx, NULL);
	
	DEBUG(7, Zynq7035_servant, "In constructor with three input parameters")
	DEBUG(7, Zynq7035_servant, "In constructor with id: " << id)
	DEBUG(7, Zynq7035_servant, "In constructor with lbl: " << lbl)
	DEBUG(7, Zynq7035_servant, "In constructor with sftwfrl: " << sftwfrl)
}

#elif defined __SDS_OS_LINUX__

Zynq7035_servant::Zynq7035_servant(
	const char * id,
	const char * lbl,
	const char * sftwfrl,
	const char * nodeName)
:Resource_impl(id)
{
	DEBUG(7, Zynq7035_servant,
		"In constructor with three input parameters")
	
	m_label = lbl;
	m_softwareProfile = sftwfrl;
	m_nodeName = nodeName;
	
	DEBUG(3, Zynq7035_servant, "In constructor with label is: " << m_label)
	DEBUG(3, Zynq7035_servant, "In constructor with softwareProfile is: "
		<< m_softwareProfile)
	
	m_orbWrap = new openscaSupport::ORB_Wrap();
	
	m_usageState = CF::Device::IDLE;
	m_operationalState = CF::Device::ENABLED;
	m_adminState = CF::Device::UNLOCKED;
	const std::string STACK_SIZE_ID("STACK_SIZE");
	const std::string PRIORITY_ID("PRIORITY");
	
	pthread_mutex_init(&m_stateMtx, NULL);
	pthread_mutex_init(&m_attrMtx, NULL);
	
	DEBUG(7, Zynq7035_servant, "In constructor with three input parameters")
	DEBUG(7, Zynq7035_servant, "In constructor with id: " << id)
	DEBUG(7, Zynq7035_servant, "In constructor with lbl: " << lbl)
	DEBUG(7, Zynq7035_servant, "In constructor with sftwfrl: " << sftwfrl)
}
#endif

Zynq7035_servant::~Zynq7035_servant()
{
	///destory mutex resource
	pthread_mutex_destroy(&m_stateMtx);
	pthread_mutex_destroy(&m_attrMtx);
}

/**
 * @brief 	The routine return administrative state of device the admin state
 *         	indicates the permission to use or prohibition against using the
 * 	 		device. AdminType include LOCKED, SHUTTING_DOWN, UNLOCKED
 *
 * @return	AdminType of device.
 */
CF::Device::AdminType
Zynq7035_servant::adminState()
throw (
	CORBA::SystemException)
{
	pthread_mutex_lock(&m_stateMtx);
	CF::Device::AdminType _adminState = m_adminState;
	pthread_mutex_unlock(&m_stateMtx);
	
	return _adminState;
}

/**
 * @brief 	The routine to set administrative state of device.
 *
 * @note  	setting "LOCKED" is only effective when the adminState attribute
 *         	value is UNLOCKED, and setting "UNLOCKED" is only effective when the
 *         	adminState attribute value is LOCKED or SHUTTING_DOWN.
 *         	Illegal state transitions commands are ignored.
 *
 * @param[in]	_adminType	the setting of LOCKED and UNLOCKED values
 */
void 
Zynq7035_servant::adminState(
	CF::Device::AdminType _adminType)
throw (
	CORBA::SystemException)
{
	DEBUG(5, [Zynq7035_servant::adminState], " In adminState.")
	pthread_mutex_lock(&m_stateMtx);

	if (_adminType == CF::Device::LOCKED) {
		if (m_adminState == CF::Device::UNLOCKED)
			m_adminState = CF::Device::LOCKED;
	} else if(_adminType == CF::Device::UNLOCKED) {
		if ((m_adminState == CF::Device::LOCKED) ||
			m_adminState == CF::Device::SHUTTING_DOWN )
			m_adminState = CF::Device::UNLOCKED;
	}
	
	pthread_mutex_unlock(&m_stateMtx);
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
Zynq7035_servant::releaseObject() 
throw (
	CF::LifeCycle::ReleaseError, 
	CORBA::SystemException)
{
	DEBUG(5, [Zynq7035_servant::releaseObject], "In releaseObject.")
	
	
	try {
		/// set device to LOCKED, prevent other object to request service of the device
		adminState(CF::Device::LOCKED);
		/// release all aggregateDevice
		CF::AggregateDevice_var _aggDev = compositeDevice();
		if (!CORBA::is_nil(_aggDev)) {
			CF::DeviceSequence_var _devSeq = _aggDev->devices();
			for (CORBA::UShort i=0; i<_devSeq->length(); ++i)
				(*_devSeq)[i]->releaseObject();
		}
	} catch(...) {
		throw CF::LifeCycle::ReleaseError();
	}
	
	std::string comp_naming = "OpenSCA_Domain/" + m_nodeName + "/" + m_label;
	CORBA::Object_ptr obj = m_orbWrap->get_object_from_string(comp_naming.c_str());
	CosNaming::NamingContext_ptr nc = CosNaming::NamingContext::_nil();
	
	try {
		nc = CosNaming::NamingContext::_narrow(obj);
	} catch(CosNaming::NamingContext::InvalidName& ex) {
		DEBUG(0, [Zynq7035_servant::releaseObject],
			" occure InvalidName Exception.")
		throw CF::LifeCycle::ReleaseError();
	} catch(...) {
		DEBUG(0, [Zynq7035_servant::releaseObject],
			" occure Unknown Exception.")
		throw CF::LifeCycle::ReleaseError();
	}
	
	try {
		m_orbWrap->destory_context( nc );
	} catch(CosNaming::NamingContext::NotEmpty) {
		DEBUG(0, [Zynq7035_servant::releaseObject],
			" NamingContext to be destroy is not empty.")
		throw CF::LifeCycle::ReleaseError();
	} catch(...) {
		DEBUG(0, [Zynq7035_servant::releaseObject],
			" Unknown  Exception.")
		throw CF::LifeCycle::ReleaseError();
	}
	
	///ubind name from domain
	try {
		m_orbWrap->unbind_string(comp_naming.c_str());
	} catch (...) {
		DEBUG(0, [Zynq7035_servant::releaseObject],
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
 * @brief 	This routine return object reference of child device
 *
 * 			The readonly compositeDevice attribute shall contain the object
 * 			reference of the aggregate device when this device is a parent
 *  		device. The readonly compositeDevice attribute shall
 * 			contain a nil CORBA object reference when this device is not a
 * 			parent device.
 *
 * @return aggregate device object reference of this device.
 */
CF::AggregateDevice_ptr 
Zynq7035_servant::compositeDevice() 
throw (
	CORBA::SystemException)
{
	DEBUG(5, [Zynq7035_servant::compositeDevice], "In compositeDevice.")
	return  CF::AggregateDevice::_duplicate(m_compositeDevice);
}

/**
 * @brief 	The allocateCapacity operation provides the mechanism to request and
 *         	allocate capacity from the Device.
 *
 * 			The allocateCapacity operation shall reduce the current capacities of
 * 			the device based upon the input capacities parameter, when the
 * 			device adminState is UNLOCKED, device operationalState is ENABLED,
 * 			and device usageState is not BUSY.
 *
 * 			The allocateCapacity operation shall set the Device usageState
 * 			attribute to BUSY, when the device determines that it is not possible
 * 			to allocate any further capacity. The allocateCapacity operation
 * 			shall set the usageState attribute to ACTIVE, when capacity is
 * 			being used and any capacity is still available for allocation.
 * 			The allocateCapacity operation shall only accept properties for the
 * 			input capacities parameter which are simple properties whose kindtype
 * 			is allocation and whose action element is external contained in the
 * 			component SPD.
 *
 * @param[in]	capacities	capacities of device need to be allocated.
 *
 * @exception The allocateCapacity operation shall raise the InvalidCapacity
 *            exception, when the input capacities parameter contains invalid
 *            properties or when attributes of those CF Properties contain an
 *            unknown id or a value of the wrong data type.
 *			  The allocateCapacity operation shall raise the InvalidState
 *			  exception, when the Device adminState is not UNLOCKED or
 *			  operationalState is DISABLED.
 *
 * @return 	  The allocateCapacity operation shall return TRUE, if the capacities
 *            have been allocated, or FALSE, if not allocated.
 */
CORBA::Boolean 
Zynq7035_servant::allocateCapacity(
	const CF::Properties & capacities)
throw (
	CF::Device::InvalidState, 
	CF::Device::InvalidCapacity, 
	CORBA::SystemException)
{	
	DEBUG(5, [Zynq7035_servant::allocateCapacity], "In allocateCapacity.")

	if ((adminState() != CF::Device::UNLOCKED) ||
		(operationalState() == CF::Device::DISABLED))
		throw CF::Device::InvalidState();
	
	if (usageState() == CF::Device::BUSY)
		throw CF::Device::InvalidCapacity();
	
/**************************OPENSCA-USERREGION-BEGIN*******************************/
	/// assume that one component will occupy whole FPGA resource, because FPGA
	/// just allow only one component run in the same chip
	if (capacities.length() > 0)
		setUsageState(CF::Device::ACTIVE);
	
	return  true;
/**************************OPENSCA-USERREGION-END*********************************/
}

/**
 * @brief 	The deallocateCapacity operation provides the mechanism to return
 * 		  	capacities back to the device, making them available to other users.
 *
 * 			The deallocateCapacity operation shall adjust the current capacities
 * 			of the device based upon the input capacities parameter.
 *
 * 			The deallocateCapacity operation shall set the usageState attribute
 * 			to ACTIVE when,after adjusting capacities, any of the device
 * 			capacities are still being used.
 *
 * 			The deallocateCapacity operation shall set the usageState attribute
 * 			to IDLE when,after adjusting capacities, none of the device capacities
 * 			are still being used.
 *
 * @param[in]	capacities	capacities of device need to be deallocate.
 *
 * @exception 	The deallocateCapacity operation shall raise the InvalidCapacity
 * 			  	exception, when the capacity ID is invalid or the capacity value
 *            	is the wrong type. The InvalidCapacity exception msg parameter
 *            	describes the reason for the exception.
 *
 * 			  	The deallocateCapacity operation shall raise the InvalidState
 * 			  	exception, when the device adminState is LOCKED or operationalState
 * 			  	is DISABLED.
 */
void 
Zynq7035_servant::deallocateCapacity(
	const CF::Properties & capacities)
throw (
	CF::Device::InvalidState, 
	CF::Device::InvalidCapacity, 
	CORBA::SystemException)
{
	DEBUG(5, [Zynq7035_servant::deallocateCapacity], " In deallocateCapacity.")
	if ((adminState() == CF::Device::LOCKED) ||
		(operationalState() == CF::Device::DISABLED))
		throw CF::Device::InvalidState();
	
/**************************OPENSCA-USERREGION-BEGIN*******************************/
	/// assume that one component will occupy whole FPGA resource, because FPGA
	/// just allow only one component run in the same chip
	if (capacities.length() > 0){
		setUsageState(CF::Device::IDLE);
	}

/**************************OPENSCA-USERREGION-END*********************************/
}

/**
 * @brief 	The configure operation allows id/value pair configuration properties
 *		  	to be assigned to components implementing this interface.
 *			The configure operation shall assign values to the properties as
 *  		indicated in the input configProperties parameter. Valid properties for
 *  		the configure operation shall at a minimum be the configure readwrite
 *  		and writeonly properties referenced in the component SPD.
 *
 * @param[in]	configProperties properties need to be configured.
 *
 * @exception 	The configure operation shall raise a PartialConfiguration exception
 *  			when some configuration properties were successfully set and some
 *  			configuration properties were not successfully set.
 *				The configure operation shall raise an InvalidConfiguration exception
 *				when a configuration error occurs and no configuration properties
 *				were successfully set.
 */
void 
Zynq7035_servant::configure(
	const CF::Properties & configProperties)
throw (
	CF::PropertySet::PartialConfiguration,
	CF::PropertySet::InvalidConfiguration,
	CORBA::SystemException)
{
	DEBUG(5, [Zynq7035_servant::configure], " In configure.")
	
	pthread_mutex_lock(&m_attrMtx);

	try {
		PropertySet_impl::configure(configProperties);
	} catch (CF::PropertySet::PartialConfiguration & e) {
		DEBUG(0, [Zynq7035_servant::configure],
			"partial configuration exception.")
		pthread_mutex_unlock (&m_attrMtx);
		throw e;
	} catch (CF::PropertySet::InvalidConfiguration & e) {
		DEBUG(0, [Zynq7035_servant::configure],
			"invalid configuration exception.")
		pthread_mutex_unlock (&m_attrMtx);
		throw e;
	} catch (...) {
		DEBUG(0, [Zynq7035_servant::configure],
			"occur unkown exception when config.")
		pthread_mutex_unlock (&m_attrMtx);
		throw CF::PropertySet::InvalidConfiguration();
	}
	
/**************************OPENSCA-USERREGION-BEGIN*******************************/
	/// only readwrite and writeonly can be configured. if this properties need to 
	/// be saved after configured, call PropertySet_impl::configure(props);
	
	
/**************************OPENSCA-USERREGION-END*********************************/
	pthread_mutex_unlock(&m_attrMtx);
}

/**
 * @brief 	The purpose of the initialize operation is to provide a mechanism to
 * 		  	set a component to a known initial state. For example, data structures
 * 		  	may be set to initial values, memory may be allocated, hardware devices
 * 		  	may be configured to some state, etc.
 *
 * @exception 	The initialize operation shall raise an InitializeError exception
 *  		  	when an initialization error occurs.
 */
void 
Zynq7035_servant::initialize()
throw (
	CF::LifeCycle::InitializeError, 
	CORBA::SystemException)
{
	DEBUG(5, [Zynq7035_servant::initialize], " In initialize.")
	
			
	
    m_compositeDevice = (new AggregateDevice_impl())->_this();
	
/**************************OPENSCA-USERREGION-BEGIN*******************************/


/**************************OPENSCA-USERREGION-END*********************************/
}

/**
 * @biref 	This routine return label of device, the label is meaning name given
 * 			to a device.
 *
 * @note  	That calling function must use "CORBA::string_var str = label()",
 *   	  	otherwise cause memory leak.
 *
 * @retval 	the pointer to label
 */
char * 
Zynq7035_servant::label() 
throw (
	CORBA::SystemException)
{
	DEBUG(9, [Zynq7035_servant::label], " In label.")
	return  CORBA::string_dup(m_label.c_str());
}

/**
 * @brief 	This routine return device operationalType, which include ENABLED and DISABLED.
 *
 * @return	operational state of device
 * @retval 	ENABLED	the device is available
 * @retval 	DISABLED the deivice is not available
 */
CF::Device::OperationalType 
Zynq7035_servant::operationalState()
throw (
	CORBA::SystemException)
{
	DEBUG(5, [Zynq7035_servant::operationalState], " In operationalState.")
	pthread_mutex_lock(&m_stateMtx);
	CF::Device::OperationalType _operatState = m_operationalState;
	pthread_mutex_unlock(&m_stateMtx);
	
	return m_operationalState;
}

/**
 * @brief 	The query operation allows a component to be queried to retrieve its
 * 			properties. The query operation shall return all component properties
 * 			when the inout parameter configProperties is zero size. The query operation
 * 			shall return only those id/value pairs specified in the configProperties
 * 			parameter if the parameter is not zero size. Valid properties for the query
 * 			operation shall be all configure properties (simple properties whose kind
 * 			element kindtype attribute is "configure" whose mode attribute is
 * 			"readwrite" or "readonly" and any allocation properties with an action
 * 			value of "external" as referenced in the component SPD.
 *
 * @param[inout]	props	properties need to be queried.
 *
 * @exception The query operation shall raise the CF UnknownProperties exception when
 * 			  one or more properties being requested are not known by the component.
 */
void 
Zynq7035_servant::query(
	CF::Properties & props)
throw (
	CF::UnknownProperties,
	CORBA::SystemException)
{
	DEBUG(5, [Zynq7035_servant::query], " In query.")
	
	pthread_mutex_lock(&m_attrMtx);
	PropertySet_impl::query(props);
/**************************OPENSCA-USERREGION-BEGIN*******************************/
	/// fill those properties of which value need to be aquired from driver interface


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
Zynq7035_servant::runTest(
	CORBA::ULong TestID, 
	CF::Properties & testValues)
throw (
	CF::UnknownProperties, 
	CF::TestableObject::UnknownTest, 
	CORBA::SystemException)
{
	DEBUG(5, [Zynq7035_servant::runTest], "In runTest.")
/**************************OPENSCA-USERREGION-BEGIN*******************************/


/**************************OPENSCA-USERREGION-END*********************************/
}

/**
 * @brief 	This routine return softwareProfile attrbute value.
 * 			The softwareProfile attribute contains the Profile Descriptor for
 * 			the application that is created by the application factory.
 * 			The readonly softwareProfile attribute shall contain a profile
 * 			element (Profile Descriptor) with a file reference to the device
 * 			SPD file. Files referenced within the profile are obtained via
 * 			FileManager.
 *
 * @return	device SPD file relative to file mount point.
 */
char *
Zynq7035_servant::softwareProfile() 
throw (
	CORBA::SystemException)
{
	DEBUG(5, [Zynq7035_servant::softwareProfile], " In softwareProfile.")
			
	return  CORBA::string_dup(m_softwareProfile.c_str());
}

/**
 * @brief The start operation is provided to command the resource implementing
 *        this interface to start internal processing.The start operation puts
 *        the resource in an operating condition.
 *
 * @exception 	StartError The start operation shall raise the StartError
 *             	exception if an error occurs while starting the resource.
 */
void 
Zynq7035_servant::start() 
throw (
	CF::Resource::StartError, 
	CORBA::SystemException)
{
	DEBUG(5, [Zynq7035_servant::start], "In start.")
	setOperationalState(CF::Device::ENABLED);
/**************************OPENSCA-USERREGION-BEGIN*******************************/


/**************************OPENSCA-USERREGION-END*********************************/
}

/**
 * @brief 	The stop operation is provided to command the resource implementing
 *         	this interface to stop internal processing.
 * 			The stop operation shall disable all current operations and put the
 * 			resource in a non-operating condition. The stop operation shall not
 * 			inhibit subsequent configure, query, and start operations.
 *
 * @exception 	StopError The start operation shall raise the StopError
 *             	exception if an error occurs while stopping the device.
 */
void 
Zynq7035_servant::stop() 
throw (
	CF::Resource::StopError, 
	CORBA::SystemException)
{
	DEBUG(5, [Zynq7035_servant::stop], " In stop.")
	setOperationalState(CF::Device::DISABLED);
/**************************OPENSCA-USERREGION-BEGIN*******************************/


/**************************OPENSCA-USERREGION-END*********************************/
}

/**
 * @brief 	This routine return usageState attribute value
 * 		  	The readonly usageState attribute shall contain the device usage
 * 		  	state (IDLE, ACTIVE, or BUSY). UsageState indicates whether or not a
 * 		  	device is actively in use at a specific instant, and if so,
 * 		  	whether or not it has spare capacity for allocation at that instant.
 *
 * @return 	UsageType of device.
 */
CF::Device::UsageType 
Zynq7035_servant::usageState() 
throw (
	CORBA::SystemException)
{
	DEBUG(9, [Zynq7035_servant::usageState], " In usageState.")
	pthread_mutex_lock(&m_stateMtx);
	CF::Device::UsageType _usageState = m_usageState;
	pthread_mutex_unlock(&m_stateMtx);
	
	return _usageState;
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
Zynq7035_servant::getPort(
	const char * portName)
throw (
	CORBA::SystemException, 
	CF::PortSupplier::UnknownPort)
{
    DEBUG(6, [Zynq7035_servant::getPort], "In getPort.")
		
	CORBA::Object_var _port;
	
	std::string portFullName = "OpenSCA_Domain/" + m_nodeName + "/" + portName;

	
	//don't find any port named by protName
	throw CF::PortSupplier::UnknownPort();
}

/**
 * @brief	The load operation provides the mechanism for loading software on a
 *        	specific device.
 *
 *	      	The loaded software may be subsequently executed on the device, if
 *	      	the device is an executable device.
 *
 *  		The load operation shall load the file identified by the input
 *  		filename parameter on the device based upon the input loadKind
 *  		parameter. The input filename parameter is a pathname relative to
 *  		the file system identified by the input FileSystem parameter
 *
 *  		The load operation shall support the load types as stated in the
 *  		device software profile LoadType allocation properties.
 *
 *  		Multiple loads of the same file as indicated by the input fileName
 *  		parameter shall not result in an exception. However, the load
 *  		operation should account for this multiple load so that the
 *  		unload operation behavior can be performed.
 *
 * @param[in]	fs			fileSystem  in where the file need loaded is.
 * @param[in]	fileName 	name of the file need loaded.
 * @param[in]	loadKind	type of load to be performed
 *
 * @exception	The load operation shall raise the InvalidState exception if upon
 *            	entry the Device adminState
 * 				attribute is either LOCKED or SHUTTING_DOWN or its operationalState
 * 				attribute is DISABLED.
 *
 *			   	The load operation shall raise the InvalidLoadKind exception when
 *			   	the input loadKind parameter is not supported.
 *
 *			   	The load operation shall raise the CF InvalidFileName exception
 *			   	when the file designated by the input filename parameter cannot
 *			   	be found.
 *
 *			   	The load operation shall raise the LoadFail exception when an
 *			   	attempt to load the device is unsuccessful.
 */
void 
Zynq7035_servant::load(
	CF::FileSystem_ptr fs,
	const char * fileName,
	CF::LoadableDevice::LoadType loadKind) 
throw (
	CF::LoadableDevice::LoadFail,
	CF::InvalidFileName,
	CF::LoadableDevice::InvalidLoadKind,
	CF::Device::InvalidState,
	CORBA::SystemException)
{
#ifdef __SDS_OS_VXWORKS__
	DEBUG(5, [Zynq7035_servant::load], "In load.")
/**************************OPENSCA-USERREGION-BEGIN*******************************/

		
/**************************OPENSCA-USERREGION-END*********************************/
#endif
}

/**
 * @brief	The unload operation provides the mechanism to unload software that
 *        	is currently loaded.
 *
 * 			The unload operation shall unload the file identified by the input
 * 			fileName parameter from the device when the number of unload requests
 * 			matches the number of load requests for the indicated file.
 *
 * @param[in]	fileName	name of file need to be unloaded.
 *
 * @exception	The unload operation shall raise the InvalidState exception if
 *            	upon entry the device adminState attribute is LOCKED or its
 *            	operationalState attribute is DISABLED.
 *
 *			  	The unload operation shall raise the CF InvalidFileName exception
 *			  	when the file designated by the input filename parameter cannot
 *			  	be found.
 */
void 
Zynq7035_servant::unload(
	const char * fileName)
throw (
	CF::InvalidFileName,
	CF::Device::InvalidState,
    CORBA::SystemException)
{
#ifdef __SDS_OS_VXWORKS__
	DEBUG(5, [Zynq7035_servant::unload], "In unload.")
/**************************OPENSCA-USERREGION-BEGIN*******************************/

		
/**************************OPENSCA-USERREGION-END*********************************/
#endif
}

/**
 * @brief	The execute operation provides the mechanism for starting up and
 *        	executing a software process/thread on a device.
 *
 * @param[in]	name		The execute operation shall execute the function or
 * 							file identified by the input name parameter using the
 * 							input parameters and options parameters. Whether the
 * 							input name parameter is a function or a file name is
 * 							device-implementation-specific.
 *
 * @param[in]	options		The execute operation input options parameters are
 * 							STACK_SIZE_ID and PRIORITY_ID. The execute operation
 * 							shall use these options, when specified, to set the
 * 							operating system process/thread stack size and
 * 							priority, for the executable image of the given input
 * 							name parameter.
 *
 * @param[in]	parameters	The execute operation shall convert the input
 * 							parameters (id/value string pairs) parameter to the
 * 							standard argv of the POSIX exec family of functions,
 * 							where argv(0) is the function name.
 *
 * 							The execute operation shall map the input parameters
 * 							parameter to argv starting at index 1 as follows,
 * 							argv (1) maps to input parameters (0) id and argv (2)
 * 							maps to input parameters (0) value and so forth.
 * 							The execute operation passes argv through the
 * 							operating system "execute" function.
 *
 * @return	a unique process ID for the process that it crated.
 */
CF::ExecutableDevice::ProcessID_Type 
Zynq7035_servant::execute(
	const char * name,
	const CF::Properties & options,
	const CF::Properties & parameters)
throw (
	CF::ExecutableDevice::ExecuteFail,
	CF::InvalidFileName, 
	CF::ExecutableDevice::InvalidOptions,
	CF::ExecutableDevice::InvalidParameters,
	CF::ExecutableDevice::InvalidFunction,
	CF::Device::InvalidState,
	CORBA::SystemException)
{
	DEBUG(5, [Zynq7035_servant::execute], " In execute.")
/**************************OPENSCA-USERREGION-BEGIN*******************************/
	CF::ExecutableDevice::ProcessID_Type processId;	
#ifdef __SDS_OS_VXWORKS__
	processId = executeOnVxworks(name, options, parameters);
#elif defined __SDS_OS_LINUX__
	processId = executeOnLinux(name, options, parameters);
#endif
	return processId;
/**************************OPENSCA-USERREGION-END*********************************/
}

CF::ExecutableDevice::ProcessID_Type
Zynq7035_servant::executeOnVxworks(
		const char * name,
		const CF::Properties & options,
		const CF::Properties & parameters)
throw (
		CF::ExecutableDevice::ExecuteFail,
	    CF::InvalidFileName,
		CF::ExecutableDevice::InvalidOptions,
	    CF::ExecutableDevice::InvalidParameters,
	    CF::ExecutableDevice::InvalidFunction,
	    CF::Device::InvalidState,
	    CORBA::SystemException)
{
#ifdef __SDS_OS_VXWORKS__
/**************************OPENSCA-USERREGION-BEGIN*******************************/


/**************************OPENSCA-USERREGION-END*********************************/
#endif
}

CF::ExecutableDevice::ProcessID_Type
Zynq7035_servant::executeOnLinux(
		const char * name,
		const CF::Properties & options,
		const CF::Properties & parameters)
throw (
		CF::ExecutableDevice::ExecuteFail,
	    CF::InvalidFileName,
		CF::ExecutableDevice::InvalidOptions,
	    CF::ExecutableDevice::InvalidParameters,
	    CF::ExecutableDevice::InvalidFunction,
	    CF::Device::InvalidState,
	    CORBA::SystemException)
{
#ifdef __SDS_OS_LINUX__
/**************************OPENSCA-USERREGION-BEGIN*******************************/
	CF::ExecutableDevice::ProcessID_Type processId =
		(CF::ExecutableDevice::ProcessID_Type)(-1);

	pid_type pid;

	if (name == NULL) {
		DEBUG(0, [Zynq7035_servant::executeOnLinux],
			" input parameter name:" << name << "is invalid")
		throw CF::InvalidFileName();
	}

	CF::Properties invalidProperties;
	invalidProperties.length(0);

	CORBA::ULong ulStacksize = 0;
	CORBA::ULong ulPriority = 0;
	std::string entrypoint;

	DEBUG(7, [Zynq7035_servant::executeOnLinux],
		" options length is " << options.length())

	parseOptions(options,ulStacksize, ulPriority,
		entrypoint, invalidProperties);

	if (invalidProperties.length() > 0) {
		throw CF::ExecutableDevice::InvalidOptions (invalidProperties);
	}

	invalidProperties.length(0);

	int argc = parameters.length() * 2 + TASK_PARAMS_COUNT + 1;
	char** argv = parseExecuteParams(
		argc, parameters, ulStacksize, ulPriority,
		entrypoint, invalidProperties);

    char openScaPath[64];
    getConfigFilePathFromSHM(openScaPath, sizeof(openScaPath));
  	ConfigParser configParser(openScaPath);
	std::string fsRoot = configParser.getValueById(CONSTANT::FSROOT);
	std::string fileFullPath = fsRoot + "/" + name;

	pid = fork();
	if(pid == -1) {
		DEBUG(0, [Zynq7035_servant::executeOnLinux],
			" fork() failed.")
		return -1;
	}

	if(pid == 0) {
		pidMap.insert(std::pair<std::string, pid_t>(entrypoint, getpid()));
		execv(fileFullPath.c_str(), argv);
		DEBUG(0, [Zynq7035_servant::executeOnLinux],
			" child process end.")
		exit(0);
	}

	processId = (CF::ExecutableDevice::ProcessID_Type)pid;

	sMsSleep(10);

	if (argv != NULL) {
		delete2DArray(argc, argv);
	}

	DEBUG(7, [Zynq7035_servant::executeOnLinux], "leaving ...")

	return processId;
/**************************OPENSCA-USERREGION-END*********************************/
#endif
}

void
Zynq7035_servant::parseOptions(
	const CF::Properties & options,
	CORBA::ULong & stacksize,
	CORBA::ULong & priority,
	std::string & entrypoint,
	CF::Properties & invalidProperties)
{
	//parse stacksize and priority from options
	for (int i = 0; i < options.length(); i++) {
		CORBA::String_var propertyId = CORBA::string_dup(options[i].id);

		if (0 == strcmp(propertyId.in(), "")) {
			invalidProperties.length(invalidProperties.length() + 1);
			invalidProperties[invalidProperties.length() - 1] = options[i];
			continue;
		}

		if (strcmp(propertyId.in(), "STACK_SIZE_ID") == 0) {
			if (options[i].value >>= stacksize) {
				DEBUG(7, [Zynq7035_servant::parseOptions],
						" stacksize is " << stacksize)
			} else {
				invalidProperties.length(invalidProperties.length() + 1);
				invalidProperties[invalidProperties.length() - 1] = options[i];
			}
		} else if (strcmp(propertyId.in(), "PRIORITY_ID") == 0) {
			if (options[i].value >>= priority) {
				DEBUG(7, [Zynq7035_servant::parseOptions],
						" priority is " << priority)
			} else {
				invalidProperties.length(invalidProperties.length() + 1);
				invalidProperties[invalidProperties.length() - 1] = options[i];
			}
		} else if (strcmp(propertyId.in(), "ENTRY_POINT") == 0) {
			const char* val = NULL;

			if (options[i].value >>= val) {
				entrypoint = val;
				DEBUG(7, [Zynq7035_servant::parseOptions],
						" entypoint is " << entrypoint)
			} else {
				invalidProperties.length(invalidProperties.length() + 1);
				invalidProperties[invalidProperties.length() - 1] = options[i];
			}
		} else {
			invalidProperties.length(invalidProperties.length() + 1);
			invalidProperties[invalidProperties.length() - 1] = options[i];
		}
	}
}

char**
Zynq7035_servant::parseExecuteParams(
	int argc,
	const CF::Properties & parameters,
	CORBA::ULong ulStacksize,
	CORBA::ULong ulPriority,
	const std::string & entrypoint,
	CF::Properties & invalidProperties)
{
	char** argv = new char*[argc];
	bzero(argv, argc);
	int index = 0;
	for (int j = 0; j < parameters.length(); j++) {
		std::string strArg = CORBA::string_dup(parameters[j].id);
		const char* str_val = 0;

		argv[2 * j] = NULL;
		argv[(2 * j) + 1] = NULL;

		if (strArg != "") {
			argv[2 * j] = new char[strArg.length() + 1];
			bzero(argv[2 * j], strArg.length() + 1);
			strcpy(argv[2 * j], strArg.c_str());

			CORBA::Any::to_string v_str(str_val, 0);

			if (parameters[j].value >>= v_str) {
				argv[(2 * j) + 1] = new char[strlen(str_val) + 1];
				bzero(argv[(2 * j) + 1], strlen(str_val) + 1);
				strcpy(argv[(2 * j) + 1], str_val);
				index = (2 * j) + 1;
			} else {
				invalidProperties.length(invalidProperties.length() + 1);
				invalidProperties[invalidProperties.length() - 1] =
						parameters[j];
			}
		} else {
			invalidProperties.length(invalidProperties.length() + 1);
			invalidProperties[invalidProperties.length() - 1] = parameters[j];
		}
	}

	argv[++index] = new char[strlen("-STACK_SIZE") + 1];
	bzero(argv[index], strlen("-STACK_SIZE") + 1);
	strcpy(argv[index], "-STACK_SIZE");

	argv[++index] = new char[sizeof(ulStacksize) + 1];
	bzero(argv[index], sizeof(ulStacksize) + 1);
	sprintf(argv[index], "%d", ulStacksize);

	argv[++index] = new char[strlen("-PRIORITY") + 1];
	bzero(argv[index], strlen("-PRIORITY") + 1);
	strcpy(argv[index], "-PRIORITY");

	argv[++index] = new char[sizeof(ulPriority) + 1];
	bzero(argv[index], sizeof(ulPriority) + 1);
	sprintf(argv[index], "%d", ulPriority);

	argv[++index] = new char[strlen("-ENTRY_POINT") + 1];
	bzero(argv[index], strlen("-ENTRY_POINT") + 1);
	strcpy(argv[index], "-ENTRY_POINT");

	argv[++index] = new char[strlen(entrypoint.c_str()) + 1];
	bzero(argv[index], strlen(entrypoint.c_str()) + 1);
	strcpy(argv[index], entrypoint.c_str());

	argv[++index] = NULL;

	if ((invalidProperties.length() > 0) && (argv)) {
		delete2DArray(argc, argv);
		throw CF::ExecutableDevice::InvalidParameters(invalidProperties);
	}
	return argv;
}

/**
 * @brief	The terminate operation provides the mechanism for terminating
 *        	the execution of a process/thread on a specific device that was
 *        	started up with the execute operation.
 *
 * 			The terminate operation shall terminate the execution of the
 * 			process/thread designated by the processId input parameter on
 * 			the device.
 *
 * @processId	id of process need to be terminated.
 *
 * @exception	The terminate operation shall raise the InvalidState exception
 *            	if upon entry the device adminState attribute is LOCKED or
 *            	its operationalState attribute is DISABLED.
 *
 * 				The terminate operation shall raise the InvalidProcess exception
 * 				when the process Id does not exist for the device.
 */
void 
Zynq7035_servant::terminate(
	CF::ExecutableDevice::ProcessID_Type processId)
throw (
	CF::Device::InvalidState,
	CF::ExecutableDevice::InvalidProcess,
    CORBA::SystemException)
{
	DEBUG(5, [Zynq7035_servant::terminate], " In terminate.")
/**************************OPENSCA-USERREGION-BEGIN*******************************/

		
/**************************OPENSCA-USERREGION-END*********************************/
}

/**
 * @brief	This funtion used to set device usage state.
 *
 * @param[in]	newUsageState	new usage state the device will be.
 */
void 
Zynq7035_servant::setUsageState(
	CF::Device::UsageType newUsageState)
{
	DEBUG(5, [Zynq7035_servant::setUsageState], " In setUsageState.")
	pthread_mutex_lock(&m_stateMtx);
	m_usageState = newUsageState;
	pthread_mutex_unlock(&m_stateMtx);
}

/**
 * @brief	This function used to set device operational state.
 *
 * @param[in]	newOperationalState	new operational state the device will be.
 */
void 
Zynq7035_servant::setOperationalState(
	CF::Device::OperationalType newOperationalState)
{
	DEBUG(5, [Zynq7035_servant::setOperationalState],
		" In setOperationalState.")
	pthread_mutex_lock(&m_stateMtx);
	m_operationalState = newOperationalState;
	pthread_mutex_unlock(&m_stateMtx);
}

