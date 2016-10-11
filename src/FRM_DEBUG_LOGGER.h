//
//  mo_debug_logger.hpp
//  optimiser
//
//  Created by Marcel Ochsendorf on 24.03.16.
//  Copyright © 2016 Marcel Ochsendorf. All rights reserved.
//

#ifndef FRM_DEBUG_LOGGER
#define FRM_DEBUG_LOGGER

#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <stdlib.h>
#include <iostream>
#include "FRM_DEFINES.h"
#if defined(FRM_OS_WIN)
#include <time.h>

//NETWORK

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#include <windows.h>
#include <shellapi.h>
#pragma comment(lib, "Shell32.lib")
#else
//TIME
#include <sys/time.h>  
//NETWORK
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
//THREAD
#include<pthread.h>
#endif


//function #def
//info
#define log_info(_tag, _reason) log(FRM::debug_logger::DEBUG_LOGGER_DEBUG_LEVELS::DL_INFO, _tag, _reason, __FUNCTION__, __LINE__)
//warning
#define log_warning(_tag, _reason) log(FRM::debug_logger::DEBUG_LOGGER_DEBUG_LEVELS::DL_WARNING, _tag, _reason, __FUNCTION__, __LINE__)
//error
#define log_error(_tag, _reason) log(FRM::debug_logger::DEBUG_LOGGER_DEBUG_LEVELS::DL_ERROR, _tag, _reason, __FUNCTION__, __LINE__)

#define get_debug_logger FRM::debug_logger::get_instance
//AT DESTRUCT save a html version if DEBUG_LOGGER_AUTO_SAVE defined
#define DEBUG_LOGGER_AUTO_SAVE
#define DEBUG_LOGGER_AUTO_SAVE_DIR_UNIX "./log/"
#define DEBUG_LOGGER_AUTO_SAVE_DIR_WIN "\log\\"

namespace FRM {
	class debug_logger {

	public:
		//DEBUG LEVELS ggf add one
		enum DEBUG_LOGGER_DEBUG_LEVELS {
			DL_INFO, DL_WARNING, DL_ERROR
		};


		//FUNCTIONS
		~debug_logger();
		//get instance
		static debug_logger* get_instance();
		//main log function see #defs
		void log(const DEBUG_LOGGER_DEBUG_LEVELS _level, const std::string _tag, const std::string _reason, const std::string _function = "", const size_t _line = -1);
		//switch / create section
		void switch_section(const std::string _section_name);
		//export functions
		void save_as_html(const char* _path, bool _open_file = false);
		void save_as_xml(const char* _path, bool _open_file = false);
		//generator functions function change pointer
		void generate_html(std::string* _string);
		void generate_xml(std::string* _string);
		//open exported file in explorer
		void open_file_in_browser(const char* _path);
		//webserver functions
		void start_webserver(const unsigned int _port, const std::string _this_ip);
		void stop_webserver();
		//CHART FUNCTIONS
		void store_chart_value(const int _value, const std::string _chartname);
		void set_chart_info(const std::string _chartname, const std::string _label_x_axis, const std::string _label_y_axis);
	
	
		unsigned int webserver_port;
		bool webserver_running;
		std::string local_ip;

#ifdef FRM_OS_WIN
		HANDLE lock;
#endif
		
	private:
		//functions
		debug_logger();
#ifdef FRM_OS_UNIX_BASED
		static void* webserver_thread_func(void* arg); //runs the webserver
#endif

#ifdef FRM_OS_WIN
		static DWORD WINAPI webserver_thread_func(LPVOID lpParameter);
#endif
		 //creates a http request
		void webserver_create_response(std::string* _result);
		
		//a log entry
		struct DEBUG_LOGGER_DATA_SET {
			DEBUG_LOGGER_DEBUG_LEVELS level = DEBUG_LOGGER_DEBUG_LEVELS::DL_INFO;
			size_t position = -1;
			size_t data_set_id = -1;
			std::string tag = "";
			std::string reason = "";
			std::string function = "";
			size_t line = -1;
			time_t timestamp = 0;
		};
		//a log section filled store entry
		struct DEBUG_LOGGER_SECTION {
			std::string section_desc = "";
			size_t level_count_error = 0;
			size_t level_count_warning = 0;
			size_t level_count_info = 0;
			std::vector<DEBUG_LOGGER_DATA_SET> entry;
		};
		//current selected section
		std::string current_section = "";
		//instance
		static debug_logger* instance;
		//map of sections
		std::map<std::string, DEBUG_LOGGER_SECTION> sections;
		//some statistics
		time_t startup_time;
		size_t section_count;
		size_t data_set_count;
		//CHART VARS
		//a chart point
		struct CHART_POINT_DESC {
			time_t timestamp;
			int value;
		};
		//a chart
		struct CHART_DESC {
			std::vector<CHART_POINT_DESC> points;
			std::string chart_name;
			std::string x_axis_label;
			std::string y_axis_label;
		};
		//chart map
		std::map<std::string, CHART_DESC> charts; //stores the charts
												  //WEBSERVER VARS

#if defined(FRM_OS_WIN)
		DWORD webserver_thread;
		HANDLE webserver_thread_handle;
		
		WSADATA wsaData;
		int iResult;
		
#else
		int listen_socket;
		struct sockaddr_in server_addr;
		pthread_t webserver_thread;
		pthread_mutex_t lock;
#endif



		const std::string canvas_181_js_min = "";
	};
}
#endif /* mo_debug_logger_hpp */
