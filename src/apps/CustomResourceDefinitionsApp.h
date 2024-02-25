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

            void run(){

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
                                                    "field1": {
                                                        "type": "string"
                                                    },
                                                    "field2": {
                                                        "type": "boolean"
                                                    }
                                                },
                                                "required": ["field1"]
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
                            "shortNames": ["exr"]
                        }
                    }
                })"_json;

                json response = kube_client.createResources( crd );

                cout << response.dump(4) << endl;
                
            }

    };

}

