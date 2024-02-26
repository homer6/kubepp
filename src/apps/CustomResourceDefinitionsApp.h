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


/*
        openAPIV3Schema:
          type: object
          properties:
            spec:
              type: object
              properties:
                checksum:
                  type: string
                  nullable: true
                source:
                  type: string
                  nullable: true
*/


                json crd = R"({
                    "apiVersion": "apiextensions.k8s.io/v1",
                    "kind": "CustomResourceDefinition",
                    "metadata": {
                        "name": "exampleresources.example.com"
                    },
                    "spec": {
                        "group": "example.com",
                        "names": {
                            "kind": "ExampleResource",
                            "listKind": "ExampleResourceList",
                            "plural": "exampleresources",
                            "singular": "exampleresource",
                            "shortNames": ["exr"]
                        },
                        "scope": "Namespaced",
                        "versions": [{
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
                                                "knownField": {
                                                    "type": "string",
                                                    "nullable": true
                                                },
                                                "anotherKnownField": {
                                                    "type": "string",
                                                    "nullable": true
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }]
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

