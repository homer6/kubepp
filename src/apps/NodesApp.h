#pragma once


#include <string>
using std::string;

#include <iostream>
using std::cout;
using std::endl;

#include "KubernetesClient.h"

#include "json.hpp"
using json = nlohmann::json;


namespace kubepp::apps {

    class NodesApp {

        public:

            void run(){

                KubernetesClient kube_client;

                json response = kube_client.runQuery( "SELECT * FROM Node" );

                cout << response.dump(4) << endl;

                
            }

    };

}