################################################################################
# openSCA/frameworks
# Copyright 2019 JFounder, Inc.
# openSCA (Licensed Product) is protected by copyright, and is
# distributed under the following terms.
# 
# openSCA is an proprietary implementation of the Joint Tactical Networking
# center (JTNC) SOFTWARE COMMUNICATION ARCHITECTURE SPECIFICATION (SCA) 
# version2.2.2, developed and copyrighted by JFounder Inc. 
################################################################################

#Version required
cmake_minimum_required(VERSION 3.5)

SET(LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/libs/examples/")
SET(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/libs/examples/")

SET(CF_DIR "${PROJECT_SOURCE_DIR}/include/CF")
SET(EXAMPLES_DIR "${PROJECT_SOURCE_DIR}/examples")

link_libraries(
       "${PROJECT_SOURCE_DIR}/examples/libu1-sdr-rf.so"
)

INCLUDE_DIRECTORIES("${CF_DIR}"
                  "${EXAMPLES_DIR}/common_utils/include")


add_subdirectory(${EXAMPLES_DIR}/common_utils)
add_subdirectory(${EXAMPLES_DIR}/AD9371)
add_subdirectory(${EXAMPLES_DIR}/Zynq7035)
add_subdirectory(${EXAMPLES_DIR}/Zynq7035_PS)
add_subdirectory(${EXAMPLES_DIR}/Zynq7035_PL)
add_subdirectory(${EXAMPLES_DIR}/MsgTrans_Ctroller)
add_subdirectory(${EXAMPLES_DIR}/AudioTrans_Ctroller)
add_subdirectory(${EXAMPLES_DIR}/CRCComp)
add_subdirectory(${EXAMPLES_DIR}/AudioCodeCComp)
add_subdirectory(${EXAMPLES_DIR}/ImageTrans_Ctroller)
add_subdirectory(${EXAMPLES_DIR}/RxTxComp)