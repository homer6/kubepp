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

                KubernetesClient kube_client;
                auto logs = kube_client.getPodLogs( "kube-system", "svclb-traefik-06f20d2a-684jx", "lb-tcp-80" );
                cout << logs.dump(4) << endl;
                
            }

    };

}