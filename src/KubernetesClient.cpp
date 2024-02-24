#include "KubernetesClient.h"

extern "C" {
    #include <kube_config.h>
    #include <apiClient.h>
    #include <CoreV1API.h>
    #include <AppsV1API.h>
}

#include <stdexcept>
#include <fmt/core.h>

#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"   // support for loading levels from the environment variable
#include "spdlog/fmt/ostr.h"  // support for user defined types


#include <iostream>
using std::cout;
using std::endl;
using std::cerr;




namespace kubepp{

    KubernetesClient::KubernetesClient( const string& base_path )
        :base_path(base_path)
    {

        int rc = load_kube_config(&detected_base_path, &sslConfig, &apiKeys, NULL);
        if (rc != 0) {
            throw std::runtime_error("Cannot load kubernetes configuration.");
        }

        fmt::print("The detected base path: {}\n", detected_base_path);
        fmt::print("Length of detected base path: {}\n", strlen(detected_base_path));
        cout << endl;
        fmt::print("The base path: {}\n", base_path);        
        fmt::print("Length of base path: {}\n", strlen(base_path.c_str()));

        //apiClient = std::shared_ptr<apiClient_t>( apiClient_create_with_base_path(const_cast<char*>(base_path.c_str()), sslConfig, apiKeys), apiClient_free);
        this->api_client = std::shared_ptr<apiClient_t>( apiClient_create_with_base_path(detected_base_path, sslConfig, apiKeys), apiClient_free);
        
        //this->api_client = std::shared_ptr<apiClient_t>( apiClient_create(), apiClient_free);
        if (!this->api_client) {
            throw std::runtime_error("Cannot create a kubernetes client.");
        }

    }

    KubernetesClient::~KubernetesClient(){

        //apiClient_free(apiClient); called from the shared_ptr custom deleter
        //apiClient = nullptr;

        free_client_config(detected_base_path, sslConfig, apiKeys);
        //basePath = nullptr;
        sslConfig = nullptr;
        apiKeys = nullptr;
        apiClient_unsetupGlobalEnv();

    }

    void KubernetesClient::run(){

        spdlog::info( "Hello, kube world! (client)" );
        this->listPods();
        this->listDeployments();

    }


    vector<string> KubernetesClient::getNamespaces() const{

        v1_namespace_list_t *namespace_list = NULL;
        namespace_list = CoreV1API_listNamespace(const_cast<apiClient_t*>(api_client.get()), 
                                                NULL,    /* pretty */
                                                NULL,    /* allowWatchBookmarks */
                                                NULL,    /* continue */
                                                NULL,    /* fieldSelector */
                                                NULL,    /* labelSelector */
                                                NULL,    /* limit */
                                                NULL,    /* resourceVersion */
                                                NULL,    /* resourceVersionMatch */
                                                NULL,    /* sendInitialEvents */
                                                NULL,    /* timeoutSeconds */
                                                NULL     /* watch */
        );

        vector<string> namespaces;

        if( namespace_list ){

            listEntry_t *listEntry = NULL;
            v1_namespace_t *_namespace = NULL;
            list_ForEach(listEntry, namespace_list->items) {
                _namespace = (v1_namespace_t *)listEntry->data;
                namespaces.push_back(_namespace->metadata->name);
            }
            v1_namespace_list_free(namespace_list);
            namespace_list = NULL;

        }else{

            spdlog::error("Cannot get any namespace.");

        }

        return namespaces;

    }




    void KubernetesClient::listPods( const vector<string>& k8s_namespaces ) const{

        auto namespaces = this->resolveNamespaces(k8s_namespaces);

        for( const string& k8s_namespace : namespaces ){

            v1_pod_list_t *pod_list = NULL;
            pod_list = CoreV1API_listNamespacedPod(const_cast<apiClient_t*>(api_client.get()), 
                                                const_cast<char*>(k8s_namespace.c_str()),   /*namespace */
                                                NULL,    /* pretty */
                                                NULL,    /* allowWatchBookmarks */
                                                NULL,    /* continue */
                                                NULL,    /* fieldSelector */
                                                NULL,    /* labelSelector */
                                                NULL,    /* limit */
                                                NULL,    /* resourceVersion */
                                                NULL,    /* resourceVersionMatch */
                                                NULL,    /* sendInitialEvents */
                                                NULL,    /* timeoutSeconds */
                                                NULL     /* watch */
                );

            //fmt::print("The return code of HTTP request={}\n", apiClient->response_code);

            if( pod_list ){

                fmt::print("Get pod list for namespace '{}':\n", k8s_namespace);

                listEntry_t *listEntry = NULL;
                v1_pod_t *pod = NULL;
                list_ForEach(listEntry, pod_list->items) {
                    pod = (v1_pod_t *)listEntry->data;
                    fmt::print("\tThe pod name: {}\n", pod->metadata->name);
                }
                v1_pod_list_free(pod_list);
                pod_list = NULL;

            }else{

                fmt::print("Cannot get any pod.\n");

            }

        }


    }





    void KubernetesClient::listDeployments( const vector<string>& k8s_namespaces ) const{

        auto namespaces = this->resolveNamespaces(k8s_namespaces);

        for( const string& k8s_namespace : namespaces ){

            v1_deployment_list_t *deployment_list = NULL;
            deployment_list = AppsV1API_listNamespacedDeployment(const_cast<apiClient_t*>(api_client.get()), 
                                                const_cast<char*>(k8s_namespace.c_str()),   /*namespace */
                                                NULL,    /* pretty */
                                                NULL,    /* allowWatchBookmarks */
                                                NULL,    /* continue */
                                                NULL,    /* fieldSelector */
                                                NULL,    /* labelSelector */
                                                NULL,    /* limit */
                                                NULL,    /* resourceVersion */
                                                NULL,    /* resourceVersionMatch */
                                                NULL,    /* sendInitialEvents */
                                                NULL,    /* timeoutSeconds */
                                                NULL     /* watch */
                );

            //fmt::print("The return code of HTTP request={}\n", apiClient->response_code);

            if( deployment_list ){

                fmt::print("Get deployment list for namespace '{}':\n", k8s_namespace);

                listEntry_t *listEntry = NULL;
                v1_deployment_t *deployment = NULL;
                list_ForEach(listEntry, deployment_list->items) {
                    deployment = (v1_deployment_t *)listEntry->data;
                    fmt::print("\tThe deployment name: {}\n", deployment->metadata->name);
                }
                v1_deployment_list_free(deployment_list);
                deployment_list = NULL;

            }else{

                fmt::print("Cannot get any deployment.\n");

            }

        }


    }


    set<string> KubernetesClient::resolveNamespaces( const vector<string>& k8s_namespaces ) const{

        // if "all" is in the list, then get all namespaces
        set<string> namespaces( k8s_namespaces.begin(), k8s_namespaces.end() );

        if( namespaces.find("all") != namespaces.end() ){
            auto all_namespaces = this->getNamespaces();
            namespaces.insert(all_namespaces.begin(), all_namespaces.end());
            namespaces.erase("all");
        }

        return namespaces;

    }

}