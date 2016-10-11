//
//  mo_debug_logger_unit_test.cpp
//  optimiser
//
//  Created by Marcel Ochsendorf on 27.05.16.
//  Copyright © 2016 Marcel Ochsendorf. All rights reserved.
//

#include <stdio.h>
#include <iostream>
#include <string>
#include "FRM_DEFINES.h"
#include "FRM_DEBUG_LOGGER.h"







int mo_debug_logger_unit_test(int argc, const char * argv[]) {

    MO::debug_logger* debug = FRM::debug_logger::get_instance(); //get instance

    debug->start_webserver(4242, "127.0.0.1"); //<- INSERT YOUR IP

    debug->switch_section("SECTION 1"); //create section


    for (int i = 0; i < 100; i++) {
        sleep(30);
        debug->log_info(std::to_string((100-i)),std::to_string(i)); //create log entry
        if(i == 60){
            debug->switch_section("SEC 3");
        }
        if(i == 40){
            debug->switch_section("SEC 2");
        }

        debug->store_chart_value_timestamp(rand() % 100, "TEST_CHART"); //create chart
    }




    debug->snapshot_callstack("test snapshot"); //create callstack
    debug->save_chart_as_csv("./", "TEST_CHART");
    debug->save_as_xml("./", false); //save as xml
    debug->save_as_html("./", true); //save as html
    debug->stop_webserver(); //stop server
    return 0;
}
