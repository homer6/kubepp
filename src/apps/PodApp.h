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

                json response = kube_client.createResources( pod_sample );
                //json createGenericResource( const string& group, const string& version, const string& plural, const json& resource ) const;

                //json response = kube_client.createGenericResource( "", "v1", "pods", pod_sample );


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

                json response = kube_client.deleteResources( pod_sample );

                //json response = kube_client.deleteGenericResource( "", "v1", "pods", pod_sample );

                cout << response.dump(4) << endl;
                
            }


            void displayPods(){

                KubernetesClient kube_client;
                //json pods = kube_client.getPods();

                // // create a ResourceDescription object for pods in the default namespace
                // const ResourceDescription resource_description = ResourceDescription( R"({
                //     "apiVersion": "v1",
                //     "kind": "Pod",
                //     "metadata": {
                //         "namespace": "default"
                //     }
                // })"_json );


                // json response = kube_client.getGenericResources( resource_description );


                json response = kube_client.runQuery( "SELECT * FROM Pod" );

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

                const ResourceDescription resource_description = ResourceDescription(pod_sample);

                json response = kube_client.replaceGenericResource( resource_description, pod_sample );

                cout << response.dump(4) << endl;

            }


            void patchSample(){

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

                json patch = R"([
                    { "op": "replace", "path": "/spec/containers/0/image", "value": "nginx:1.16.1" }
                ])"_json;

                const ResourceDescription resource_description = ResourceDescription(pod_sample);


                json response = kube_client.patchGenericResource( resource_description, patch );

                cout << response.dump(4) << endl;


                // // Original Pod JSON
                // json original = R"({
                //     "apiVersion": "v1",
                //     "kind": "Pod",
                //     "metadata": {
                //         "namespace": "default",
                //         "name": "nginx"
                //     },
                //     "spec": {
                //         "containers": [
                //             {
                //                 "name": "nginx",
                //                 "image": "nginx:old_version"
                //             }
                //         ]
                //     }
                // })"_json;

                // // Modified Pod JSON
                // json modified = R"({
                //     "apiVersion": "v1",
                //     "kind": "Pod",
                //     "metadata": {
                //         "namespace": "default",
                //         "name": "nginx"
                //     },
                //     "spec": {
                //         "containers": [
                //             {
                //                 "name": "nginx",
                //                 "image": "nginx:new_version"
                //             }
                //         ]
                //     }
                // })"_json;

                // // Generate JSON Patch
                // json patch = json::diff(original, modified);
                // std::cout << patch.dump(4) << std::endl;


            }

    };

}

