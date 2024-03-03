#pragma once


#include <string>
using std::string;

#include <iostream>
using std::cout;
using std::endl;

#include "KubernetesClient.h"


namespace kubepp::apps {

    class ExportApp {

        public:

            void exportAllResources(){

                KubernetesClient kube_client;
                json workloads = kube_client.runQuery( "SELECT * FROM *" );
                cout << workloads.dump(4) << endl;

            }

            void exportApiResources(){

                KubernetesClient kube_client;
                json workloads = kube_client.getApiResources();
                cout << workloads.dump(4) << endl;

            }

    };

}