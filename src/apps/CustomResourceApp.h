
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
                    "apiVersion": "example.com/v1",
                    "kind": "ExampleResource",
                    "metadata": {
                        "name": "my-example-resource",
                        "namespace": "default"
                    },
                    "spec": {
                        "knownField": "value1",
                        "anotherKnownField": "value2"
                    }
                })"_json;

                json response = kube_client.createResources( cr );

                cout << response.dump(4) << endl;
                                
            }



            void deleteSample(){

                KubernetesClient kube_client;
                
                json cr = R"({
                    "apiVersion": "example.com/v1",
                    "kind": "ExampleResource",
                    "metadata": {
                        "name": "my-example-resource",
                        "namespace": "default"
                    }
                })"_json;

                json response = kube_client.deleteResources( cr );

                cout << response.dump(4) << endl;
                                
            }


    };

}




