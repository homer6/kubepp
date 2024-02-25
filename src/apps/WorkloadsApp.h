#pragma once


#include <string>
using std::string;

#include <iostream>
using std::cout;
using std::endl;

#include "KubernetesClient.h"


namespace kubepp::apps {

    class WorkloadsApp {

        public:

            void run(){

                KubernetesClient kube_client;
                kube_client.displayWorkloads();
                
            }

    };

}