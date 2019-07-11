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

INCLUDE_DIRECTORIES("${CF_DIR}"
                  "${EXAMPLES_DIR}/common_utils/include")

add_subdirectory(${EXAMPLES_DIR}/common_utils)
add_subdirectory(${EXAMPLES_DIR}/U1_Zynq7035)
add_subdirectory(${EXAMPLES_DIR}/MsgTrans_Ctroller)
