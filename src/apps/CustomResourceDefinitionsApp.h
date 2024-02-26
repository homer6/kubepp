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
                        "name": "exampleresources.example.com"
                    },
                    "spec": {
                        "group": "example.com",
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
                                                    "failurePolicy": {
                                                        "type": "string",
                                                        "nullable": true
                                                    },
                                                    "valuesContent": {
                                                        "type": "string",
                                                        "nullable": true
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        ],
                        "scope": "Namespaced",
                        "names": {
                            "plural": "exampleresources",
                            "singular": "exampleresource",
                            "kind": "ExampleResource",
                            "shortNames": [
                                "exr"
                            ]
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
                        "name": "exampleresources.example.com"
                    }
                })"_json;

                json response = kube_client.deleteResources( crd );

                cout << response.dump(4) << endl;
                                
            }


            void displayCustomResourceDefinitions(){

                KubernetesClient kube_client;
                json crds = kube_client.getCustomResourceDefinitions();
                cout << crds.dump(4) << endl;

            }


    };

}

