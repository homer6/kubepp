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

    class CustomResourceDefinitionsApp {

        public:


            void createSample(){

                KubernetesClient kube_client;

                json crd = R"({
                    "apiVersion": "apiextensions.k8s.io/v1",
                    "kind": "CustomResourceDefinition",
                    "metadata": {
                        "name": "crontabs.stable.example.com"
                    },
                    "spec": {
                        "group": "stable.example.com",
                        "versions": [
                            {
                                "name": "v1",
                                "served": true,
                                "storage": true,
                                "schema": {
                                    "openAPIV3Schema": {
                                        "type": "object",
                                        "properties": {
                                            "spec": {
                                                "type": "object",
                                                "properties": {
                                                    "cronSpec": {"type": "string"},
                                                    "image": {"type": "string"},
                                                    "replicas": {"type": "integer"}
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        ],
                        "scope": "Namespaced",
                        "names": {
                            "plural": "crontabs",
                            "singular": "crontab",
                            "kind": "CronTab",
                            "shortNames": ["ct"]
                        }
                    }
                })"_json;

                json response = kube_client.createResources( crd );

                cout << response.dump(4) << endl;
                                
            }



            void deleteSample(){

                KubernetesClient kube_client;
                
                json crd = R"({
                    "apiVersion": "apiextensions.k8s.io/v1",
                    "kind": "CustomResourceDefinition",
                    "metadata": {
                        "name": "crontabs.stable.example.com"
                    }
                })"_json;

                json response = kube_client.deleteResources( crd );

                cout << response.dump(4) << endl;
                                
            }


            void displayCustomResourceDefinitions(){

                KubernetesClient kube_client;
                json crds = kube_client.runQuery( "SELECT * FROM CustomResourceDefinition" );
                cout << crds.dump(4) << endl;

            }


    };

}

