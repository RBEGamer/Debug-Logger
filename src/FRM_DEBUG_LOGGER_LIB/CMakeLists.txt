cmake_minimum_required(VERSION 2.7)
project(FRM_DEBUG_LOGGER_LIB)
ADD_LIBRARY( lib_debug_logger STATIC debug_logger.cpp)
TARGET_LINK_LIBRARIES( lib_debug_logger )
