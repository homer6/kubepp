#pragma once


#include <string>
using std::string;

#include <iostream>
using std::cout;
using std::endl;

#include "KubernetesClient.h"


namespace kubepp::apps {

    class EventsApp {

        public:

            void run(){

                spdlog::info( "Hello, kube world! (events)" );

                KubernetesClient client;
                client.run();
                
            }

    };

}