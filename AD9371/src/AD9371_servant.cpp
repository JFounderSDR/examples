/***************************************************************************//**
* @file     AD9371_servant.cpp
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

#include "../include/AD9371_servant.h"

AD9371_servant::AD9371_servant()
{
	DEBUG(7, AD9371_servant, "In default contructor.")
}

#ifdef __SDS_OS_VXWORKS__
AD9371_servant::AD9371_servant(
	const char * id,
	const char * lbl,
	const char * sftwfrl,
	const char * nodeName,
	pthread_cond_t * _shutdownCond) 
:Resource_impl(id)
{
	DEBUG(7, AD9371_servant,
		"In constructor with three input parameters")
				
	
	m_label = lbl;
	m_softwareProfile = sftwfrl;
	m_nodeName = nodeName;
	
	DEBUG(3, AD9371_servant,
		"In constructor with label is: " << m_label)
	DEBUG(3, AD9371_servant,
		"In constructor with softwareProfile is: " << m_softwareProfile)
	
	m_orbWrap = new openscaSupport::ORB_Wrap::ORB_Wrap();
	
	m_usageState = CF::Device::IDLE;
	m_operationalState = CF::Device::ENABLED;
	m_adminState = CF::Device::UNLOCKED;
	
	m_shutdownCond = _shutdownCond;
	pthread_mutex_init(&m_stateMtx, NULL);
	pthread_mutex_init(&m_attrMtx, NULL);
	
	DEBUG(7, AD9371_servant, "In constructor with three input parameters")
	DEBUG(7, AD9371_servant, "In constructor with id: " << id)
	DEBUG(7, AD9371_servant, "In constructor with lbl: " << lbl)
	DEBUG(7, AD9371_servant, "In constructor with sftwfrl: " << sftwfrl)
}

#elif defined __SDS_OS_LINUX__

AD9371_servant::AD9371_servant(
	const char * id,
	const char * lbl,
	const char * sftwfrl,
	const char * nodeName) 
:Resource_impl(id)
{
	DEBUG(7, AD9371_servant,
		"In constructor with three input parameters")
				
	
	m_label = lbl;
	m_softwareProfile = sftwfrl;
	m_nodeName = nodeName;
	
	DEBUG(3, AD9371_servant,
		"In constructor with label is: " << m_label)
	DEBUG(3, AD9371_servant,
		"In constructor with softwareProfile is: " << m_softwareProfile)
	
	m_orbWrap = new openscaSupport::ORB_Wrap();
	
	m_usageState = CF::Device::IDLE;
	m_operationalState = CF::Device::ENABLED;
	m_adminState = CF::Device::UNLOCKED;
	
	pthread_mutex_init(&m_stateMtx, NULL);
	pthread_mutex_init(&m_attrMtx, NULL);
	
	DEBUG(7, AD9371_servant, "In constructor with three input parameters")
	DEBUG(7, AD9371_servant, "In constructor with id: " << id)
	DEBUG(7, AD9371_servant, "In constructor with lbl: " << lbl)
	DEBUG(7, AD9371_servant, "In constructor with sftwfrl: " << sftwfrl)
}
#endif

AD9371_servant::~AD9371_servant()
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
AD9371_servant::adminState()
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
AD9371_servant::adminState(
	CF::Device::AdminType _adminType)
throw (
	CORBA::SystemException)
{
	DEBUG(5, [AD9371_servant::adminState], "In adminState.")
	pthread_mutex_lock(&m_stateMtx);

	if (_adminType == CF::Device::LOCKED) {
		if (m_adminState == CF::Device::UNLOCKED)
			m_adminState = CF::Device::LOCKED;
	} else if (_adminType == CF::Device::UNLOCKED) {
		if ((m_adminState == CF::Device::LOCKED)||
				m_adminState == CF::Device::SHUTTING_DOWN)
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
AD9371_servant::releaseObject()
throw (
	CF::LifeCycle::ReleaseError, 
	CORBA::SystemException)
{
	DEBUG(5, [AD9371_servant::releaseObject], "In releaseObject.")
	
	
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
	CORBA::Object_ptr obj =
		m_orbWrap->get_object_from_string(comp_naming.c_str());
	CosNaming::NamingContext_ptr nc = CosNaming::NamingContext::_nil();
	
	try {
		nc = CosNaming::NamingContext::_narrow(obj);
	} catch(CosNaming::NamingContext::InvalidName& ex) {
		DEBUG(0, [AD9371_servant::releaseObject],
			" occure InvalidName Exception.")
		throw CF::LifeCycle::ReleaseError();
	} catch(...) {
		DEBUG(0, [AD9371_servant::releaseObject],
			" occure Unknown Exception.")
		throw CF::LifeCycle::ReleaseError();
	}
	
	try {
		m_orbWrap->destory_context(nc);
	} catch(CosNaming::NamingContext::NotEmpty) {
		DEBUG(0, [AD9371_servant::releaseObject],
			" NamingContext to be destroy is not empty.")
		throw CF::LifeCycle::ReleaseError();
	} catch(...) {
		DEBUG(0, [AD9371_servant::releaseObject], " Unknown Exception.")
		throw CF::LifeCycle::ReleaseError();
	}
	
	///ubind name from domain
	try {
		m_orbWrap->unbind_string(comp_naming.c_str());
	} catch (...) {
		DEBUG(0, [AD9371_servant::releaseObject],
			" unbing_string with Unknown Exception.")
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
AD9371_servant::compositeDevice()
throw (
	CORBA::SystemException)
{
	DEBUG(5, [AD9371_servant::compositeDevice], " In compositeDevice.")
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
AD9371_servant::allocateCapacity(
	const CF::Properties& capacities)
throw (
	CF::Device::InvalidState, 
	CF::Device::InvalidCapacity, 
	CORBA::SystemException)
{	
	DEBUG(5, [AD9371_servant::allocateCapacity], "In allocateCapacity.")

	if ((adminState() != CF::Device::UNLOCKED) ||
			(operationalState() == CF::Device::DISABLED))
		throw CF::Device::InvalidState();
	
	if (usageState() == CF::Device::BUSY)
		throw CF::Device::InvalidCapacity();
	
/**************************OPENSCA-USERREGION-BEGIN*******************************/
	/// assume that one component will occupy whole FPGA resource, because FPGA
	/// just allow only one component run in the same chip
	if (capacities.length() > 0)
		setUsageState(CF::Device::BUSY);
	
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
 * 			to ACTIVE when,after adjusting capacities, any of the device capacities
 * 			are still being used.
 *
 * 			The deallocateCapacity operation shall set the usageState attribute to
 * 			IDLE when,after adjusting capacities, none of the device capacities
 * 			are still being used.
 *
 * @param[in]	capacities	capacities of device need to be deallocate.
 *
 * @exception 	The deallocateCapacity operation shall raise the InvalidCapacity
 * 			  	exception, when the capacity ID is invalid or the capacity value is
 *            	the wrong type. The InvalidCapacity exception msg parameter describes
 *            	the reason for the exception.
 *
 * 			  	The deallocateCapacity operation shall raise the InvalidState exception,
 * 			  	when the device adminState is LOCKED or operationalState is DISABLED.
 */
void 
AD9371_servant::deallocateCapacity(
	const CF::Properties& capacities)
throw (
	CF::Device::InvalidState, 
	CF::Device::InvalidCapacity, 
	CORBA::SystemException)
{
	DEBUG(5, [AD9371_servant::deallocateCapacity], "In deallocateCapacity.")
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
 * @brief 	The configure operation allows id/value pair configuration properties to
 *		  	be assigned to components implementing this interface.
 *			The configure operation shall assign values to the properties as indicated
 *  		in the input configProperties parameter. Valid properties for the configure
 *  		operation shall at a minimum be the configure readwrite and writeonly properties
 *  		referenced in the component SPD.
 *
 * @param[in]	configProperties properties need to be configured.
 *
 * @exception 	The configure operation shall raise a PartialConfiguration exception when
 *  			some configuration properties were successfully set and some configuration
 *  			properties were not successfully set.
 *				The configure operation shall raise an InvalidConfiguration exception when
 *				a configuration error occurs and no configuration properties were successfully
 *				set.
 */
void 
AD9371_servant::configure(
	const CF::Properties & configProperties)
throw (
	CF::PropertySet::PartialConfiguration,
	CF::PropertySet::InvalidConfiguration,
	CORBA::SystemException)
{
	DEBUG(5, [AD9371_servant::configure], " In configure.")
	
	pthread_mutex_lock(&m_attrMtx);

	if (1 == configProperties.length()) {
		CORBA::UShort rxGainControlMode = 0;
		for (CORBA::ULong i = 0; i < propSet.length(); ++i) {
			if (0 == strcmp(RX_GAIN_CONTROL_MODE, propSet[i].id)) {
				propSet[i].value >>= rxGainControlMode;
			}
		}

		if (0 == strcmp(RX1_RX_MGC_GAIN, configProperties[0].id)
				|| 0 == strcmp(RX2_RX_MGC_GAIN, configProperties[0].id)) {
			if (2 == rxGainControlMode) {
				pthread_mutex_unlock(&m_attrMtx);
				char *errText = "当前状态为自动增益，请将增益模式设置为手动增益!";
				throw CF::PropertySet::InvalidConfiguration(
					errText, configProperties);
			}
		}
	}

	try {
		PropertySet_impl::configure(configProperties);
	} catch (CF::PropertySet::PartialConfiguration & e) {
		DEBUG(0, [AD9371_servant::configure],
			"partial configuration exception.")
		pthread_mutex_unlock(&m_attrMtx);
		throw e;
	} catch (CF::PropertySet::InvalidConfiguration & e) {
		DEBUG(0, [AD9371_servant::configure],
			"invalid configuration exception.")
		pthread_mutex_unlock(&m_attrMtx);
		throw e;
	} catch (...) {
		DEBUG(0, [AD9371_servant::configure],
			"occur unkown exception when config." )
		pthread_mutex_unlock(&m_attrMtx);
		throw CF::PropertySet::InvalidConfiguration();
	}
/**************************OPENSCA-USERREGION-BEGIN*******************************/
	/// only readwrite and writeonly can be configured. if this properties need to
	/// be saved after configured, call PropertySet_impl::configure(props);
	CF::Properties props = configProperties;

	for (CORBA::ULong i = 0; i < props.length(); i++) {
		if (0 == strcmp(CLOCK_SOURCE, props[i].id)) {
			CORBA::UShort clockSource;
			props[i].value >>= clockSource;
			dwApiErr_t dwApi = dw_setClcokSource((dwClockSorce_t) clockSource);
			if (0 != dwApi) {
				DEBUG(0, [AD9371_servant::configure],
					" Setup clockSource failed.")
				pthread_mutex_unlock(&m_attrMtx);
				throw CF::PropertySet::InvalidConfiguration();
			}
			DEBUG(2, [AD9371_servant::configure],
				" Set up clockSource:" << clockSource)
		}
	}

	bool isClockOutput = false;
	for (CORBA::ULong i = 0; i < props.length(); i++) {
		if (0 == strcmp(OUTPUT_CLK_SELECT, props[i].id)) {
			CORBA::UShort outPutClk;
			props[i].value >>= outPutClk;
			dwApiErr_t dwApi = dw_setClockOutput(
					(dwOutputClkSelect_t) outPutClk);

			if (0 != dwApi) {
				DEBUG(0, [AD9371_servant::configure],
					" Setup sampling rate failed.")
				pthread_mutex_unlock(&m_attrMtx);
				throw CF::PropertySet::InvalidConfiguration();
			}
			DEBUG(2, [AD9371_servant::configure],
					" Set up sampling rate:" << outPutClk)

			dwApi = dw_initDevice(NULL);
			if (0 != dwApi) {
				DEBUG(0, [AD9371_servant::configure],
					" initialize failed.")
				pthread_mutex_unlock(&m_attrMtx);
				throw CF::PropertySet::InvalidConfiguration();
			}
			isClockOutput = true;
		}
	}
	if (isClockOutput) {
		configureAD9371(propSet);
	} else {
		configureAD9371(props);
	}
/**************************OPENSCA-USERREGION-END*********************************/
	pthread_mutex_unlock(&m_attrMtx);
}

/**
 * @brief 	The configure operation allows id/value pair configuration AD9371 device properties
 *         	to be assigned to components implementing this interface.Valid properties
 *			for the configure operation shall at a minimum be the configure
 *			readwrite and writeonly properties referenced in the component SPD.
 *
 * @param[in]	configProperties properties of AD9371 device need to be configured.
 */
void
AD9371_servant::configureAD9371(
	const CF::Properties & configProperties)
{
	CF::Properties props = configProperties;

	DEBUG(5, [AD9371_servant::configureAD9371],
		" props.length():" << props.length())
	for (CORBA::ULong i = 0; i < props.length(); i++) {
		if (0 == strcmp(RX_RF_PLL_FREQUENCY, props[i].id)) {
			CORBA::Float rxRfPllFrequency;
			props[i].value >>= rxRfPllFrequency;
			dwApiErr_t dwApi = dw_setRfPllFrequency(DW_RX_PLL,
					rxRfPllFrequency);
			if (0 != dwApi) {
				DEBUG(0, [AD9371_servant::configureAD9371],
					" Setup receive frequency failed.")
				pthread_mutex_unlock(&m_attrMtx);
				throw CF::PropertySet::InvalidConfiguration();
			}
			DEBUG(2, [AD9371_servant::configureAD9371],
					" Set up receive frequency:" << rxRfPllFrequency)

		} else if (0 == strcmp(TX_RF_PLL_FREQUENCY, props[i].id)) {
			CORBA::Float txRfPllFrequency;
			props[i].value >>= txRfPllFrequency;

			dwApiErr_t dwApi = dw_setRfPllFrequency(DW_TX_PLL,
					txRfPllFrequency);
			if (0 != dwApi) {
				DEBUG(0, [AD9371_servant::configureAD9371],
					" Setup send frequency failed.")
				pthread_mutex_unlock(&m_attrMtx);
				throw CF::PropertySet::InvalidConfiguration();
			}
			DEBUG(2, [AD9371_servant::configureAD9371],
					" Set up send frequency:" << txRfPllFrequency)

		} else if (0 == strcmp(RX_GAIN_CONTROL_MODE, props[i].id)
				|| 0 == strcmp(RX1_RX_MGC_GAIN, props[i].id)
				|| 0 == strcmp(RX2_RX_MGC_GAIN, props[i].id)) {

			configureRxGainMode(props[i]);

		} else if (0 == strcmp(TX1_TX_ATTENUATION, props[i].id)) {
			CORBA::Float tx1TxAttenuation;
			props[i].value >>= tx1TxAttenuation;

			dwApiErr_t dwApi = dw_setTxAttenuation(DW_RF_TX1, tx1TxAttenuation);
			if (0 != dwApi) {
				DEBUG(0, [AD9371_servant::configureAD9371],
					" Setup TX1 emission attenuation failed.")
				pthread_mutex_unlock(&m_attrMtx);
				throw CF::PropertySet::InvalidConfiguration();
			}
			DEBUG(2, [AD9371_servant::configureAD9371],
					" Set up TX1 emission attenuation:" << tx1TxAttenuation)

		} else if (0 == strcmp(TX2_TX_ATTENUATION, props[i].id)) {
			CORBA::Float tx2TxAttenuation;
			props[i].value >>= tx2TxAttenuation;

			dwApiErr_t dwApi = dw_setTxAttenuation(DW_RF_TX2, tx2TxAttenuation);
			if (0 != dwApi) {
				DEBUG(0, [AD9371_servant::configureAD9371],
					" Setup TX2 emission attenuation failed.")
				pthread_mutex_unlock(&m_attrMtx);
				throw CF::PropertySet::InvalidConfiguration();
			}
			DEBUG(2, [AD9371_servant::configureAD9371],
					" Set up TX2 emission attenuation:" << tx2TxAttenuation)

		} else if (0 == strcmp(RF_ENABLE_DW_RF_TX1, props[i].id)) {
			int selectValue = 0x1;
			configureAD9371Enable(props[i], selectValue);

			DEBUG(2, [AD9371_servant::configureAD9371],
					" Set up TX1 enable:" << selectValue)

		} else if (0 == strcmp(RF_ENABLE_DW_RF_TX2, props[i].id)) {
			int selectValue = 0x2;
			configureAD9371Enable(props[i], selectValue);

			DEBUG(2, [AD9371_servant::configureAD9371],
					" Set up TX2 enable:" << selectValue)

		} else if (0 == strcmp(RF_ENABLE_DW_RF_RX1, props[i].id)) {
			int selectValue = 0x4;
			configureAD9371Enable(props[i], selectValue);

			DEBUG(2, [AD9371_servant::configureAD9371],
					" Set up RX1 enable:" << selectValue)

		} else if (0 == strcmp(RF_ENABLE_DW_RF_RX2, props[i].id)) {
			int selectValue = 0x8;
			configureAD9371Enable(props[i], selectValue);

			DEBUG(2, [AD9371_servant::configureAD9371],
					" Set up RX2 enable:" << selectValue)
		}
	}
}

/**
 * @brief 	The configure operation allows id/value pair configuration AD9371 device
 * 			gain properties to be assigned to components implementing this interface.
 * 			Valid properties for the configure operation shall at a minimum be the configure
 *			readwrite and writeonly properties referenced in the component SPD.
 *
 * @param[in]	configProperty  gain properties need to be configured for AD9371 device.
 */
void
AD9371_servant::configureRxGainMode(
	const CF::DataType & configProperty)
{
	CORBA::UShort rxGainControlMode = 2;
	CORBA::Float rx1RxMGCGain = 0;
	CORBA::Float rx2RxMGCGain = 0;
	string rx1RxMGCGainID = "";
	string rx2RxMGCGainID = "";

	if (0 == strcmp(RX_GAIN_CONTROL_MODE, configProperty.id)) {
		configProperty.value >>= rxGainControlMode;
		dwApiErr_t dwApi = dw_setRxGainControlMode(
				(dwRxGainMode_t) rxGainControlMode);
		if (0 != dwApi) {
			DEBUG(0, [AD9371_servant::configureAD9371],
				" Setup gain control mode failed.")
			pthread_mutex_unlock(&m_attrMtx);
			throw CF::PropertySet::InvalidConfiguration();
		}
		DEBUG(2, [AD9371_servant::configureRxGainMode],
				" Set up gain control mode:" << rxGainControlMode)
	}

	for (CORBA::ULong i = 0; i < propSet.length(); ++i) {
		if (0 == strcmp(RX_GAIN_CONTROL_MODE, propSet[i].id)) {
			propSet[i].value >>= rxGainControlMode;
		} else if (0 == strcmp(RX1_RX_MGC_GAIN, propSet[i].id)) {
			rx1RxMGCGainID = propSet[i].id;
			propSet[i].value >>= rx1RxMGCGain;
		} else if (0 == strcmp(RX2_RX_MGC_GAIN, propSet[i].id)) {
			rx2RxMGCGainID = propSet[i].id;
			configProperty.value >>= rx2RxMGCGain;
		}
	}

	if (0 != rxGainControlMode) {
		return;
	}

	if (0 == strcmp(RX1_RX_MGC_GAIN, rx1RxMGCGainID.c_str())) {
		configProperty.value >>= rx1RxMGCGain;
		dwApiErr_t dwApi = dw_setRxMGCGain(DW_RF_RX1, rx1RxMGCGain);
		if (0 != dwApi) {
			DEBUG(0, [AD9371_servant::configureRxGainMode],
				" Setup RX1 gain failed.")
			pthread_mutex_unlock(&m_attrMtx);
			throw CF::PropertySet::InvalidConfiguration();
		}
		DEBUG(2, [AD9371_servant::configureRxGainMode],
				" Set up RX1 gain:" << rx1RxMGCGain)
	}

	if (0 == strcmp(RX2_RX_MGC_GAIN, rx2RxMGCGainID.c_str())) {
		configProperty.value >>= rx2RxMGCGain;
		dwApiErr_t dwApi = dw_setRxMGCGain(DW_RF_RX2, rx2RxMGCGain);
		if (0 != dwApi) {
			DEBUG(0, [AD9371_servant::configureRxGainMode],
				" Setup RX2 gain failed.")
			pthread_mutex_unlock(&m_attrMtx);
			throw CF::PropertySet::InvalidConfiguration();
		}
		DEBUG(2, [AD9371_servant::configureRxGainMode],
				" Set up RX2 gain:" << rx2RxMGCGain)
	}
}

void
AD9371_servant::configureAD9371Enable(
	const CF::DataType & configProperty,
	int selectValue){
	CORBA::Boolean rfEnable;
	configProperty.value >>= CORBA::Any::to_boolean(rfEnable);

	if(rfEnable){
		dwApiErr_t dwApi = dw_setRfEnable((dwRfChannels_t) selectValue);
		if (0 != dwApi) {
			DEBUG(0, [AD9371_servant::configureAD9371],
				" Set up transceiver enable failed.")
			pthread_mutex_unlock(&m_attrMtx);
			throw CF::PropertySet::InvalidConfiguration();
		}
	}else{
		dwApiErr_t dwApi = dw_setRfDisable((dwRfChannels_t) selectValue);
		if (0 != dwApi) {
			DEBUG(0, [AD9371_servant::configureAD9371],
				" Set up transceiver disable failed.")
			pthread_mutex_unlock(&m_attrMtx);
			throw CF::PropertySet::InvalidConfiguration();
		}
	}
}

/**
 * @brief 	The purpose of the initialize operation is to provide a mechanism to set
 * 		  	a component to a known initial state. For example, data structures may be
 * 		  	set to initial values, memory may be allocated, hardware devices may be
 * 		  	configured to some state, etc.
 *
 * @exception 	The initialize operation shall raise an InitializeError exception when
 *  		  	an initialization error occurs.
 */
void 
AD9371_servant::initialize()
throw (
	CF::LifeCycle::InitializeError, 
	CORBA::SystemException)
{
	DEBUG(5, [AD9371_servant::initialize], " In initialize.")
	
			
	
    m_compositeDevice = CF::AggregateDevice::_nil();
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
 * @retval 	the pointer to label.
 */
char * 
AD9371_servant::label()
throw (
	CORBA::SystemException)
{
	DEBUG(9, [AD9371_servant::label], " In label.")
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
AD9371_servant::operationalState()
throw (
	CORBA::SystemException)
{
	DEBUG(5, [AD9371_servant::operationalState], "In operationalState.")
	pthread_mutex_lock(&m_stateMtx);
	CF::Device::OperationalType _operatState = m_operationalState;
	pthread_mutex_unlock(&m_stateMtx);
	
	return m_operationalState;
}

/**
 * @brief 	The query operation allows a component to be queried to retrieve its properties.
 * 			The query operation shall return all component properties when the inout
 * 			parameter configProperties is zero size. The query operation shall return only
 * 			those id/value pairs specified in the configProperties parameter if the parameter
 * 			is not zero size. Valid properties for the query operation shall be all configure
 * 			properties (simple properties whose kind element kindtype attribute is "configure"
 * 			whose mode attribute is "readwrite" or "readonly" and any allocation properties
 * 			with an action value of "external" as referenced in the component SPD.
 *
 * @param[inout]	props	properties need to be queried.
 *
 * @exception The query operation shall raise the CF UnknownProperties exception when one
 * 			  or more properties being requested are not known by the component.
 */
void 
AD9371_servant::query(
	CF::Properties & props)
throw (
	CF::UnknownProperties,
	CORBA::SystemException)
{
	DEBUG(5, [AD9371_servant::query], " In query.")
	
	pthread_mutex_lock(&m_attrMtx);
	PropertySet_impl::query(props);
/**************************OPENSCA-USERREGION-BEGIN*******************************/
	/// fill those properties of which value need to be aquired from driver interface

	
/**************************OPENSCA-USERREGION-END*********************************/
	pthread_mutex_unlock(&m_attrMtx);
}

/**
 * @brief 	The runTest operation allows components to be "black box" tested. This
 * 			allows built-in tests (BITs) to be implemented which provide a means to
 * 			isolate faults (both software and hardware) within the system.
 * 			The runTest operation shall use the input testId parameter to determine
 * 			which of its predefined test implementations should be performed. The
 * 			id/value pair(s) of the testValues parameter shall be used to provide
 * 			additional information to the implementation-specific test to be run.
 * 			The runTest operation shall return the result(s) of the test in the
 * 			testValues parameter. Tests to be implemented by a component are
 * 			component-dependent and are specified in the component Properties Descriptor.
 * 			Valid testId(s) and both input and output testValues (properties) for the
 * 			runTest operation shall at a minimum be the test properties defined in
 * 			the properties test element of the component Properties Descriptor
 * 			(refer to Appendix D Domain Profile). The testId parameter corresponds to
 * 			the XML attribute testId of the property element test in a propertyfile.
 *
 * 			A CF UnknownProperties exception is raised by the runTest operation. All
 * 			testValues parameter properties (i.e., test properties defined in the
 * 			propertyfile(s) referenced in the component SPD) shall be validated.
 *
 * 			The runTest operation shall not execute any testing when the input testId
 *			or any of the input testValues are not known by the device or are out of range.
 *
 * param[inout]	testValues	Specific which test shall be executed by testId, and store
 * 				the test value.
 *
 * @exception 	The runTest operation shall raise the UnknownTest exception when there
 * 				is no underlying test implementation that is associated with the input
 * 				testId given.
 * 				The runTest operation shall raise the CF UnknownProperties exception
 * 				when the input parameter testValues contains any CF DataTypes that are
 * 				not known by the component test implementation or any values that are
 * 				out of range for the requested test. The exception parameter invalidProperties
 * 				shall contain the invalid testValues properties id(s) that are not known by
 * 				the component or the value(s) are out of range.
 */
void 
AD9371_servant::runTest(
	CORBA::ULong TestID, 
	CF::Properties & testValues)
throw (
	CF::UnknownProperties, 
	CF::TestableObject::UnknownTest, 
	CORBA::SystemException)
{
	DEBUG(5, [AD9371_servant::runTest], " In runTest.")
/**************************OPENSCA-USERREGION-BEGIN*******************************/


/**************************OPENSCA-USERREGION-END*********************************/
}

/**
 * @brief 	This routine return softwareProfile attrbute value
 * 			The softwareProfile attribute contains the Profile Descriptor for the
 * 			application that is created by the application factory.
 * 			The readonly softwareProfile attribute shall contain a profile element
 * 			(Profile Descriptor) with a file reference to the device SPD file.
 * 			Files referenced within the profile are obtained via FileManager.
 *
 * @return	device SPD file relative to file mount point.
 */
char * 
AD9371_servant::softwareProfile()
throw (
	CORBA::SystemException)
{
	DEBUG(5, [AD9371_servant::softwareProfile], " In softwareProfile.")
			
	return  CORBA::string_dup(m_softwareProfile.c_str());
}

/**
 * @brief The start operation is provided to command the resource implementing
 *		  this interface to start internal processing.The start operation puts
 *		  the resource in an operating condition.
 *
 * @exception 	StartError The start operation shall raise the StartError
 * 				exception if an error occurs while starting the device.
 */
void 
AD9371_servant::start()
throw (
	CF::Resource::StartError, 
	CORBA::SystemException)
{
	DEBUG(5, [AD9371_servant::start], " In start.")
	setOperationalState(CF::Device::ENABLED);
/**************************OPENSCA-USERREGION-BEGIN*******************************/
	configure(propSet);

/**************************OPENSCA-USERREGION-END*********************************/
}

/**
 * @brief 	The stop operation is provided to command the resource implementing
 * 		  	this interface to stop internal processing.
 * 			The stop operation shall disable all current operations and put the
 * 	 		resource in a non-operating condition. The stop operation shall not
 * 	 		inhibit subsequent configure, query, and start operations.
 *
 * @exception 	StopError The start operation shall raise the StopError exception
 * 				if an error occurs while stopping the device.
 */
void 
AD9371_servant::stop()
throw (
	CF::Resource::StopError, 
	CORBA::SystemException)
{
	DEBUG(5, [AD9371_servant::stop], " In stop.")
	setOperationalState(CF::Device::DISABLED);
/**************************OPENSCA-USERREGION-BEGIN*******************************/


/**************************OPENSCA-USERREGION-END*********************************/
}

/**
 * @brief 	This routine return usageState attribute value
 * 		  	The readonly usageState attribute shall contain the device usage state
 * 			(IDLE, ACTIVE, or BUSY). UsageState indicates whether or not a device is
 * 			actively in use at a specific instant, and if so, whether or not it has
 * 			spare capacity for allocation at that instant.
 *
 * @return 	UsageType of device.
 */
CF::Device::UsageType 
AD9371_servant::usageState()
throw (
	CORBA::SystemException)
{
	DEBUG(9, [AD9371_servant::usageState], "In usageState.")
	pthread_mutex_lock(&m_stateMtx);
	CF::Device::UsageType _usageState = m_usageState;
	pthread_mutex_unlock(&m_stateMtx);
	
	return _usageState;
}

/**
 * @brief 	The getPort operation provides a mechanism to obtain a specific consumer
 * 		  	or producer port, returns the object reference to the named port as stated
 * 		  	in the component SCD.
 *
 * @param[in] 	portName-name references to the port user want to get.
 *
 * @return		the references of the port object user want to get.
 *
 * @exception  	The getPort operation shall raise an UnknownPort exception if the
 * 				port name is invalid.
 */
CORBA::Object_ptr 
AD9371_servant::getPort(
	const char * portName)
throw (
	CORBA::SystemException, 
	CF::PortSupplier::UnknownPort)
{
    DEBUG(6, [AD9371_servant::getPort], "In getPort.")
		
	CORBA::Object_var _port;
	
	std::string portFullName = "OpenSCA_Domain/" + m_nodeName + "/" + portName;

	
	///don't find any port named by protName
	throw CF::PortSupplier::UnknownPort();
}

/**
 * @brief	This funtion used to set device usage state.
 *
 * @param[in]	newUsageState	new usage state the device will be.
 */
void 
AD9371_servant::setUsageState(
	CF::Device::UsageType newUsageState)
{
	DEBUG(5, [AD9371_servant::setUsageState], "In setUsageState.")
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
AD9371_servant::setOperationalState(
	CF::Device::OperationalType newOperationalState)
{
	DEBUG(5, [AD9371_servant::setOperationalState], "In setOperationalState.")
	pthread_mutex_lock(&m_stateMtx);
	m_operationalState = newOperationalState;
	pthread_mutex_unlock(&m_stateMtx);
}

