/***************************************************************************//**
* @file     U1_Zynq7035_server.cpp
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

#include "../include/U1_Zynq7035_servant.h"
#include "debug.h"
#include "openscaSupport.h"
#include <cctype>
#include <assert.h>

void usage()
{
	DEBUG(0, [U1_Zynq7035_Main],
		" Example: U1_Zynq7035_Main -DEBUG <deubg level>.")
	DEBUG(0, [U1_Zynq7035_Main],
		" Example: U1_Zynq7035_Main -DEBUG 1.")
	DEBUG(0, [U1_Zynq7035_Main],
		" Example: U1_Zynq7035_Main -ID DCE:fewf-wfe-34ss-fsef:1.")
	DEBUG(0, [U1_Zynq7035_Main],
		" Example: U1_Zynq7035_Main -SFTFL device.spd.xml.")
	DEBUG(0, [U1_Zynq7035_Main],
		" Example: U1_Zynq7035_Main --help.")
}

/**
 * @brief check if input parameter is valid
 * @param[in] argc-			 total parameter num in main
 * @param[in] argvIndex-	 index of argv[] from 0 to argc-1
 *
 * @return	legality of the parameters.
 * @retval	true	parameters are legal
 * @retval	false	parameters are illegal
 */ 
bool checkInParam(int argc, int argvIndex)
{
	if (argvIndex+1 == argc) {
		DEBUG(0, [U1_Zynq7035_Main],
				" ERROR: illegal input parameter.")
		usage();
		return false;
	}
	return true;
}

#ifdef __SDS_OS_VXWORKS__
extern "C"
int U1_Zynq7035_Main(int argc, char * argv[])
#elif defined __SDS_OS_LINUX__
int main(int argc, char * argv[])
#endif
{
	DEBUG(7, [U1_Zynq7035_Main], " entering U1_Zynq7035_Main...")
			
	int debugLevel = 10;
	std::string id = "";
	std::string label = "";
	std::string sftfl = "";
	std::string nodeName = "";
	std::string devmgrName = "";
	std::string devName = "";
	
#ifdef __SDS_OS_VXWORKS__
	/// for shutdown device and release all resource
	pthread_cond_t 		shutdownCond;
	pthread_mutex_t 	shutdownMutex;
	pthread_mutex_init(&shutdownMutex, NULL);
	pthread_condattr_t 	condAttr;
	pthread_condattr_init(&condAttr);
	/// the flag to shutdown device
	bool shutdown = false;
	pthread_cond_init(&shutdownCond, &condAttr);
#endif

	///parse input parameter
	for (unsigned int numArg=0; numArg<argc; numArg++) {
		///parse DEUBG
		if (strcmp(argv[numArg],"-DEBUG") == 0) {
			if (checkInParam(argc, numArg) == false)
				exit(EXIT_FAILURE);

			if (isdigit(*argv[numArg+1])) {
				debugLevel = atoi(argv[numArg+1]);
			} else {
				DEBUG(0, [U1_Zynq7035_Main], " ERROR: illegal debug level given.")
				usage();
				exit(EXIT_FAILURE);
			}
		}

		///parse id
		if (strcmp(argv[numArg], "-ID") == 0) {
			if (checkInParam(argc, numArg) == false)
				exit(EXIT_FAILURE);

			id = argv[numArg+1];
			DEBUG(7, [U1_Zynq7035_Main], "id is " << id)
		}

		///parse label
		if (strcmp(argv[numArg], "-LABEL") == 0) {
			if (checkInParam(argc, numArg) == false)
				exit(EXIT_FAILURE);

			label = argv[numArg+1];
			devName = label;
			DEBUG(7, [U1_Zynq7035_Main], " label is " << label)
		}

		///parse software profile
		if (strcmp(argv[numArg], "-SFTFL") == 0) {
			if (checkInParam(argc, numArg) == false)
				exit(EXIT_FAILURE);

			sftfl = argv[numArg+1];
			DEBUG(7, [U1_Zynq7035_Main], "sftfl is " << sftfl)
		}
		
		///parse node name
		if (strcmp(argv[numArg], "-NODENAME") == 0) {
			if (checkInParam(argc, numArg) == false)
				exit(EXIT_FAILURE);

			nodeName = argv[numArg+1];
			DEBUG(7, [U1_Zynq7035_Main], "node name is " << nodeName)
		}
		
		///parse devicemanager name
		if (strcmp(argv[numArg], "-DEVMGRNAME") == 0) {
			if (checkInParam(argc, numArg) == false)
				exit(EXIT_FAILURE);

			devmgrName = argv[numArg+1];
			DEBUG(7, [U1_Zynq7035_Main], "devicemanager name is " << devmgrName)
		}
	}		///for (unsigned int numArg=0; numArg<argc; numArg++)

	/// instantiate Excutable_Device servant
#ifdef __SDS_OS_VXWORKS__
	U1_Zynq7035_servant* devServant = new U1_Zynq7035_servant(
											id.c_str(),
											label.c_str(),
											sftfl.c_str(),
											nodeName.c_str(),
											&shutdownCond);
#elif defined __SDS_OS_LINUX__
	U1_Zynq7035_servant* devServant = new U1_Zynq7035_servant(
											id.c_str(),
											label.c_str(),
											sftfl.c_str(),
											nodeName.c_str());
#endif
	/// obtain object reference of this device
	assert(devServant);

#ifdef __SDS_OS_VXWORKS__
	CF::ExecutableDevice_var devServant_var = devServant->_this();
#elif defined __SDS_OS_LINUX__
	CF::ExecutableDevice_var devServant_var =
		devServant->POA_CF::ExecutableDevice::_this();
#endif
	
	/// create device cosname in namingservice, then bing ojbect to device cosname  
	std::string cosName = "OpenSCA_Domain";
	cosName.append("/");
	cosName.append(nodeName.c_str());
	cosName.append("/");
	cosName.append(devmgrName.c_str());
	cosName.append("/");
	cosName.append(devName);
	/// instantiate orb
	openscaSupport::ORB_Wrap * orbWrap = new openscaSupport::ORB_Wrap();
	assert(orbWrap);

	if (CORBA::is_nil(devServant_var)) {
		DEBUG(0, [U1_Zynq7035_Main], " bind device " << devName
			<< " objref with name into nameservice failed")
		delete devServant;
		delete orbWrap;
		return -1;
	}
	
	if (orbWrap->bind_object_to_string(
		devServant_var.in(), cosName.c_str()) == false) {
		DEBUG(0, [U1_Zynq7035_Main], " bind device " << devName
			<< " objref with name into nameservice failed")
		delete devServant;
		delete orbWrap;
		return -1;
	}
	
	/// create device naming context by calling bind_new_context_with_string
	std::string compNaming = "OpenSCA_Domain/" + nodeName + "/" + devName;
	if (orbWrap->bind_new_context_with_string( compNaming.c_str()) == false)
		throw CF::LifeCycle::InitializeError();
	
	DEBUG(7, [U1_Zynq7035_server], " is running...")
	
#ifdef __SDS_OS_LINUX__
	try{
		DEBUG(7, [U1_Zynq7035_server], " orb is running...")
		// run orb
		orbWrap->orb->run();
		// shutdown orb
		DEBUG(7, [U1_Zynq7035_server], " orb is shutdowm...")
	}catch (...){
		DEBUG(0, [U1_Zynq7035_server], " run with unkown exception.")
	}
#endif
	
#ifdef __SDS_OS_VXWORKS__
	/// device server block here to wait shutdown command
	pthread_mutex_lock(&shutdownMutex);
	pthread_cond_wait(&shutdownCond, &shutdownMutex);
	pthread_mutex_unlock(&shutdownMutex);
	
	pthread_mutex_destroy(&shutdownMutex);
	pthread_condattr_destroy(&condAttr);
	pthread_cond_destroy(&shutdownCond);
#endif

	delete devServant;
	delete orbWrap;

	DEBUG(0, [U1_Zynq7035_Main], " leaving...")
	
	return 0;
}

