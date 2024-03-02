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

    class PodApp {

        public:

            void createSample(){

                KubernetesClient kube_client;
                
                json pod_sample = R"({
                    "apiVersion": "v1",
                    "kind": "Pod",
                    "metadata": {
                        "namespace": "default",
                        "name": "nginx"
                    },
                    "spec": {
                        "containers": [
                            {
                                "name": "nginx",
                                "image": "nginx:1.14.2",
                                "ports": [
                                    {
                                        "containerPort": 80
                                    }
                                ]
                            }
                        ]
                    }
                })"_json;

                //json response = kube_client.createResources( pod_sample );
                //json createGenericResource( const string& group, const string& version, const string& plural, const json& resource ) const;

                json response = kube_client.createGenericResource( "", "v1", "pods", pod_sample );


                cout << response.dump(4) << endl;
                
            }


            void deleteSample(){

                KubernetesClient kube_client;
                
                json pod_sample = R"({
                    "apiVersion": "v1",
                    "kind": "Pod",
                    "metadata": {
                        "namespace": "default",
                        "name": "nginx"
                    }
                })"_json;

                //json response = kube_client.deleteResources( pod_sample );

                json response = kube_client.deleteGenericResource( "", "v1", "pods", pod_sample );

                cout << response.dump(4) << endl;
                
            }


            void displayPods(){

                KubernetesClient kube_client;
                //json pods = kube_client.getPods();

                json response = kube_client.getGenericResources( "", "v1", "pods", "default" );

                cout << response.dump(4) << endl;

            }


            void replaceSample(){

                KubernetesClient kube_client;
                
                json pod_sample = R"({
                    "apiVersion": "v1",
                    "kind": "Pod",
                    "metadata": {
                        "namespace": "default",
                        "name": "nginx"
                    },
                    "spec": {
                        "containers": [
                            {
                                "name": "nginx",
                                "image": "nginx:1.15.8"
                            }
                        ]
                    }
                })"_json;


                json response = kube_client.replaceGenericResource( "", "v1", "pods", pod_sample );

                cout << response.dump(4) << endl;

            }

    };

}

