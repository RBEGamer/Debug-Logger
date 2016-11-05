//
//  main.cpp
//  FRM_DEBUG_LOGGER_TESTER
//
//  Created by Marcel Ochsendorf on 04.11.16.
//  Copyright © 2016 Marcel Ochsendorf. All rights reserved.
//

#include <iostream>
#include "../../FRM_DEBUG_LOGGER/debug_logger.hpp"


int main(int argc, const char * argv[]) {
    std::cout << "DEBUG_LOGGER_STARTED" << std::endl;
     FRM::debug_logger* debug = FRM::debug_logger::get_instance(); //get instance
   debug->start_webserver(4242, "127.0.0.1"); // <- INSERT YOUR IP
    
    debug->switch_section("SECTION 1"); //create section
    
 
    for (int i = 0; i < 100; i++) {
      sleep(30); //to show timestamps
       debug->log_info(std::to_string(100*i),"213"); //create log entry
    
    if(i == 60){
            debug->switch_section("SEC 3");
        }
        if(i == 40){
            debug->switch_section("SEC 2");
        }
        
        debug->store_chart_value_timestamp(rand() % 100, "TEST_CHART"); //create chart
        std::cout << i << std::endl;
    }
   
    
    
    
    debug->snapshot_callstack("test snapshot"); //create callstack
    debug->save_chart_as_csv("./", "TEST_CHART");
    debug->save_as_xml("./", false); //save as xml
    debug->save_as_html("./", true); //save as html
    debug->stop_webserver(); //stop server
    
    std::cout << "DEBUG_LOGGER:STOPPED" << std::endl;
    return 0;

}
