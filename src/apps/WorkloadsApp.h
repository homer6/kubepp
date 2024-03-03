#pragma once


#include <string>
using std::string;

#include <iostream>
using std::cout;
using std::endl;

#include "KubernetesClient.h"


namespace kubepp::apps {

    class WorkloadsApp {

        public:

            void run(){

                // KubernetesClient kube_client;
                // json workloads = kube_client.runQuery( "SELECT * FROM Pod, Deployment, stable.example.com/v1:CronTab" );
                // cout << workloads.dump(4) << endl;


                KubernetesClient kube_client;
                json workloads = kube_client.runQuery( "SELECT * FROM *" );
                cout << workloads.dump(4) << endl;

                // KubernetesClient kube_client;
                // json workloads = kube_client.getApiResources();
                // cout << workloads.dump(4) << endl;

                /*
                
                    {
                        "apiVersion": "rbac.authorization.k8s.io/v1",
                        "kind": "Role",
                        "name": "roles",
                        "namespaced": true,
                        "singularName": "role",
                        "storageVersionHash": "7FuwZcIIItM=",
                        "verbs": [
                            "create",
                            "delete",
                            "deletecollection",
                            "get",
                            "list",
                            "patch",
                            "update",
                            "watch"
                        ]
                    },
                
                */
                
            }

    };

}