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
                json all_resources = kube_client.runQuery( "SELECT * FROM *" );
                cout << all_resources.dump(4) << endl;

            }

            void exportApiResources(){

                KubernetesClient kube_client;
                json all_kinds = kube_client.getApiResources();
                cout << all_kinds.dump(4) << endl;

            }

    };

}