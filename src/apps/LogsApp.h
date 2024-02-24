#pragma once


#include <string>
using std::string;

#include <iostream>
using std::cout;
using std::endl;

#include "KubernetesClient.h"


namespace kubepp::apps {

    class LogsApp {

        public:

            void run(){

                spdlog::info( "Hello, kube world! (logs)" );
                KubernetesClient kube_client;
                kube_client.run();
                
            }

    };

}