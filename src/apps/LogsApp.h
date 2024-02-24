#pragma once


#include <string>
using std::string;

#include <iostream>
using std::cout;
using std::endl;


namespace kubepp::apps {

    class LogsApp {

        public:

            void run(){

                spdlog::info( "Hello, kube world! (logs)" );
                
            }

    };

}