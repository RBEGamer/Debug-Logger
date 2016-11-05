//
//  frm_debug_logger.hpp
//  FRM_DEBUG_LOGGER
//
//  Created by Marcel Ochsendorf on 04.11.16.
//  Copyright © 2016 Marcel Ochsendorf. All rights reserved.
//

#ifndef frm_debug_logger_hpp
#define frm_debug_logger_hpp


#define DEBUG_LOGGER_INCLUDE_WEBSERVER

#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <stdlib.h>
#include <iostream>

//WINDOWS
#if defined(MO_WIN) || defined(_WIN) || defined(_WIN32) || defined(_WIN64) || defined(__WIN__)
#include <time.h>
#include <windows.h>
//NETWORK
#ifdef DEBUG_LOGGER_INCLUDE_WEBSERVER
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")
#endif
//CALLSTACK
#include "DbgHelp.h"
#include <WinBase.h>
#pragma comment(lib, "Dbghelp.lib")


//LINUX
#else
//TIME
#include <sys/time.h>
//NETWORK
#ifdef DEBUG_LOGGER_INCLUDE_WEBSERVER
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif
//THREAD
#include<pthread.h>
//CALLSTACK
#include <execinfo.h>
#include <unistd.h>
#endif


//LOG FUNCTION DEFS FOR EASY USE
#define log_info(_tag, _reason) log(FRM::debug_logger::DEBUG_LOGGER_DEBUG_LEVELS::DL_INFO, _tag, _reason, __FUNCTION__, __LINE__)
#define log_warning(_tag, _reason) log(FRM::debug_logger::DEBUG_LOGGER_DEBUG_LEVELS::DL_WARNING, _tag, _reason, __FUNCTION__, __LINE__)
#define log_error(_tag, _reason) log(FRM::debug_logger::DEBUG_LOGGER_DEBUG_LEVELS::DL_ERROR, _tag, _reason, __FUNCTION__, __LINE__)
//AT DESTRUCT save a html version if DEBUG_LOGGER_AUTO_SAVE defined
#define DEBUG_LOGGER_AUTO_SAVE
#define DEBUG_LOGGER_AUTO_SAVE_DIR_UNIX "./log/"
#define DEBUG_LOGGER_AUTO_SAVE_DIR_WIN "\log"
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN__) || defined(MO_WIN)
#define DEBUG_LOGGER_DEFUALT_DIR DEBUG_LOGGER_AUTO_SAVE_DIR_WIN
#else
#define DEBUG_LOGGER_DEFUALT_DIR DEBUG_LOGGER_AUTO_SAVE_DIR_UNIX
#endif
//NEW LINE CHARS
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN__) || defined(MO_WIN)
#define DEBUG_LOGGER_NEW_LINE_CHARAKTER "\r\n"
#else
#define DEBUG_LOGGER_NEW_LINE_CHARAKTER "\n"
#endif
//XML DEFINES
#define DEBUG_LOGGER_XNL_CHART_SECTION_NAME "GENERATED_CHARTS"
#define DEBUG_LOGGER_XNL_CALLSTACK_SECTION_NAME "GENERATED_CALLSTACKS"
//CALLSTACK DEFINES
#define DEBUG_LOGGER_MAX_CALLSTACK_SIZE 100
//at release disbable all loggin functions
#define DEBUG_LOGGER_DISABLE_LOGGING_AT_RELEASE //IF DEFINED DISABLE ALL LOGGIN FUNTIONS ON RELEASE CONFIG
#define DEBUG_LOGGER_DISABLE_LOGGING //DISABLE LOGGING FUNCTIONS
#if ((defined(_RELEASE) || defined(MO_RELEASE)) && defined(DEBUG_LOGGER_DISABLE_LOGGING_AT_RELEASE)) || DEBUG_LOGGER_DISABLE_LOGGIN
#define log(a, b, c, d, e)
#define switch_section(a)
#define save_as_html(a,b)
#define save_as_xml(a,b)
#define start_webserver(a,b)
#define stop_webserver()
#define store_chart_value_timestamp(a,b)
#define store_chart_value_no_timestamp(a,b)
#define set_chart_info(a,b,c)
#define snapshot_callstack(a)
#define register_custom_webpage(a,b)
#define clear_custom_page_content(a)
#define save_chart_as_csv(a,b,c)
#endif
//USE JS LIB IN CODE
//if defined please put the canvasjs.min.js (Verison 1.8.1) into your debug logger output dir (defined above)
//#define DEBUG_LOGGER_USE_CANVASJS_IN_COMPILED
#if defined(DEBUG_LOGGER_USE_CANVASJS_IN_COMPILED)
#define DEBUG_LOGGER_ANVASJS_181 "<script src='canvasjs.min.js'></script>"
#endif
//if defined insert a refresh metatag in the hmtl code
#define DEBUG_LOGGER_HTML_AUTO_REFRESH
#if defined(DEBUG_LOGGER_HTML_AUTO_REFRESH)
#define DEBUG_LOGGER_AUTO_REFRESH_TIME "5" // set refresh time here
#else
#define DEBUG_LOGGER_AUTO_REFRESH_TIME ""
#endif
//CSV EXPORT
#define DEBUG_LOGGER_CSV_EXPORT_CHAR ";" //the csv seperator char

//yeah some namespacing ignore that
#ifndef FRM_GENERAL_NAMESPACE
#define FRM_GENERAL_NAMESPACE FRM
#endif
namespace FRM_GENERAL_NAMESPACE {
    class debug_logger{
    public:
        //DEBUG LEVELS
        enum DEBUG_LOGGER_DEBUG_LEVELS{
            DL_INFO, DL_WARNING, DL_ERROR
        };
        
        
        //FUNCTIONS
        ~ debug_logger();
        //get instance
        static debug_logger* get_instance();
        //main log function see #defs
        void log(const DEBUG_LOGGER_DEBUG_LEVELS _level,const std::string _tag,const std::string _reason, const std::string _function = "",const size_t _line = -1);
        //switch / create section
        void switch_section(const std::string _section_name);
        //export functions
        void save_as_html(const char* _path,const bool _open_file = false);
        void save_as_xml(const char* _path,const bool _open_file = false);
        void save_chart_as_csv(const char* _path,std::string _chartname,const bool _open_file = false);
        //open exported file in explorer
        void open_file_in_browser(const char* _path);
        //webserver functions
#ifdef DEBUG_LOGGER_INCLUDE_WEBSERVER
        void start_webserver(const unsigned int _port,const std::string _this_ip);
        void stop_webserver();
        void register_custom_webpage(const std::string _page,const std::string _content_to_append);
        void clear_custom_page_content(const std::string _page);
#endif
        //CHART FUNCTIONS
        void store_chart_value_timestamp(const float _value,const std::string _chartname);
        void store_chart_value_no_timestamp(const float _value, const std::string _chartname);
        void set_chart_info(const std::string _chartname,const std::string _label_x_axis, const std::string _label_y_axis);
        //CALLSTACK FUNCTIONS
        void snapshot_callstack(std::string _name);
        
        
        
    private:
        //functions
        debug_logger();
        static void* webserver_thread_func(void* arg); //runs the webserver
        void webserver_create_response(std::string* _result,const bool _from_webserver); //creates a http request
        void webserver_create_chart_js_var(std::string* _result,const bool _from_webserver);
        void generate_callstack_table(std::string* _result);
        void create_callstack(const std::string _name );
        void webserver_create_response_xml(std::string* _result,const bool _from_webserver);
        void get_current_timestamp(time_t* _time);
        //MUTEX FUNCS
#ifdef DEBUG_LOGGER_INCLUDE_WEBSERVER
        void mutex_lock();
        void mutex_unlock();
#endif
        //generator functions function change pointer
        void generate_html( std::string* _string,const bool _from_webserver);
        void generate_xml( std::string* _string);
        //a log entry
        struct DEBUG_LOGGER_DATA_SET{
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
        struct DEBUG_LOGGER_SECTION{
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
        struct CHART_POINT_DESC{
            time_t timestamp;
            float value;
            float alt_x_value;
        };
        //a chart
        struct CHART_DESC{
            std::vector<CHART_POINT_DESC> points;
            std::string chart_name;
            std::string x_axis_label;
            std::string y_axis_label;
            bool use_alt_x_value;
        };
        //chart map
        std::map<std::string, CHART_DESC> charts; //stores the charts
        //BACKTRACE VARS
        struct STACK_CALL_DESC{
            int return_ptr;
            std::string symbol;
            int id;
        };
        
        struct STACK_STACK_DESC{
            int id;
            time_t generated_at;
            std::vector<STACK_CALL_DESC> calls;
            std::string callstack_name;
        };
        std::vector<STACK_STACK_DESC> saved_call_stacks;
#ifdef DEBUG_LOGGER_INCLUDE_WEBSERVER
        //CUSTOM PAGE
        struct CUSTOM_PAGE{
            std::string _url;
            std::string _content;
        };
        std::map<std::string, CUSTOM_PAGE> pages;
        
        
        //WEBSERVER VARS
        unsigned int webserver_port;
        bool webserver_running;
        std::string local_ip;
        bool webserver_was_started;
#if defined(MO_WIN) || defined(_WIN) || defined(_WIN32) || defined(_WIN64) || defined(__WIN__)
        DWORD webserver_thread;
        HANDLE webserver_thread_handle;
        HANDLE lock;
        WSADATA wsaData;
        int iResult;
#else
        int listen_socket;
        struct sockaddr_in server_addr;
        pthread_t webserver_thread;
        pthread_mutex_t lock;
#endif
#endif
    };
}
#endif /* frm_debug_logger_hpp */
