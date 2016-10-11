//
//  FRM_debug_logger.cpp
//  optimiser
//
//  Created by Marcel Ochsendorf on 24.03.16.
//  Copyright © 2016 Marcel Ochsendorf. All rights reserved.
//

#include "FRM_DEBUG_LOGGER.h"


FRM::debug_logger* FRM::debug_logger::instance = nullptr;

FRM::debug_logger::debug_logger() {
	//reset counters
	data_set_count = 0;
	section_count = 0;

	//create startup timestamp
#if defined(FRM_OS_WIN)
	time_t ltime;
	time(&ltime);
	struct tm* timeinfo = gmtime(&ltime); /* Convert to UTC */
	startup_time = mktime(timeinfo); /* Store as unix timestamp */
#else
	startup_time = time(NULL);
#endif



	//CREATE A SHORT DEBUG INFO
	switch_section("BUILD INFO");
	log_info("COMPILATION DATE", __DATE__);
	log_info("COMPILATION TIME", __TIME__);
	log_info("LOG FILE SERVICE STARTED", std::to_string(startup_time));

	#if defined(FRM_BUILD_RELEASE)
	 log_info("CONFIGURATION", "RELEASE");
	#endif

	 #if defined(FRM_BUILD_DEBUG)
	log_info("CONFIGURATION", "DEBUG");
#endif

#if defined(FRM_OS_WIN)
	log_warning("PLATTFORM", "WINDOWS NOT GOOD :(");
#endif

#if defined(FRM_OS_APPLE)
	log_info("PLATTFORM", "APPLE");
#endif

#if defined(FRM_OS_LINUX)
	log_info("PLATTFORM", "LINUX");
#endif
}


FRM::debug_logger::~debug_logger() {
	//stop webserver
	if (webserver_running) {
		stop_webserver();
	}
	//autosave log
#if defined(DEBUG_LOGGER_AUTO_SAVE)
#if defined(FRM_OS_WIN)
	save_as_xml(DEBUG_LOGGER_AUTO_SAVE_DIR_WIN);
#else
	save_as_html(DEBUG_LOGGER_AUTO_SAVE_DIR_UNIX);
#endif
#endif

}


FRM::debug_logger* FRM::debug_logger::get_instance() {
	if (!instance) {
		instance = new FRM::debug_logger();
	}
	return instance;
}




void FRM::debug_logger::store_chart_value(int _value, std::string _chartname) {

	//LOCK MUTEX
#if defined(FRM_WIN) || defined(_WIN) || defined(_WIN32) || defined(_WIN64) || defined(__WIN__)
	WaitForSingleObject(lock, 10);
#else
	pthread_mutex_lock(&lock);
#endif


	if (_chartname == "") {
		_chartname = "default_chart";
	}


	//create new point with value
	CHART_POINT_DESC point;
	point.value = _value;
	//store timestamp
#if defined(FRM_WIN) || defined(_WIN) || defined(_WIN32) || defined(_WIN64) || defined(__WIN__)
	time_t ltime;
	time(&ltime);
	struct tm* timeinfo = gmtime(&ltime); /* Convert to UTC */
	point.timestamp = mktime(timeinfo); /* Store as unix timestamp */
#else
	point.timestamp = time(NULL);
#endif

	//store point
	charts[_chartname].points.push_back(point);
	//store name
	if (charts[_chartname].chart_name == "") {
		charts[_chartname].chart_name = _chartname;
	}
	//store label x
	if (charts[_chartname].x_axis_label == "") {
		charts[_chartname].x_axis_label = "TIME";
	}
	//store label y
	if (charts[_chartname].y_axis_label == "") {
		charts[_chartname].y_axis_label = "VALUE";
	}


	//UNLOCK MUTEX
#if defined(FRM_WIN) || defined(_WIN) || defined(_WIN32) || defined(_WIN64) || defined(__WIN__)
	ReleaseMutex(lock);
#else
	pthread_mutex_unlock(&lock);
#endif

}


void FRM::debug_logger::set_chart_info(const std::string _chartname, const std::string _label_x_axis, const std::string _label_y_axis) {


	if (_chartname == "") {
		return;
	}

	//LOCK MUTEX
#if defined(FRM_WIN) || defined(_WIN) || defined(_WIN32) || defined(_WIN64) || defined(__WIN__)
	WaitForSingleObject(lock, 10);
#else
	pthread_mutex_lock(&lock);
#endif


	if (_label_x_axis != "") {
		charts[_chartname].x_axis_label = _label_x_axis;
	}

	if (_label_y_axis != "") {
		charts[_chartname].y_axis_label = _label_y_axis;
	}

	//UNLOCK MUTEX
#if defined(FRM_WIN) || defined(_WIN) || defined(_WIN32) || defined(_WIN64) || defined(__WIN__)
	ReleaseMutex(lock);
#else
	pthread_mutex_unlock(&lock);
#endif


}






void FRM::debug_logger::switch_section(std::string _section_name) {
	if (_section_name == "") {
		_section_name = "__no_section__";
	}
	//store section
	current_section = _section_name;
	//create new section in map
	sections[_section_name].section_desc = _section_name;
}

void FRM::debug_logger::log(const FRM::debug_logger::DEBUG_LOGGER_DEBUG_LEVELS _level, const std::string _tag, const std::string _reason, const std::string _function, const size_t _line) {
	//create dataset
	DEBUG_LOGGER_DATA_SET tmp_set;
	tmp_set.position = sections[current_section].entry.size(); //may reFRMve ? calc in the js
	tmp_set.level = _level;
	tmp_set.tag = _tag;
	tmp_set.reason = _reason;
	tmp_set.function = _function;
	tmp_set.level = _level;
	tmp_set.data_set_id = data_set_count++;
	tmp_set.line = _line;
	//store timestamp
#if defined(FRM_WIN) || defined(_WIN) || defined(_WIN32) || defined(_WIN64) || defined(__WIN__)
	time_t ltime;
	time(&ltime);
	struct tm* timeinfo = gmtime(&ltime); /* Convert to UTC */
	tmp_set.timestamp = mktime(timeinfo); /* Store as unix timestamp */
#else
	tmp_set.timestamp = time(NULL);
#endif

	//LOCK MUTEX
#if defined(FRM_WIN) || defined(_WIN) || defined(_WIN32) || defined(_WIN64) || defined(__WIN__)
	WaitForSingleObject(lock, 10);
#else
	pthread_mutex_lock(&lock);
#endif


	//store new dataset in map
	sections[current_section].entry.push_back(tmp_set);
	//update counters
	if (_level == DEBUG_LOGGER_DEBUG_LEVELS::DL_ERROR) {
		sections[current_section].level_count_error++;
	}
	else if (_level == DEBUG_LOGGER_DEBUG_LEVELS::DL_INFO) {
		sections[current_section].level_count_info++;
	}
	else if (_level == DEBUG_LOGGER_DEBUG_LEVELS::DL_WARNING) {
		sections[current_section].level_count_warning++;
	}

	//UNLOCK MUTEX
#if defined(FRM_WIN) || defined(_WIN) || defined(_WIN32) || defined(_WIN64) || defined(__WIN__)
	ReleaseMutex(lock);
#else
	pthread_mutex_unlock(&lock);
#endif
}


void FRM::debug_logger::open_file_in_browser(const char* _path) {
	if (_path == nullptr) { return; }
#if defined(FRM_OS_WIN)
	ShellExecute(NULL, (LPCWSTR)"open", (LPCWSTR)_path, NULL, NULL, SW_SHOWNORMAL);
#else
	std::string call = "open ";
	call.append(_path);
	system(call.c_str());
#endif
}


void FRM::debug_logger::generate_xml(std::string* _string) {
	if (_string == nullptr) {
		return;
	}
	//start with version
	*_string = "<xml version\"1.0\">\n";
	//though each section
	std::map<std::string, DEBUG_LOGGER_SECTION>::iterator iter;
	for (iter = sections.begin(); iter != sections.end(); ++iter) {
		//add section desc
		_string->append("<" + iter->second.section_desc + ">\n");
		for (size_t i = 0; i < iter->second.entry.size(); i++) {
			DEBUG_LOGGER_DATA_SET* set = &iter->second.entry.at(i);
			//get log level as string
			std::string stufe = "";
			if (set->level == DEBUG_LOGGER_DEBUG_LEVELS::DL_ERROR) {
				stufe = "ERROR";
			}
			else if (set->level == DEBUG_LOGGER_DEBUG_LEVELS::DL_WARNING) {
				stufe = "WARNING";
			}
			else if (set->level == DEBUG_LOGGER_DEBUG_LEVELS::DL_INFO) {
				stufe = "INFO";
			}
			//append enty
			_string->append("<log_entry id=\"" + std::to_string(set->data_set_id) + "\" function=\"" + set->function + "\" line=\"" + std::to_string(set->line) + "\" reason=\"" + set->reason + "\" log_level=\"" + stufe + "\" tag=\"" + set->tag + "\" timestamp=\"" + std::to_string(set->timestamp) + "\">\n");
		}
		//close section
		_string->append("</" + iter->second.section_desc + ">\n");
	}
	//wird xml abgeschlossen ???
	_string->append("</xml>");
}


void FRM::debug_logger::generate_html(std::string* _string) {
	if (_string == nullptr) {
		return;
	}
	//add header &
	*_string = "<html><head><title>LOGFILE</title><meta http-equiv='refresh' content='5' />";
	//add scriptfiles
	_string->append("<script type='text/javascript'>\nfunction showStuff(id){\nif(document.getElementById(id).style.display=='none'){\ndocument.getElementById(id).style.display='block';\n}else{\ndocument.getElementById(id).style.display='none';\n}\n}\n</script>\n");
	//add end head / body
	_string->append("</head><body><font face='courier new'>");

	size_t sec_count = 0;
	std::map<std::string, DEBUG_LOGGER_SECTION>::iterator iter;
	for (iter = sections.begin(); iter != sections.end(); ++iter) {
		//create section
		_string->append("<div width='100%' align='center' id='section_" + std::to_string(sec_count) + "' style='background-color: lightgrey;'><b><a href='#' onclick='showStuff(\"section_content_" + std::to_string(sec_count) + "\");'> " + iter->second.section_desc + "[<font color = 'red'> ERRORS " + std::to_string(iter->second.level_count_error) + " </font><font color = 'orange'> WARNINGS " + std::to_string(iter->second.level_count_warning) + " </font><font color = 'green'> INFOS " + std::to_string(iter->second.level_count_info) + " </font>]</a></b><br><br><div id='section_content_" + std::to_string(sec_count) + "' style='display: none;'>");
		//add entry
		for (size_t i = 0; i < iter->second.entry.size(); i++) {
			//GET ENTRY
			DEBUG_LOGGER_DATA_SET* set = &iter->second.entry.at(i);
			//ADD ENTRY

			//INFO
			if (set->level == DEBUG_LOGGER_DEBUG_LEVELS::DL_INFO) {
				_string->append("<table id='entry_0' width='80%' cellSpacing='1' cellPadding ='5' border='0' bgcolor='#DFDFE5'><tr><td bgcolor='#FFFFF' width='30%%'><font color = 'green'>INFO</font></td><td bgcolor='#FFFFF' width='35%%'>" + set->tag + "</td><b>Time:" + std::to_string(set->timestamp) + "[" + set->function + ":" + std::to_string(set->line) + "]</b><td bgcolor='#FFFFF' width='35%%'>" + set->reason + "</TD></tr></table><br>");
				//WARNING
			}
			else if (set->level == DEBUG_LOGGER_DEBUG_LEVELS::DL_WARNING) {
				_string->append("<table id='entry_0' width='80%' cellSpacing='1' cellPadding ='5' border='0' bgcolor='#DFDFE5'><tr><td bgcolor='#FFFFF' width='30%%'><font color = 'orange'>WARNING</font></td><td bgcolor='#FFFFF' width='35%%'>" + set->tag + "</td><b>Time:" + std::to_string(set->timestamp) + "[" + set->function + ":" + std::to_string(set->line) + "]</b><td bgcolor='#FFFFF' width='35%%'>" + set->reason + "</TD></tr></table><br>");
				//ERROR
			}
			else if (set->level == DEBUG_LOGGER_DEBUG_LEVELS::DL_ERROR) {
				_string->append("<table id='entry_0' width='80%' cellSpacing='1' cellPadding ='5' border='0' bgcolor='#DFDFE5'><tr><td bgcolor='#FFFFF' width='30%%'><font color='red'>EROR</font></td><td bgcolor='#FFFFF' width='35%%'>" + set->tag + "</td><b>Time:" + std::to_string(set->timestamp) + "[" + set->function + ":" + std::to_string(set->line) + "]</b><td bgcolor='#FFFFF' width='35%%'>" + set->reason + "</TD></tr></table><br>");
			}
		}
		//close divs
		_string->append("</div><br></div>");
		//
		sec_count++;
	}
	//close body and font
	_string->append("<br><br></font></body></html>");
}


void FRM::debug_logger::save_as_html(const char* _path, bool _open_file) {
	if (_path == nullptr) {
		_path = "./";
	}
	//create path
	std::string fpath = "";
	fpath.append(_path);
	fpath.append("generated_logfile_" + std::to_string(startup_time));
	fpath.append(".html");
	//generate html content
	std::string gen_html = "";
	generate_html(&gen_html);
	//save file
	FILE* save_file = nullptr;
	save_file = fopen(fpath.c_str(), "wt");
	if (save_file) {
		fwrite(gen_html.c_str(), sizeof(char), gen_html.size(), save_file);
		fclose(save_file);
		//open browser
		if (_open_file) {
			open_file_in_browser(_path);
		}
	}
}

void FRM::debug_logger::save_as_xml(const char* _path, bool _open_file) {
	if (_path == nullptr) {
		_path = "./";
	}
	//create path
	std::string fpath = "";
	fpath.append(_path);
	fpath.append("generated_logfile_" + std::to_string(startup_time));
	fpath.append(".xml");
	//generate html content
	std::string gen_xml = "";
	generate_xml(&gen_xml);
	//save file
	FILE* save_file = nullptr;
	save_file = fopen(fpath.c_str(), "wt");
	if (save_file) {
		fwrite(gen_xml.c_str(), sizeof(char), gen_xml.size(), save_file);
		fclose(save_file);
		//open browser
		if (_open_file) {
			open_file_in_browser(_path);
		}
	}
}



void FRM::debug_logger::start_webserver(unsigned int _port, std::string _this_ip) {
	webserver_port = _port;
	webserver_running = true;
	local_ip = _this_ip;
	if (webserver_port < 1000) {
		log_warning("DEBUG_LOGGER WEBSERVER", "Port is near at the standart ports");
	}

#if defined(FRM_OS_WIN)
	//NOW CREATE THREAD AND MUTEX
	lock = CreateMutex(NULL, FALSE, NULL);
	if (lock == NULL) {
		log_error("DEBUG_LOGGER WEBSERVER", "cant create mutex");
	}
	webserver_thread_handle = CreateThread(0, 0, webserver_thread_func, this, 0, &webserver_thread);
	if (webserver_thread_handle == NULL) {
		log_error("DEBUG_LOGGER WEBSERVER", "cant create webserver_thread");
	}

#else
	//NOW CREATE THREAD AND MUTEX
	if (pthread_mutex_init(&lock, NULL) != 0)
	{
		log_error("DEBUG_LOGGER WEBSERVER", "cant create mutex");
		close(listen_socket);
		return;
	}
	int err = pthread_create(&webserver_thread, NULL, webserver_thread_func, this);
	if (err != 0) {
		log_error("DEBUG_LOGGER WEBSERVER", "cant create webserver_thread");
	}
#endif
}


void FRM::debug_logger::webserver_create_response(std::string* _result) {
	if (_result == nullptr) {
		return;
	}
	std::string html_content = "";
	instance->generate_html(&html_content);
	*_result = "";
	_result->append("HTTP/1.1 200 OK\r\nHost: ");
	_result->append(local_ip);
	_result->append("\r\nServer: Apache/1.1.1\r\nContent-Type: text/html;charset=UTF-8\r\nConnection: close\r\nConnection: close\r\nContent-Lenght: ");
	_result->append(std::to_string(html_content.size()));
	_result->append("\r\n\r\n");
	_result->append(html_content);
}



void FRM::debug_logger::stop_webserver() {
	webserver_running = false;
	log_info("DEBUG_LOGGER WEBSERVER", "shutdown debug log server");

#if defined(FRM_OS_WIN)
	ReleaseMutex(lock);
	CloseHandle(webserver_thread_handle);
#else
	pthread_mutex_unlock(&lock);
	pthread_mutex_destroy(&lock);
	pthread_join(webserver_thread, NULL);
#endif
}



/* THREAD FUNCTIONS */
#if defined(FRM_OS_WIN) // WIN :(
DWORD WINAPI webserver_thread_func(LPVOID lpParameter)
//DWORD WINAPI webserver_thread_func(LPVOID lpParameter)
{
	//get current instance
	FRM::debug_logger* log_instance = reinterpret_cast<FRM::debug_logger*>(lpParameter);
	//vars
	WSADATA wsa;
	SOCKET s, new_socket;
	struct sockaddr_in server, client;
	int c;
	char *message;
	//yeah winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}
	//Create a socket
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		return 1;
	}
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(log_instance->webserver_port);
	//Bind
	if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		return 1;
	}
	//Listen to incoming connections
	listen(s, 5);
	//main server loop wait for client send data to them
	while (log_instance->webserver_running) {
		c = sizeof(struct sockaddr_in);
		new_socket = accept(s, (struct sockaddr *)&client, &c);
		if (new_socket == INVALID_SOCKET)
		{
			continue;
		}
		//Reply to client
		WaitForSingleObject(log_instance->lock, INFINITE);
		std::string newss = "";
		log_instance->   webserver_create_response(&newss);
		ReleaseMutex(log_instance->lock);
		send(new_socket, newss.c_str(), newss.size(), 0);
		closesocket(new_socket);
	}

	closesocket(s);
	WSACleanup();
	return 0;
}
#else //LINUX APPLE an all other cool plattforms
void* FRM::debug_logger::webserver_thread_func(void* arg) {
	FRM::debug_logger* log_instance = reinterpret_cast<debug_logger*>(arg);
	int sockfd, newsockfd; //sockets
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) { return 0; }
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(log_instance->webserver_port);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
		sizeof(serv_addr)) < 0) {
	}
	listen(sockfd, 5);
	clilen = sizeof(cli_addr);
	//webserver main loop
	while (log_instance->webserver_running) {
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) { continue; }
		//WAIT FOR LOCK THEN SEND DATA TO SERVER
		while (true) {
			if (pthread_mutex_trylock(&log_instance->lock) == 0) {
				std::string newss = "";
				log_instance->webserver_create_response(&newss);
				pthread_mutex_unlock(&log_instance->lock);
				write(newsockfd, newss.c_str(), newss.size());
				close(newsockfd);
				break;
			}
		}
	}
	close(sockfd);
	return 0;
}
#endif

















