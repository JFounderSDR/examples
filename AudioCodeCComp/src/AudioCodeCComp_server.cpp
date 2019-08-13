/***************************************************************************//**
* @file     AudioCodeCComp_server.cpp
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
#include "debug.h"
#include "openscaSupport.h"
#include "utils.h"
#include "ConfigParser.h"
#include "Boost_utils.h"

#include <pthread.h>
#include <cctype>
#include <assert.h>

void usage()
{
	DEBUG(0, [AudioCodeCComp_Main], 
		" Example: AudioCodeCComp_Main -DEBUG <deubg level>.")
	DEBUG(0, [AudioCodeCComp_Main], 
		" Example: AudioCodeCComp_Main -DEBUG 1.")
	DEBUG(0, [AudioCodeCComp_Main], 
		" Example: AudioCodeCComp_Main -ID DCE:fewf-wfe-34ss-fsef:1.")
	DEBUG(0, [AudioCodeCComp_Main], 
		" Example: AudioCodeCComp_Main -SFTFL device.spd.xml.")
	DEBUG(0, [AudioCodeCComp_Main], 
		" Example: AudioCodeCComp_Main --help.")
}
	
/**
 * @brief	check if input parameter is valid
 * 
 * @param[in]	argc		total parameter num in main
 * @param[in]	argvIndex	index of argv[] from 0 to argc-1
 
 * @return	legality of the parameters.
 * @retval	true	parameters are legal
 * @retval	false	parameters are illegal
 */ 
bool checkInParam(int argc, int argvIndex)
{
	if (argvIndex+1 == argc) {
		DEBUG(0, [AudioCodeCComp_Main], 
			" ERROR: illegal input parameter.")
		usage();
		return false;
	}
	return true;
}

#ifdef __SDS_OS_VXWORKS__
extern "C"
int AudioCodeCComp_Main(int argc, char * argv[])
#elif defined __SDS_OS_LINUX__
int main(int argc, char * argv[])
#endif
{
	DEBUG(7, [AudioCodeCComp_Main], "entering AudioCodeCComp_Main...")
			
	int debugLevel = 10;
	std::string id = "";
	std::string cosNaming = "";
	std::string sftfl = "";
	std::string appName = "";
	std::string fsroot = "";

#ifdef __SDS_OS_VXWORKS__
	/// for shutdown device and release all resource
	pthread_cond_t 		shutdownCond;
	pthread_mutex_t 	shutdownMutex;
	pthread_mutex_init(&shutdownMutex, NULL);
	/// the flag to shutdown device
	bool shutdown = false;
	pthread_cond_init(&shutdownCond, NULL);
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
				DEBUG(0, [AudioCodeCComp_Main], 
					"ERROR: illegal debug level given.")
				usage();
				exit(EXIT_FAILURE);
			}
		}

		///parse id
		if (strcmp(argv[numArg], "-ID") == 0) {
			if (checkInParam(argc, numArg) == false)
				exit(EXIT_FAILURE);

			id = argv[numArg+1];
			DEBUG(7, [AudioCodeCComp_Main], "id is " << id)
		}

		///parse cosNaming
		if (strcmp(argv[numArg], "-COS_NAMING") == 0) {
			if (checkInParam(argc, numArg) == false)
				exit(EXIT_FAILURE);

			cosNaming = argv[numArg+1];
			DEBUG(7, [AudioCodeCComp_Main], " cosNaming is " << cosNaming)
		}

		///parse appName
		if (strcmp(argv[numArg], "-APP_NAME") == 0) {
			if (checkInParam(argc, numArg) == false)
				exit(EXIT_FAILURE);

			appName = argv[numArg+1];
			DEBUG(7, [AudioCodeCComp_Main], " appName is " << appName)
		}

		///parse software profile
		if (strcmp(argv[numArg], "-SFTFL") == 0) {
			if (checkInParam(argc, numArg) == false)
				exit(EXIT_FAILURE);

			sftfl = argv[numArg+1];
			DEBUG(7, [AudioCodeCComp_Main], "sftfl is " << sftfl)
		}
	}		///for (unsigned int numArg=0; numArg<argc; numArg++)

    char openScaPath[64];
    getConfigFilePathFromSHM(openScaPath, sizeof(openScaPath));
  	ConfigParser configParser(openScaPath);
	fsroot = configParser.getValueById(CONSTANT::FSROOT);

	///instantiate orb
	openscaSupport::ORB_Wrap * orbWrap = new openscaSupport::ORB_Wrap();
	///instantiate component servant
#ifdef __SDS_OS_VXWORKS__
	AudioCodeCComp_servant * compServant = new AudioCodeCComp_servant(
											id.c_str(), 
											cosNaming.c_str(), 
											appName.c_str(),
											sftfl.c_str(), 
											fsroot.c_str(), 
											&shutdownCond);
#elif defined __SDS_OS_LINUX__
	AudioCodeCComp_servant * compServant = new AudioCodeCComp_servant(
											id.c_str(), 
											cosNaming.c_str(), 
											appName.c_str(),
											sftfl.c_str(), 
											fsroot.c_str());
#endif
	assert(orbWrap);
	assert(compServant);
	//construct coomponent cosname in namingservice
	std::string cosName = "OpenSCA_Domain/Applications";
	cosName.append("/");
	cosName.append(appName);
	cosName.append("/");
	cosName.append(cosNaming);

	DEBUG(7, [AudioCodeCComp_Main], " begin to bind object with " << cosName)
	CF::Resource_var gppComp_v= compServant->_this();
	if (orbWrap->bind_object_to_string(
			gppComp_v.in(), cosName.c_str()) == false) {
		DEBUG(0, [AudioCodeCComp_Main], "bind object with string failed.")
		delete orbWrap;
		delete compServant;
		return -1;
	}
	
	///create component port naming context by call bind_new_context_with_string
	std::string compNaming = "OpenSCA_Domain/" + appName + "/" + cosNaming;
	if( false == orbWrap->bind_new_context_with_string( compNaming.c_str()))
		throw CF::LifeCycle::InitializeError();
	
	DEBUG(7, [AudioCodeCComp_server], " is running...")
	
#ifdef __SDS_OS_LINUX__
	try {
		DEBUG(7, [AudioCodeCComp_server], " orb is running...")
		// run orb
		orbWrap->orb->run();
		// shutdown orb
		DEBUG(7, [AudioCodeCComp_server], " orb is shutdowm...")
	} catch (...) {
		DEBUG(0, [AudioCodeCComp_server], " run with unkown exception.")
	}
#endif

#ifdef __SDS_OS_VXWORKS__
	/// device server block here to wait shutdown command
	pthread_mutex_lock(&shutdownMutex);
	pthread_cond_wait(&shutdownCond, &shutdownMutex);
	pthread_mutex_unlock(&shutdownMutex);
	
	pthread_mutex_destroy(&shutdownMutex);
    pthread_cond_destroy(&shutdownCond);
#endif
			
	delete compServant;
	delete orbWrap;

	DEBUG(0, [AudioCodeCComp_Main], " leaving ...")
	
	return 0;
}
