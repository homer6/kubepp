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
                json workloads = kube_client.runQuery( "SELECT * FROM Pod, Deployment, stable.example.com/v1:CronTab" );
                cout << workloads.dump(4) << endl;
                
            }

    };

}