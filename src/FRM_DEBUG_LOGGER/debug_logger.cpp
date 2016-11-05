//
//  frm_debug_logger.cpp
//  FRM_DEBUG_LOGGER
//
//  Created by Marcel Ochsendorf on 04.11.16.
//  Copyright © 2016 Marcel Ochsendorf. All rights reserved.
//

#include "debug_logger.hpp"


#ifdef DEBUG_LOGGER_INCLUDE_WEBSERVER
void FRM::debug_logger::mutex_lock(){
    //LOCK MUTEX
#if defined(MO_WIN) || defined(_WIN) || defined(_WIN32) || defined(_WIN64) || defined(__WIN__)
    WaitForSingleObject(lock, 10);
#else
    pthread_mutex_lock(&lock);
#endif
}
void FRM::debug_logger::mutex_unlock(){
    //UNLOCK MUTEX
#if defined(MO_WIN) || defined(_WIN) || defined(_WIN32) || defined(_WIN64) || defined(__WIN__)
    ReleaseMutex(lock);
#else
    pthread_mutex_unlock(&lock);
#endif
}
#endif

void FRM::debug_logger::get_current_timestamp(time_t* _time){
    if(_time == nullptr){
        return;
    }
    
#if defined(MO_WIN) || defined(_WIN) || defined(_WIN32) || defined(_WIN64) || defined(__WIN__)
    time(_time);
    struct tm* timeinfo = gmtime(_time); /* Convert to UTC */
    *_time = mktime(timeinfo); /* Store as unix timestamp */
#else
    *_time = time(NULL);
#endif
    
    
}


FRM::debug_logger* FRM::debug_logger::instance=nullptr;

FRM::debug_logger::debug_logger(){
    //reset counters
    data_set_count = 0;
    section_count = 0;
    
    get_current_timestamp(&startup_time);
    //CREATE A SHORT DEBUG INFO
    switch_section("BUILD_INFO"); //NO SPACES !
    log_info("COMPILATION DATE", __DATE__);
    log_info("COMPILATION TIME", __TIME__);
    log_info("LOG FILE SERVICE STARTED", std::to_string(startup_time));
    
#if defined(_RELEASE) || defined(MO_RELEASE)
    log_info("CONFIGURATION", "RELEASE");
#endif
    
#if defined(_DEBUG) || defined(MO_DEBUG)
    log_info("CONFIGURATION", "DEBUG");
#endif
    
#if defined(MO_WIN) || defined(_WIN) || defined(_WIN32) || defined(_WIN64) || defined(__WIN__)
    log_warning("PLATTFORM", "WINDOWS NOT GOOD :(");
#endif
    
#if defined(MO_APPLE) || defined(__APPLE__)
    log_info("PLATTFORM", "APPLE");
#endif
    
#if defined(MO_LINUX) || defined(__LINUX__) || defined(MO_UNIX_BASED)
    log_info("PLATTFORM", "LINUX");
#endif
}

FRM::debug_logger::~debug_logger(){
    //stop webserver
    if(webserver_running){
        stop_webserver();
    }
    //autosave log
#if defined(DEBUG_LOGGER_AUTO_SAVE)
    save_as_html(DEBUG_LOGGER_DEFUALT_DIR);
#endif
    
}

FRM::debug_logger* FRM::debug_logger::get_instance(){
    if(!instance){
        instance=new debug_logger();
    }
    return instance;
}





void FRM::debug_logger::store_chart_value_timestamp(float _value, std::string _chartname){
    //LOCK MUTEX
    mutex_lock();
    if(_chartname == ""){
        _chartname = "default_chart";
    }
    //create new point with value
    CHART_POINT_DESC point;
    point.value = _value;
    point.alt_x_value = 0.0f;
    //store timestamp
    get_current_timestamp(&point.timestamp);
    
    charts[_chartname].use_alt_x_value = false;
    //store point
    charts[_chartname].points.push_back(point);
    //store name
    if(charts[_chartname].chart_name == ""){
        charts[_chartname].chart_name = _chartname;
    }
    //store label x
    if(charts[_chartname].x_axis_label == ""){
        charts[_chartname].x_axis_label = "TIME";
    }
    //store label y
    if(charts[_chartname].y_axis_label == ""){
        charts[_chartname].y_axis_label = "VALUE";
    }
    //UNLOCK MUTEX
    mutex_unlock();
}


void FRM::debug_logger::store_chart_value_no_timestamp(float _value, std::string _chartname){
    //LOCK MUTEX
    mutex_lock();
    if(_chartname == ""){
        _chartname = "default_chart";
    }
    //create new point with value
    CHART_POINT_DESC point;
    point.value = _value;
    point.alt_x_value = _value;
    //store timestamp
    get_current_timestamp(&point.timestamp);
    
    charts[_chartname].use_alt_x_value = true; //!!!!!
    //store point
    charts[_chartname].points.push_back(point);
    //store name
    if(charts[_chartname].chart_name == ""){
        charts[_chartname].chart_name = _chartname;
    }
    //store label x
    if(charts[_chartname].x_axis_label == ""){
        charts[_chartname].x_axis_label = "X";
    }
    //store label y
    if(charts[_chartname].y_axis_label == ""){
        charts[_chartname].y_axis_label = "Y";
    }
    //UNLOCK MUTEX
    mutex_unlock();
}



void FRM::debug_logger::set_chart_info(const std::string _chartname,const std::string _label_x_axis, const std::string _label_y_axis){
    
    
    if(_chartname == ""){
        return;
    }
    std::string no_spaces = _chartname;
    no_spaces.erase(std::remove(no_spaces.begin(), no_spaces.end(), ' '), no_spaces.end());
    
    
    mutex_lock();
    
    if(_label_x_axis != ""){
        charts[no_spaces].x_axis_label = _label_x_axis;
    }
    
    if(_label_y_axis != ""){
        charts[no_spaces].y_axis_label = _label_y_axis;
    }
    
    
    mutex_unlock();
    
    
}

void FRM::debug_logger::webserver_create_chart_js_var(std::string* _result, bool _from_webserver){
    if(_result == nullptr){return;}
    //if the request comes from the webserver we cant use a local version of the jslib
    if(_from_webserver){
        *_result += "<script src='http://canvasjs.com/assets/script/canvasjs.min.js'></script>";
    }else{
#if defined(DEBUG_LOGGER_USE_CANVASJS_IN_COMPILED)
        *_result += DEBUG_LOGGER_ANVASJS_181;
#else
        *_result += "<script src='http://canvasjs.com/assets/script/canvasjs.min.js'></script>";
#endif
    }
    *_result += "<script type='text/javascript'>window.onload=function(){";
    std::map<std::string, CHART_DESC>::iterator it;
    for ( it = charts.begin(); it != charts.end(); it++ )
    {
        *_result +="var "+ it->second.chart_name +"_name = new CanvasJS.Chart('"+ it->second.chart_name +"_container', {title:{text:'"+ it->second.chart_name +"'},axisX:{interval: 10},data:[{type: 'line',xValueType:'dateTime',dataPoints:[";
        //add points to json array
        for (int i = 0; i < it->second.points.size(); i++) {
            //multiply by 1000 for the right scale
            if(it->second.use_alt_x_value){
                *_result += "{x:" + (std::to_string(it->second.points.at(i).alt_x_value))+ ",y:" + std::to_string(it->second.points.at(i).value) + "},";
            }else{
                *_result += "{x:" + (std::to_string(it->second.points.at(i).timestamp*1000))+ ",y:" + std::to_string(it->second.points.at(i).value) + "},";
            }
            
        }
        *_result += "]}]});"+ it->second.chart_name +"_name.render();";
    }
    *_result += "}</script>";
}

void FRM::debug_logger::switch_section(std::string _section_name){
    if(_section_name == "" || _section_name == DEBUG_LOGGER_XNL_CHART_SECTION_NAME){
        _section_name = "__no_section__";
    }
    std::string no_spaces= _section_name;
    no_spaces.erase(std::remove(no_spaces.begin(), no_spaces.end(), ' '), no_spaces.end());
    //store section
    current_section = no_spaces;
    //create new section in map
    sections[no_spaces].section_desc = no_spaces;
}

void FRM::debug_logger::log(const FRM::debug_logger::DEBUG_LOGGER_DEBUG_LEVELS _level,const std::string _tag,const std::string _reason, const std::string _function,const size_t _line){
    //create dataset
    DEBUG_LOGGER_DATA_SET tmp_set;
    tmp_set.position = sections[current_section].entry.size(); //may remove ? calc in the js
    tmp_set.level = _level;
    tmp_set.tag = _tag;
    tmp_set.reason = _reason;
    tmp_set.function = _function;
    tmp_set.level = _level;
    tmp_set.data_set_id = data_set_count++;
    tmp_set.line = _line;
    //store timestamp
    get_current_timestamp(&tmp_set.timestamp);
    
    //LOCK MUTEX
    mutex_lock();
    
    
    //store new dataset in map
    sections[current_section].entry.push_back(tmp_set);
    //update counters
    if(_level == DEBUG_LOGGER_DEBUG_LEVELS::DL_ERROR){
        sections[current_section].level_count_error++;
    }else if(_level == DEBUG_LOGGER_DEBUG_LEVELS::DL_INFO){
        sections[current_section].level_count_info++;
    }else if(_level == DEBUG_LOGGER_DEBUG_LEVELS::DL_WARNING){
        sections[current_section].level_count_warning++;
    }
    
    //UNLOCK MUTEX
    mutex_unlock();
}

void FRM::debug_logger::open_file_in_browser(const char* _path){
    if(_path == nullptr){return;}
#if defined(MO_WIN) || defined(_WIN) || defined(_WIN32) || defined(_WIN64) || defined(__WIN__)
    ShellExecute(NULL, "open", _path,
                 NULL, NULL, SW_SHOWNORMAL);
#else
    std::string call = "open ";
    call.append(_path);
    system(call.c_str());
#endif
}



void FRM::debug_logger::snapshot_callstack(std::string _name){
    if(_name == ""){
        _name = "user_generated_callstack";
    }
    _name += " (UGCS)"; //add hint for user gen cs;
    create_callstack(_name);
}



void FRM::debug_logger::generate_xml(std::string* _string){
    if(_string == nullptr){
        return;
    }
    //start with version
    *_string = "<xml version=\"1.0\">";
    _string->append(DEBUG_LOGGER_NEW_LINE_CHARAKTER);
    //though each section
    std::map<std::string, DEBUG_LOGGER_SECTION>::iterator iter;
    for (iter = sections.begin(); iter != sections.end(); ++iter) {
        //add section desc
        _string->append("<" + iter->second.section_desc + ">");
        _string->append(DEBUG_LOGGER_NEW_LINE_CHARAKTER);
        for (size_t i = 0; i < iter->second.entry.size(); i++) {
            DEBUG_LOGGER_DATA_SET* set =&iter->second.entry.at(i);
            //get log level as string
            std::string stufe = "";
            if(set->level == DEBUG_LOGGER_DEBUG_LEVELS::DL_ERROR){
                stufe = "ERROR";
            }else if(set->level == DEBUG_LOGGER_DEBUG_LEVELS::DL_WARNING){
                stufe = "WARNING";
            }else if(set->level == DEBUG_LOGGER_DEBUG_LEVELS::DL_INFO){
                stufe = "INFO";
            }
            //append enty
            _string->append("<log_entry id=\""+std::to_string(set->data_set_id)+"\" function=\""+set->function+"\" line=\""+std::to_string(set->line)+"\" reason=\""+set->reason+"\" log_level=\""+stufe+"\" tag=\""+set->tag+"\" timestamp=\""+ std::to_string(set->timestamp) +"\" />");
            _string->append(DEBUG_LOGGER_NEW_LINE_CHARAKTER);
        }
        //close section
        _string->append("</" + iter->second.section_desc + ">" + DEBUG_LOGGER_NEW_LINE_CHARAKTER);
    }
    
    
    
    //NOW INSERT CHARTS
    //INSERT CHART SECION
    _string->append("<");_string->append(DEBUG_LOGGER_XNL_CHART_SECTION_NAME); _string->append(">");
    _string->append(DEBUG_LOGGER_NEW_LINE_CHARAKTER);
    
    
    //INSERT EACH CHART IN A NEW SCECTION AND POINTS INSIDE THIS
    std::map<std::string, CHART_DESC>::iterator it;
    int count = 0;
    for ( it = charts.begin(); it != charts.end(); it++ )
    {
        _string->append("<chart id=\"" + std::to_string(count) + "\" name=\""+ it->second.chart_name  +"\" x_asis=\""+ it->second.x_axis_label  +"\" y_asis=\""+ it->second.chart_name  +"\">");
        _string->append(DEBUG_LOGGER_NEW_LINE_CHARAKTER);
        //add points
        for (int i = 0; i < it->second.points.size(); i++) {
            if(it->second.use_alt_x_value){
                *_string += "<chart_point chart_id=\"" + std::to_string(count) + "\" x_value=\""  + std::to_string(it->second.points.at(i).alt_x_value)+ "\" y_value=\""  + std::to_string(it->second.points.at(i).value) + "\" />";
            }else{
                *_string += "<chart_point chart_id=\"" + std::to_string(count) + "\" x_value=\""  + std::to_string(it->second.points.at(i).timestamp)+ "\" y_value=\""  + std::to_string(it->second.points.at(i).value) + "\" />";
            }
            _string->append(DEBUG_LOGGER_NEW_LINE_CHARAKTER);
        }
        _string->append("</chart>");
        _string->append(DEBUG_LOGGER_NEW_LINE_CHARAKTER);
        count++;
    }
    
    //close chart section
    _string->append("</");_string->append(DEBUG_LOGGER_XNL_CHART_SECTION_NAME); _string->append(">");
    _string->append(DEBUG_LOGGER_NEW_LINE_CHARAKTER);
    
    
    
    //NOW ADD CALLSTACKS
    _string->append("<");_string->append(DEBUG_LOGGER_XNL_CALLSTACK_SECTION_NAME); _string->append(">");
    _string->append(DEBUG_LOGGER_NEW_LINE_CHARAKTER);
    
    
    for (int i = 0; i < saved_call_stacks.size(); i++) {
        
        _string->append("<callstack id=\"" + std::to_string(i) + "\" name=\"" +saved_call_stacks.at(i).callstack_name + "\" generation_time=\"" + std::to_string(saved_call_stacks.at(i).generated_at) + "\" >");
        _string->append(DEBUG_LOGGER_NEW_LINE_CHARAKTER);
        
        for (int j = 0; j < saved_call_stacks.at(i).calls.size(); j++) {
            
            _string->append("<callstack_call id=\"" + std::to_string(saved_call_stacks.at(i).calls.at(j).id) + "\" return_ptr=\"" +std::to_string(saved_call_stacks.at(i).calls.at(j).return_ptr) + "\" symbol_name=\"" +saved_call_stacks.at(i).calls.at(j).symbol + "\" />");
            _string->append(DEBUG_LOGGER_NEW_LINE_CHARAKTER);
        }
        _string->append("</callstack>");
        _string->append(DEBUG_LOGGER_NEW_LINE_CHARAKTER);
    }
    
    //close callstack section
    _string->append("</");_string->append(DEBUG_LOGGER_XNL_CALLSTACK_SECTION_NAME); _string->append(">");
    _string->append(DEBUG_LOGGER_NEW_LINE_CHARAKTER);
    
    
    //wird xml abgeschlossen
    _string->append("</xml>");
    _string->append(DEBUG_LOGGER_NEW_LINE_CHARAKTER);
    _string->append(DEBUG_LOGGER_NEW_LINE_CHARAKTER);
}

#ifdef DEBUG_LOGGER_INCLUDE_WEBSERVER
void FRM::debug_logger::register_custom_webpage(std::string _page, std::string _content_to_append){
    if(_page == "" || _page == "/" || _page == "/index" || _page == "/" || _page == "/xml" || _page =="/index.html" || _page =="/data.xml" || _page == "/#"){
        return;
    }
    _page = "/" + _page;
    std::string no_spaces = _page;
    no_spaces.erase(std::remove(no_spaces.begin(), no_spaces.end(), ' '), no_spaces.end());
    //LOCK MUTEX
    mutex_lock();
    
    
    pages[no_spaces]._url = no_spaces;
    pages[no_spaces]._content.append(_content_to_append);
    
    
    //UNLOCK MUTEX
    mutex_unlock();
    
}
void FRM::debug_logger::clear_custom_page_content(std::string _page){
    if(_page == "" || _page == "/" || _page == "/index" || _page == "/" || _page == "/xml" || _page =="/index.html" || _page =="/data.xml" || _page == "/#" || _page == "/favicon.ico"){
        return;
    }
    //remove spaces
    _page = "/" + _page;
    std::string no_spaces = _page;
    no_spaces.erase(std::remove(no_spaces.begin(), no_spaces.end(), ' '), no_spaces.end());
    
    mutex_lock();
    pages[no_spaces]._content = "";
    mutex_unlock();
}
#endif

void FRM::debug_logger::save_chart_as_csv(const char* _path,std::string _chartname, bool _open_file){
    if(_chartname == ""){
        return;
    }
    //remove spaces
    std::string no_spaces_name = _chartname;
    no_spaces_name.erase(std::remove(no_spaces_name.begin(), no_spaces_name.end(), ' '), no_spaces_name.end());
    
    
    if(_path == nullptr){
        _path = DEBUG_LOGGER_DEFUALT_DIR;
    }
    //create path
    std::string fpath = "";
    fpath.append(_path);
    fpath.append("exported_chart_" + no_spaces_name + "_" + std::to_string(startup_time));
    fpath.append(".csv");
    //generate csv content
    std::string gen_html = "";
    //ADD HEADLINE
    gen_html.append(DEBUG_LOGGER_CSV_EXPORT_CHAR);
    gen_html.append(_chartname);
    gen_html.append(DEBUG_LOGGER_CSV_EXPORT_CHAR);
    gen_html.append(DEBUG_LOGGER_NEW_LINE_CHARAKTER);
    //ADD AXIS DESC
    gen_html.append(DEBUG_LOGGER_CSV_EXPORT_CHAR);
    if(charts[no_spaces_name].x_axis_label != ""){
        gen_html.append(charts[no_spaces_name].x_axis_label);
    }else{
        gen_html.append("X");
    }
    gen_html.append(DEBUG_LOGGER_CSV_EXPORT_CHAR);
    if(charts[no_spaces_name].y_axis_label != ""){
        gen_html.append(charts[no_spaces_name].y_axis_label);
    }else{
        gen_html.append("Y");
    }
    gen_html.append(DEBUG_LOGGER_CSV_EXPORT_CHAR);
    gen_html.append(DEBUG_LOGGER_NEW_LINE_CHARAKTER);
    //ADD POINTS
    mutex_lock();
    for (int i = 0; i < charts[no_spaces_name].points.size(); i++) {
        gen_html.append(DEBUG_LOGGER_CSV_EXPORT_CHAR);
        if(charts[no_spaces_name].use_alt_x_value){
            gen_html.append(std::to_string(charts[no_spaces_name].points.at(i).alt_x_value));
        }else{
            gen_html.append(std::to_string(charts[no_spaces_name].points.at(i).timestamp));
        }
        gen_html.append(DEBUG_LOGGER_CSV_EXPORT_CHAR);
        gen_html.append(std::to_string(charts[no_spaces_name].points.at(i).value));
        gen_html.append(DEBUG_LOGGER_CSV_EXPORT_CHAR);
        gen_html.append(DEBUG_LOGGER_NEW_LINE_CHARAKTER);
    }
    
    mutex_unlock();
    gen_html.append(DEBUG_LOGGER_CSV_EXPORT_CHAR);
    gen_html.append(DEBUG_LOGGER_CSV_EXPORT_CHAR);
    gen_html.append(DEBUG_LOGGER_CSV_EXPORT_CHAR);
    gen_html.append(DEBUG_LOGGER_NEW_LINE_CHARAKTER);
    //save file
    FILE* save_file = nullptr;
    save_file= fopen(fpath.c_str(), "wt");
    if(save_file){
        fwrite (gen_html.c_str() , sizeof(char), gen_html.size(), save_file);
        fclose (save_file);
        //open browser
        if(_open_file){
            open_file_in_browser(_path);
        }
    }
    
    
}
void FRM::debug_logger::generate_html(std::string* _string, bool _from_webserver){
    if(_string == nullptr){
        return;
    }
    //add header &
    *_string = "<html><head><title>LOGFILE</title>"; //<meta http-equiv='refresh' content='5' />";
    if(_from_webserver){
#if defined(DEBUG_LOGGER_HTML_AUTO_REFRESH)
        _string->append("<meta http-equiv='refresh' content='");
        _string->append(DEBUG_LOGGER_AUTO_REFRESH_TIME);
        _string->append("' />");
        _string->append(DEBUG_LOGGER_NEW_LINE_CHARAKTER);
#endif
    }
    //add scriptfiles
    _string->append("<script type='text/javascript'>\nfunction showStuff(id){\nif(document.getElementById(id).style.display=='none'){\ndocument.getElementById(id).style.display='block';\n}else{\ndocument.getElementById(id).style.display='none';\n}\n}\n</script>");
    _string->append(DEBUG_LOGGER_NEW_LINE_CHARAKTER);
    webserver_create_chart_js_var(_string, _from_webserver);
    
    //add end head / body
    _string->append("</head><body><font face='courier new'><center><h1>HELP-LOG</h1><br><h3>For the xml-version use /xml</h3><br><hr></center>");
    _string->append(DEBUG_LOGGER_NEW_LINE_CHARAKTER);
    size_t sec_count = 0;
    std::map<std::string, DEBUG_LOGGER_SECTION>::iterator iter;
    for (iter = sections.begin(); iter != sections.end(); ++iter) {
        //create section
        _string->append("<div width='100%' align='center' id='section_" + std::to_string(sec_count) + "' style='background-color: lightgrey;'><b><a href='#' onclick='showStuff(\"section_content_"+ std::to_string(sec_count)+"\");'> " + iter->second.section_desc + "[<font color = 'red'> ERRORS " + std::to_string(iter->second.level_count_error) + " </font><font color = 'orange'> WARNINGS " + std::to_string(iter->second.level_count_warning) + " </font><font color = 'green'> INFOS " + std::to_string(iter->second.level_count_info) + " </font>]</a></b><br><br><div id='section_content_"+ std::to_string(sec_count)+"' style='display: none;'>");
        //add entry
        for (size_t i = 0; i < iter->second.entry.size(); i++) {
            //GET ENTRY
            DEBUG_LOGGER_DATA_SET* set =&iter->second.entry.at(i);
            //ADD ENTRY
            
            //INFO
            if(set->level == DEBUG_LOGGER_DEBUG_LEVELS::DL_INFO){
                _string->append("<table id='entry_0' width='80%' cellSpacing='1' cellPadding ='5' border='0' bgcolor='#DFDFE5'><tr><td bgcolor='#FFFFF' width='30%%'><font color = 'green'>INFO</font></td><td bgcolor='#FFFFF' width='35%%'>"+set->tag+"</td><b>Time:"+std::to_string(set->timestamp)+"["+set->function+":"+ std::to_string(set->line)+"]</b><td bgcolor='#FFFFF' width='35%%'>"+set->reason+"</TD></tr></table><br>");
                //WARNING
            }else if(set->level == DEBUG_LOGGER_DEBUG_LEVELS::DL_WARNING){
                _string->append("<table id='entry_0' width='80%' cellSpacing='1' cellPadding ='5' border='0' bgcolor='#DFDFE5'><tr><td bgcolor='#FFFFF' width='30%%'><font color = 'orange'>WARNING</font></td><td bgcolor='#FFFFF' width='35%%'>"+set->tag+"</td><b>Time:"+std::to_string(set->timestamp)+"["+set->function+":"+ std::to_string(set->line)+"]</b><td bgcolor='#FFFFF' width='35%%'>"+set->reason+"</TD></tr></table><br>");
                //ERROR
            }else if(set->level == DEBUG_LOGGER_DEBUG_LEVELS::DL_ERROR){
                _string->append("<table id='entry_0' width='80%' cellSpacing='1' cellPadding ='5' border='0' bgcolor='#DFDFE5'><tr><td bgcolor='#FFFFF' width='30%%'><font color='red'>EROR</font></td><td bgcolor='#FFFFF' width='35%%'>"+set->tag+"</td><b>Time:"+std::to_string(set->timestamp)+"["+set->function+":"+ std::to_string(set->line)+"]</b><td bgcolor='#FFFFF' width='35%%'>"+set->reason+"</TD></tr></table><br>");
            }
            _string->append(DEBUG_LOGGER_NEW_LINE_CHARAKTER);
        }
        //close divs
        _string->append("</div><br></div>");
        _string->append(DEBUG_LOGGER_NEW_LINE_CHARAKTER);
        //
        sec_count++;
    }
    //close body and font
    
    //ADD CHARTS
    std::map<std::string, CHART_DESC>::iterator it;
    for ( it = charts.begin(); it != charts.end(); it++ )
    {
        _string->append("<br><div id='" + it->second.chart_name + "_container' style='height: 400px; width: 100%;'></div><br><br>");
        _string->append(DEBUG_LOGGER_NEW_LINE_CHARAKTER);
    }
    
    
    //ADD CALLSTACKS
    generate_callstack_table(_string);
    
    _string->append("<br><br></font></body></html>");
}
void FRM::debug_logger::generate_callstack_table(std::string* _result){
    if(_result == nullptr){
        return;
    }
    //append html headline
    _result->append("<center><h3>CALL STACKS</h3><hr><br /><br />");
    //insert callstack data
    for (int i = 0; i < saved_call_stacks.size(); i++) {
        _result->append("<h3> [ " + std::to_string(saved_call_stacks.at(i).id) + " ] " + saved_call_stacks.at(i).callstack_name + " [ "+  std::to_string( saved_call_stacks.at(i).generated_at)  +" ]</h3><br />");
        //create html tabel
        _result->append("<table><tr><th>ID</th><th>PTR</th><th>SYMBOL</th> </tr>");
        //create data cells
        for (int j = 0; j < saved_call_stacks.at(i).calls.size(); j++) {
            _result->append("<tr> <td>" +std::to_string(saved_call_stacks.at(i).calls.at(j).id) +"</td>"
                            "<td>" +std::to_string(saved_call_stacks.at(i).calls.at(j).return_ptr) +"</td>"
                            "<td>" +saved_call_stacks.at(i).calls.at(j).symbol +"</td>"
                            "</tr>");
        }
        //close table
        _result->append("</table>");
    }
    //close center tag
    _result->append("</center>");
}
void FRM::debug_logger::save_as_html(const char* _path, bool _open_file){
    if(_path == nullptr){
        _path = DEBUG_LOGGER_DEFUALT_DIR;
    }
    //create path
    std::string fpath = "";
    fpath.append(_path);
    fpath.append("generated_logfile_" + std::to_string(startup_time));
    fpath.append(".html");
    //generate html content
    std::string gen_html = "";
    generate_html(&gen_html, false);
    //save file
    FILE* save_file = nullptr;
    save_file= fopen(fpath.c_str(), "wt");
    if(save_file){
        fwrite (gen_html.c_str() , sizeof(char), gen_html.size(), save_file);
        fclose (save_file);
        //open browser
        if(_open_file){
            open_file_in_browser(_path);
        }
    }
}
void FRM::debug_logger::save_as_xml(const char* _path, bool _open_file){
    if(_path == nullptr){
        _path = DEBUG_LOGGER_DEFUALT_DIR;
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
    save_file= fopen(fpath.c_str(), "wt");
    if(save_file){
        fwrite (gen_xml.c_str() , sizeof(char), gen_xml.size(), save_file);
        fclose (save_file);
        //open browser
        if(_open_file){
            open_file_in_browser(_path);
        }
    }
}

#ifdef DEBUG_LOGGER_INCLUDE_WEBSERVER
void FRM::debug_logger::start_webserver(unsigned int _port, std::string _this_ip){
    webserver_port =_port;
    webserver_running = true;
    webserver_was_started = true;
    local_ip = _this_ip;
    if(webserver_port < 1000){
        log_warning("DEBUG_LOGGER WEBSERVER", "Port is near at the standart ports");
    }
    
#if defined(MO_WIN) || defined(_WIN) || defined(_WIN32) || defined(_WIN64) || defined(__WIN__)
    //NOW CREATE THREAD AND MUTEX
    lock = CreateMutex( NULL, FALSE, NULL);
    if(lock == NULL){
        log_error("DEBUG_LOGGER WEBSERVER", "cant create mutex");
    }
    webserver_thread_handle = CreateThread(0, 0, , this, 0, &webserver_thread);
    if(webserver_thread_handle == NULL){
        log_error("DEBUG_LOGGER WEBSERVER", "cant create webserver_thread");
    }
#else
    //NOW CREATE THREAD AND MUTEX FOR LINUX AND MAC
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        log_error("DEBUG_LOGGER WEBSERVER", "cant create mutex");
        close(listen_socket);
        return ;
    }
    int err =pthread_create (&webserver_thread, NULL, webserver_thread_func, this);
    if (err != 0){
        log_error("DEBUG_LOGGER WEBSERVER", "cant create webserver_thread");
    }
#endif
}
void FRM::debug_logger::webserver_create_response_xml(std::string* _result, bool _from_webserver){
    if(_result == nullptr){
        return;
    }
    std::string xml_content = "";
    generate_xml(&xml_content);
    *_result = "";
    _result->append("HTTP/1.1 200 OK\r\nHost: ");
    _result->append(local_ip);
    _result->append("\r\nServer: Apache/1.1.1\r\nContent-Type: text/xml;charset=UTF-8\r\nContent-Lenght: ");
    _result->append(std::to_string(xml_content.size()));
    _result->append("\r\n\r\n");
    _result->append(xml_content);
}
void FRM::debug_logger::webserver_create_response(std::string* _result, bool _from_webserver){
    if(_result == nullptr){
        return;
    }
    std::string html_content = "";
    generate_html(&html_content, _from_webserver);
    *_result = "";
    _result->append("HTTP/1.1 200 OK\r\nHost: ");
    _result->append(local_ip);
    _result->append("\r\nServer: Apache/1.1.1\r\nContent-Type: text/html;charset=UTF-8\r\nContent-Lenght: ");
    _result->append(std::to_string(html_content.size()));
    _result->append("\r\n\r\n");
    _result->append(html_content);
}
void FRM::debug_logger::stop_webserver(){
    if(!webserver_was_started){return;}
    webserver_was_started = false;
    webserver_running = false;
    log_info("DEBUG_LOGGER WEBSERVER", "shutdown debug log server");
    //RELEASE MUTEX AND THREAD
#if defined(MO_WIN) || defined(_WIN) || defined(_WIN32) || defined(_WIN64) || defined(__WIN__)
    ReleaseMutex(lock);
    CloseHandle(webserver_thread_handle);
#else
    pthread_mutex_unlock(&lock);
    pthread_mutex_destroy(&lock);
    pthread_join(webserver_thread, NULL);
#endif
}
#endif
//CREATE CALLSTACK FUNCTIONS FOR WINDOOF AND THE COOL PLATTFORMSs
#if defined(MO_WIN) || defined(_WIN) || defined(_WIN32) || defined(_WIN64) || defined(__WIN__)
void FRM::debug_logger::create_callstack(std::string _name){
    if(_name == ""){_name = "AUTO_GENERATED_CALLSTACK";}
    STACK_STACK_DESC new_stack;
    new_stack.callstack_name = _name;
    //save time id
    new_stack.id = (int)saved_call_stacks.size() + 1;
    //SAVE TIME
    get_current_timestamp(&new_stack.generated_at);
    
    unsigned int   i;
    void         * stack[ DEBUG_LOGGER_MAX_CALLSTACK_SIZE ];
    unsigned short frames;
    SYMBOL_INFO  * symbol;
    HANDLE         process;
    
    process = GetCurrentProcess();
    SymInitialize( process, NULL, TRUE );
    frames               = CaptureStackBackTrace( 0, DEBUG_LOGGER_MAX_CALLSTACK_SIZE, stack, NULL );
    symbol               = ( SYMBOL_INFO * )calloc( sizeof( SYMBOL_INFO ) + 256 * sizeof( char ), 1 );
    symbol->MaxNameLen   = 255;
    symbol->SizeOfStruct = sizeof( SYMBOL_INFO );
    
    for( i = 0; i < frames; i++ )
    {
        STACK_CALL_DESC call;
        call.id = i;
        SymFromAddr( process, ( DWORD64 )( stack[ i ] ), 0, symbol );
        //printf( "%i: %s - 0x%0X\n", frames - i - 1, symbol->Name, symbol->Address );
        call.return_ptr = symbol->Address;
        call.symbol = std::string(symbol->Name);
    }
    
    free( symbol );
    
    saved_call_stacks.push_back(new_stack);
    free(strings);
}
#else
void FRM::debug_logger::create_callstack(std::string _name ){
    if(_name == ""){_name = "AUTO_GENERATED_CALLSTACK";}
    STACK_STACK_DESC new_stack;
    //save time id
    new_stack.id = (int)saved_call_stacks.size() + 1;
    //SAVE TIME
    get_current_timestamp(&new_stack.generated_at);
    int j, nptrs;
    void *buffer[DEBUG_LOGGER_MAX_CALLSTACK_SIZE];
    char **strings;
    
    nptrs = backtrace(buffer, DEBUG_LOGGER_MAX_CALLSTACK_SIZE);
    strings = backtrace_symbols(buffer, nptrs);
    if (strings == NULL) {
        log_error("CALLSTACK_GENERATOR", "backtrace_symbols_failed");
        return;
    }
    
    for (j = 0; j < nptrs; j++){
        //printf("%s\n", strings[j]);
        STACK_CALL_DESC call;
        call.return_ptr = 0;
        call.id = j;
        call.symbol = std::string(strings[j]);
        new_stack.calls.push_back(call);
        new_stack.callstack_name = _name;
    }
    saved_call_stacks.push_back(new_stack);
    free(strings);
}
#endif

/* THREAD FUNCTIONS */
#ifdef DEBUG_LOGGER_INCLUDE_WEBSERVER
#if defined(MO_WIN) || defined(_WIN) || defined(_WIN32) || defined(_WIN64) || defined(__WIN__) // WIN :(
DWORD WINAPI webserver_thread_func(LPVOID lpParameter)
{
    //get current instance
    debug_logger* log_instance = *((debug_logger*)lpParameter);
    //vars
    WSADATA wsa;
    SOCKET s , new_socket;
    struct sockaddr_in server , client;
    int c;
    char *message;
    //yeah winsock
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
    {
        printf("Failed. Error Code : %d",WSAGetLastError());
        return 1;
    }
    //Create a socket
    if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
    {
        return 1;
    }
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( log_instance->webserver_port );
    //Bind
    if( bind(s ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR)
    {
        return 1;
    }
    //Listen to incoming connections
    listen(s , 5);
    //main server loop wait for client send data to them
    while (log_instance->webserver_running) {
        c = sizeof(struct sockaddr_in);
        new_socket = accept(s , (struct sockaddr *)&client, &c);
        if (new_socket == INVALID_SOCKET)
        {
            continue;
        }
        int n;
        char buffer[256];
        bzero(buffer, 256);
        volatile bool found = false;
        int end = -1;
        std::string com_buffer = "";
        std::string requested_data = "";
        while (!found)
        {
            //READ DATA
            n = (int)read(new_socket, buffer, 255);
            if (n < 0) {
                close(new_socket);
                break;
            }
            
            //PARSE REQUEST THE BROWSER SEND GET REQUESTES NORMALLY
            if (n > 0) {
                com_buffer += buffer;
                end = (int)com_buffer.find("\r\n\r\n");
                if (end != -1) {
                    const char* start = com_buffer.c_str();
                    start = strstr(start, "GET ");
                    if (start != 0)
                    {
                        start += 4;
                        const char* end = strstr(start, " HTTP/");
                        if (end != 0) {
                            requested_data.append(start, end);
                            found = true;
                        }
                    }
                }
            }
        }
        //SEND DATA TO CLIENT
        if(requested_data == "/" || requested_data == "/index" || requested_data == "/index.hmtl" || requested_data == "/#"){
            //WAIT FOR LOCK THEN SEND DATA TO SERVER
            log_instance->mutex_lock();
            std::string newss = "";
            log_instance->webserver_create_response(&newss, true);
            log_instance->mutex_unlock();
            write(new_socket,newss.c_str(),newss.size());
        }else if(requested_data == "/data" || requested_data == "/data.xml" || requested_data == "/xml"){
            log_instance->mutex_lock();
            std::string newss = "";
            log_instance->webserver_create_response_xml(&newss, true);
            log_instance->mutex_unlock();
            write(new_socket,newss.c_str(),newss.size());
        }else{
            std::map<std::string, CUSTOM_PAGE>::iterator it;
            std::string html_content = "";
            bool page_found = false;
            log_instance->mutex_lock();
            
            for ( it = log_instance->pages.begin(); it != log_instance->pages.end(); it++ )
            {
                if(requested_data == it->second._url){
                    html_content = "";
                    html_content.append("HTTP/1.1 200 OK\r\nHost: ");
                    html_content.append(log_instance->local_ip);
                    html_content.append("\r\nServer: Apache/1.1.1\r\nContent-Type: text/html;charset=UTF-8\r\nContent-Lenght: ");
                    html_content.append(std::to_string(it->second._content.size()));
                    html_content.append("\r\n\r\n");
                    html_content.append(it->second._content);
                    write(newsockfd,html_content.c_str(),html_content.size());
                    page_founf = true;
                    break;
                }
            }
            log_instance->mutex_unlock();
            
            if(!found_page){
                html_content = "";
                html_content.append("HTTP/1.1 404 OK\r\nHost: ");
                html_content.append(log_instance->local_ip);
                html_content.append("\r\nServer: Apache/1.1.1\r\nContent-Type: text/html;charset=UTF-8\r\nContent-Lenght: ");
                html_content.append("14");
                html_content.append("\r\n\r\n");
                html_content.append("PAGE_NOT_FOUND");
                write(newsockfd,html_content.c_str(),html_content.size());
            }
        }
        close(new_socket);
    }
    
    closesocket(s);
    WSACleanup();
    return 0;
}
#else //LINUX APPLE an all other cool plattforms
void* FRM::debug_logger::webserver_thread_func(void* arg){
    debug_logger* log_instance = reinterpret_cast<debug_logger*>(arg);
    int sockfd, newsockfd; //sockets
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){return 0;}
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(log_instance->webserver_port);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0){}
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    //webserver main loop
    while (log_instance->webserver_running) {
        newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);
        if (newsockfd < 0){
            std::cout  << " sock err "<< newsockfd << std::endl;
            continue;}
        std::cout << "conn" << std::endl;
        //GET CLIENT REQUEST AND SEND DATA BACK
        int n;
        char buffer[256];
        bzero(buffer, 256);
        volatile bool found = false;
        int end = -1;
        std::string com_buffer = "";
        std::string requested_data = "";
        while (!found)
        {
            //READ DATA
            n = (int)read(newsockfd, buffer, 255);
            if (n < 0) {
                close(newsockfd);
                break;
            }
            
            //PARSE REQUEST THE BROWSER SEND GET REQUESTES NORMALLY
            if (n > 0) {
                com_buffer += buffer;
                end = (int)com_buffer.find("\r\n\r\n");
                if (end != -1) {
                    const char* start = com_buffer.c_str();
                    start = strstr(start, "GET ");
                    if (start != 0)
                    {
                        start += 4;
                        const char* end = strstr(start, " HTTP/");
                        if (end != 0) {
                            requested_data.append(start, end);
                            found = true;
                        }
                    }
                }
            }
        }
        std::cout << "req page:" << requested_data << std::endl;
        //SEND DATA TO CLIENT
        if(requested_data == "/" || requested_data == "/index" || requested_data == "/index.html" || requested_data == "/#"){
            //WAIT FOR LOCK THEN SEND DATA TO SERVER
            log_instance->mutex_lock();
            std::string newss = "";
            log_instance->webserver_create_response(&newss, true);
            log_instance->mutex_unlock();
            write(newsockfd,newss.c_str(),newss.size());
            std::cout << "send page" << std::endl;
        }else if(requested_data == "/data" || requested_data == "/data.xml" || requested_data == "/xml"){
            log_instance->mutex_unlock();
            std::string newss = "";
            log_instance->webserver_create_response_xml(&newss, true);
            log_instance->mutex_unlock();
            write(newsockfd,newss.c_str(),newss.size());
            //CUSTOM PAGE
        }else if(requested_data == "/favicon.ico"){
            
        }else{
            
            std::map<std::string, CUSTOM_PAGE>::iterator it; //iterator for map
            std::string html_content = "";
            bool found_page = false;
            log_instance->mutex_lock();
            for ( it = log_instance->pages.begin(); it != log_instance->pages.end(); it++ )
            {
                if(requested_data == it->second._url){
                    html_content = "";
                    html_content.append("HTTP/1.1 200 OK\r\nHost: ");
                    html_content.append(log_instance->local_ip);
                    html_content.append("\r\nServer: Apache/1.1.1\r\nContent-Type: text/html;charset=UTF-8\r\nContent-Lenght: ");
                    html_content.append(std::to_string(it->second._content.size()));
                    html_content.append("\r\n\r\n");
                    html_content.append(it->second._content);
                    write(newsockfd,html_content.c_str(),html_content.size());
                    found_page = true;
                    break;
                }
            }
            log_instance->mutex_unlock();
            //SHOW 404
            if(!found_page){
                html_content = "";
                html_content.append("HTTP/1.1 404 OK\r\nHost: ");
                html_content.append(log_instance->local_ip);
                html_content.append("\r\nServer: Apache/1.1.1\r\nContent-Type: text/html;charset=UTF-8\r\nContent-Lenght: ");
                html_content.append("14");
                html_content.append("\r\n\r\n");
                html_content.append("PAGE_NOT_FOUND");
                write(newsockfd,html_content.c_str(),html_content.size());
            }
        }
        
        close(newsockfd);
        std::cout << "sock closed" << std::endl;
    }
    close(sockfd);
    return 0;
}
#endif
#endif
















