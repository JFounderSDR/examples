################################################################################
# examples/common_utils
# Copyright 2019 JFounder, Inc.
# openSCA (Licensed Product) is protected by copyright, and is
# distributed under the following terms.
# 
# openSCA is an proprietary implementation of the Joint Tactical Networking
# center (JTNC) SOFTWARE COMMUNICATION ARCHITECTURE SPECIFICATION (SCA) 
# version2.2.2, developed and copyrighted by JFounder Inc. 
################################################################################
cmake_minimum_required(VERSION 3.5)

project(common_utils)

aux_source_directory (./src SRC)

add_library (${PROJECT_NAME} SHARED ${SRC})
