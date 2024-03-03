
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

    class CustomResourceApp {

        public:


            void createSample(){

                KubernetesClient kube_client;

                json cr = R"({
                    "apiVersion": "stable.example.com/v1",
                    "kind": "CronTab",
                    "metadata": {
                        "name": "my-crontab",
                        "namespace": "default"
                    },
                    "spec": {
                        "cronSpec": "*/5 * * * *",
                        "image": "my-cron-image:latest",
                        "replicas": 3
                    }
                })"_json;

                json response = kube_client.createResources( cr );

                cout << response.dump(4) << endl;
                                
            }



            void deleteSample(){

                KubernetesClient kube_client;
                
                json cr = R"({
                    "apiVersion": "stable.example.com/v1",
                    "kind": "CronTab",
                    "metadata": {
                        "name": "my-crontab",
                        "namespace": "default"
                    }
                })"_json;

                json response = kube_client.deleteResources( cr );

                cout << response.dump(4) << endl;
                                
            }


            void listSample(){

                KubernetesClient kube_client;

                json response = kube_client.runQuery( "SELECT * FROM stable.example.com/v1:CronTab" );

                cout << response.dump(4) << endl;
                
            }


    };

}




