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


#include "json.hpp"


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

    void KubernetesClient::displayWorkloads() const{

        spdlog::info( "Listing pods:" );
        cout << this->getPods().dump(4) << endl;

        spdlog::info( "Listing deployments:" );
        cout << this->getDeployments().dump(4) << endl;

    }

    void KubernetesClient::displayEvents() const{

        spdlog::info( "Listing events:" );
        cout << this->getEvents().dump(4) << endl;

    }


    void KubernetesClient::displayLogs() const{

        spdlog::info( "Listing logs:" );

        auto pods = this->getPods();

        for( const auto& pod : pods ){
            for( const auto& container : pod["containers"] ){                
                cout << this->getPodLogs(pod["namespace"], pod["name"], container).dump(4) << endl;
            }            
        }

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




    json KubernetesClient::getPods( const vector<string>& k8s_namespaces ) const{

        auto namespaces = this->resolveNamespaces(k8s_namespaces);

        json pods = json::array();

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

                //fmt::print("Get pod list for namespace '{}':\n", k8s_namespace);

                listEntry_t *listEntry = NULL;
                v1_pod_t *pod = NULL;
                list_ForEach(listEntry, pod_list->items) {
                    pod = (v1_pod_t *)listEntry->data;
                    //fmt::print("\tThe pod name: {}\n", pod->metadata->name);
                    

                    // add the container names
                    json containers = json::array();
                    listEntry_t *container_list_entry = NULL;
                    v1_container_t *container = NULL;
                    list_ForEach(container_list_entry, pod->spec->containers) {
                        container = (v1_container_t *)container_list_entry->data;
                        containers.push_back( container->name );
                    }
                    
                    pods.push_back( json{ {"namespace", k8s_namespace}, {"type","pod"}, {"name", string(pod->metadata->name)}, {"containers", containers} } );

                }
                v1_pod_list_free(pod_list);
                pod_list = NULL;

            }else{

                fmt::print("Cannot get any pod.\n");

            }

        }

        return pods;


    }





    json KubernetesClient::getDeployments( const vector<string>& k8s_namespaces ) const{

        auto namespaces = this->resolveNamespaces(k8s_namespaces);

        json deployments = json::array();

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

                //fmt::print("Get deployment list for namespace '{}':\n", k8s_namespace);

                listEntry_t *listEntry = NULL;
                v1_deployment_t *deployment = NULL;
                list_ForEach(listEntry, deployment_list->items) {
                    deployment = (v1_deployment_t *)listEntry->data;
                    //fmt::print("\tThe deployment name: {}\n", deployment->metadata->name);
                    deployments.push_back( json{ {"namespace", k8s_namespace}, {"type","deployment"}, {"name", deployment->metadata->name} } );
                }
                v1_deployment_list_free(deployment_list);
                deployment_list = NULL;

            }else{

                fmt::print("Cannot get any deployment.\n");

            }

        }

        return deployments;

    }


    json KubernetesClient::getEvents( const vector<string>& k8s_namespaces ) const{

        auto namespaces = this->resolveNamespaces(k8s_namespaces);

        json events = json::array();

        for( const string& k8s_namespace : namespaces ){

            core_v1_event_list_t *event_list = NULL;
            event_list = CoreV1API_listNamespacedEvent(const_cast<apiClient_t*>(api_client.get()), 
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

            if( event_list ){

                //fmt::print("Get event list for namespace '{}':\n", k8s_namespace);

                listEntry_t *listEntry = NULL;
                core_v1_event_t *event = NULL;
                list_ForEach(listEntry, event_list->items) {
                    event = (core_v1_event_t *)listEntry->data;

                    // let's add all of the fields from the event

                    json event_json = json::object();
                    event_json["namespace"] = k8s_namespace;                    
                    event_json["type"] = "event";

                    event_json["reason"] = string(event->reason);
                    event_json["message"] = string(event->message);
                    event_json["source"] = json{ {"component", string(event->source->component)}, {"host", string(event->source->host)} };
                    event_json["first_timestamp"] = string(event->first_timestamp);
                    event_json["last_timestamp"] = string(event->last_timestamp);
                    event_json["count"] = event->count;
                    event_json["event_type"] = string(event->type);
                    event_json["reporting_component"] = string(event->reporting_component);
                    event_json["reporting_instance"] = string(event->reporting_instance);

                    events.push_back(event_json);

                }
                core_v1_event_list_free(event_list);
                event_list = NULL;

            }else{

                fmt::print("Cannot get any event.\n");

            }

        }

        return events;

    }


    json KubernetesClient::getPodLogs( const string& k8s_namespace, const string& pod_name, const string& container ) const{


        json logs = json::object();

            // read log of the specified Pod
            //
            //char* CoreV1API_readNamespacedPodLog(apiClient_t *apiClient, char *name, char *_namespace, char *container, int *follow, int *insecureSkipTLSVerifyBackend, int *limitBytes, char *pretty, int *previous, int *sinceSeconds, int *tailLines, int *timestamps);

            char* log = CoreV1API_readNamespacedPodLog(const_cast<apiClient_t*>(api_client.get()), 
                                                const_cast<char*>(pod_name.c_str()),   /*name */
                                                const_cast<char*>(k8s_namespace.c_str()),   /*namespace */
                                                const_cast<char*>(container.c_str()),    /* container */
                                                NULL,    /* follow */
                                                NULL,    /* insecureSkipTLSVerifyBackend */
                                                NULL,    /* limitBytes */
                                                NULL,    /* pretty */
                                                NULL,    /* previous */
                                                NULL,    /* sinceSeconds */
                                                NULL,    /* tailLines */
                                                NULL     /* timestamps */
            );

            if( log ){

                logs["namespace"] = k8s_namespace;
                logs["type"] = "log";
                logs["name"] = pod_name;
                logs["container"] = container;
                logs["log"] = string(log);
                
                free(log);

            }

        return logs;


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