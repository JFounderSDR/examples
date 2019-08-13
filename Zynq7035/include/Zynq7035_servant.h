/***************************************************************************//**
* @file     Zynq7035_servant.h
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

#if !defined __ZYNQ7035_INCLUDED__
#define __ZYNQ7035_INCLUDED__

#include <pthread.h>

#include "CFS.h"
#include "AggregateDevice_impl.h"
#include "debug.h"
#include "openscaSupport.h"
#include "Resource_impl.h"
#include "utils.h"
#include "Boost_utils.h"

using namespace std;

class Zynq7035_servant :
public virtual Resource_impl,
public virtual POA_CF::ExecutableDevice
{

public:
	
#ifdef __SDS_OS_VXWORKS__
	Zynq7035_servant(
		const char * id,
		const char * lbl,
		const char * sftwfrl,
		const char * nodeName,
		pthread_cond_t * _shutdownCond);
#elif defined __SDS_OS_LINUX__
	Zynq7035_servant(
		const char * id,
		const char * lbl,
		const char * sftwfrl,
		const char * nodeName);
#endif

	~Zynq7035_servant();
	
	/**
	 * @biref 	This routine return label of device, the label is meaning name
	 *         	given to a device.
	 *
	 * @note  	That calling function must use "CORBA::string_var str = label()",
	 *         	otherwise cause memory leak.
	 *
	 * @return 	the pointer to label
	 */
	char * 
	label() 
	throw (
		CORBA::SystemException);
	
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
	softwareProfile() 
	throw (
		CORBA::SystemException);
	
	/**
	* @brief 	The routine return administrative state of device the admin state
	*         	indicates the permission to use or prohibition against using the
	* 	 		device. AdminType include LOCKED, SHUTTING_DOWN, UNLOCKED
	*
	* @return	AdminType of device.
	*/
	CF::Device::AdminType 
	adminState() 
	throw (
		CORBA::SystemException);
	
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
	adminState(
		CF::Device::AdminType _adminType) 
	throw (
		CORBA::SystemException);
	
	/**
	 * @brief 	This routine return device operationalType, which include
	 *         	ENABLED and DISABLED.
	 *
	 * @return	operational state of device
	 * @retval 	ENABLED	the device is available
	 * @retval 	DISABLED the deivice is not available
	 */
	CF::Device::OperationalType 
	operationalState() 
	throw (
		CORBA::SystemException);
	
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
	usageState() 
	throw (
		CORBA::SystemException);
	
	/**
	 * @brief 	This routine return object reference of child device
	 *
	 * 			The readonly compositeDevice attribute shall contain the object
	 * 			reference of the aggregate device when this device is a parent
	 * 			device. The readonly compositeDevice attribute shall
	 * 			contain a nil CORBA object reference when this device is not a
	 * 			parent device.
	 *
	 * @return aggregate device object reference of this device.
	 */
	CF::AggregateDevice_ptr 
	compositeDevice() 
	throw (
		CORBA::SystemException);

	/**
	 * @brief The start operation is provided to command the resource implementing
	 *        this interface to start internal processing.The start operation puts
	 *        the resource in an operating condition.
	 *
	 * @exception 	StartError The start operation shall raise the StartError
	 *             	exception if an error occurs while starting the resource.
	 */
	void 
	start() 
	throw (
		CF::Resource::StartError, 
		CORBA::SystemException);

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
	stop() 
	throw (
		CF::Resource::StopError, 
		CORBA::SystemException);

	/**
	 * @brief 	The purpose of the initialize operation is to provide a mechanism
	 *         	to set a component to a known initial state. For example,
	 *         	data structures may be set to initial values, memory may
	 * 		  	be allocated, hardware devices may be configured to some state, etc.
	 *
	 * @exception 	The initialize operation shall raise an InitializeError exception
	 *             	when an initialization error occurs.
	 */
	void 
	initialize() 
	throw (
		CF::LifeCycle::InitializeError, 
		CORBA::SystemException);

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
	configure(
		const CF::Properties & configProperties)
	throw (
		CF::PropertySet::PartialConfiguration,
		CF::PropertySet::InvalidConfiguration,
		CORBA::SystemException);

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
	query(
		CF::Properties & configProperties)
	throw (
		CF::UnknownProperties,
		CORBA::SystemException);

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
	allocateCapacity(
		const CF::Properties & capacities)
	throw (
		CF::Device::InvalidState, 
		CF::Device::InvalidCapacity, 
		CORBA::SystemException);

	/**
	 * @brief 	The deallocateCapacity operation provides the mechanism to return
	 *         	capacities back to the device, making them available to other users.
	 *
	 * 			The deallocateCapacity operation shall adjust the current capacities
	 * 			of the device based upon the input capacities parameter.
	 *
	 * 			The deallocateCapacity operation shall set the usageState attribute
	 * 			to ACTIVE when,after adjusting capacities, any of the device
	 * 			capacities are still being used.
	 *
	 * 			The deallocateCapacity operation shall set the usageState attribute
	 * 			to IDLE when,after adjusting capacities, none of the device
	 * 			capacities are still being used.
	 *
	 * @param[in]	capacities	capacities of device need to be deallocate.
	 *
	 * @exception 	The deallocateCapacity operation shall raise the InvalidCapacity
	 *             	exception, when the capacity ID is invalid or the capacity value
	 *             	is the wrong type. The InvalidCapacity exception msg parameter
	 *             	describes the reason for the exception.
	 *
	 * 			  	The deallocateCapacity operation shall raise the InvalidState
	 * 			  	exception, when the device adminState is LOCKED or
	 * 			  	operationalState is DISABLED.
	 */
	void 
	deallocateCapacity(
		const CF::Properties & capacities)
	throw (
		CF::Device::InvalidState, 
		CF::Device::InvalidCapacity, 
		CORBA::SystemException);

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
	getPort(
		const char * name)
	throw (
		CF::PortSupplier::UnknownPort, 
		CORBA::SystemException);

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
	runTest(
		CORBA::ULong TestID, 
		CF::Properties & testValues)
	throw (
		CF::UnknownProperties, 
		CF::TestableObject::UnknownTest,
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
	void 
	releaseObject() 
	throw (
		CF::LifeCycle::ReleaseError, 
		CORBA::SystemException);

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
	load(
		CF::FileSystem_ptr fs, 
		const char * fileName,
		CF::LoadableDevice::LoadType loadKind)
	throw (
		CF::LoadableDevice::LoadFail, 
		CF::InvalidFileName,
	    CF::LoadableDevice::InvalidLoadKind, 
		CF::Device::InvalidState,
	    CORBA::SystemException);

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
	unload(
		const char * fileName)
	throw (
		CF::InvalidFileName, 
		CF::Device::InvalidState, 
		CORBA::SystemException);

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
	execute(
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
		CORBA::SystemException);

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
	terminate(
		CF::ExecutableDevice::ProcessID_Type processId)
	throw (
		CF::Device::InvalidState,
		CF::ExecutableDevice::InvalidProcess,
	    CORBA::SystemException);
	            

protected:

	CF::Device::AdminType 		m_adminState;
	CF::Device::OperationalType m_operationalState;
	CF::Device::UsageType		m_usageState;
	std::string 				m_label;	
	std::string			 		m_softwareProfile;
	std::string					m_nodeName;
	//TODO
	const std::string 			STACK_SIZE_ID;
	const std::string 			PRIORITY_ID;
	
	/// synchronosize to fix adminState|operationalState|usageState simultanously
	pthread_mutex_t		        m_stateMtx;
	/// synchronosize to configure and query device attribute simultaneously
	pthread_mutex_t 			m_attrMtx;
	/// control to release device servant
	pthread_cond_t *			m_shutdownCond;
	
	bool 	 					m_initConfig;
	CF::AggregateDevice_var		m_compositeDevice;

	/**
	 * @brief	This funtion used to set device usage state.
	 *
	 * @param[in]	newUsageState	new usage state the device will be.
	 */
	void 
	setUsageState(
		CF::Device::UsageType newUsageState);

	/**
	 * @brief	This function used to set device operational state.
	 *
	 * @param[in]	newOperationalState	new operational state the device will be.
	 */
	void 
	setOperationalState(
		CF::Device::OperationalType operationalState);
	           

private:

#ifdef __SDS_OS_LINUX__
	PidMap pidMap;
#endif

	Zynq7035_servant();

	///define port here: use_port and provide_port


	openscaSupport::ORB_Wrap* m_orbWrap;

	
	CF::ExecutableDevice::ProcessID_Type
	executeOnVxworks(
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
		    CORBA::SystemException);

	CF::ExecutableDevice::ProcessID_Type
	executeOnLinux(
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
		    CORBA::SystemException);

	void
	parseOptions(
		const CF::Properties & options,
		CORBA::ULong & stacksize,
		CORBA::ULong & priority,
		std::string & entrypoint,
		CF::Properties & invalidProperties);

	char**
	parseExecuteParams(
		int argc,
		const CF::Properties & parameters,
		CORBA::ULong ulStacksize,
		CORBA::ULong ulPriority,
		const std::string & entrypoint,
		CF::Properties & invalidProperties);
			
};

#endif // !defined(Zynq7035_servant_INCLUDED_)
