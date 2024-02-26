#include "KubernetesClient.h"

extern "C" {
    #include <kube_config.h>
    #include <apiClient.h>
    #include <CoreV1API.h>
    #include <AppsV1API.h>
    #include <ApiextensionsV1API.h>
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

#include "cjson.h"


namespace kubepp{

    KubernetesClient::KubernetesClient( const string& base_path )
        :base_path(base_path)
    {

        int rc = load_kube_config(&detected_base_path, &sslConfig, &apiKeys, NULL);
        if (rc != 0) {
            throw std::runtime_error("Cannot load kubernetes configuration.");
        }

        fmt::print("The detected base path: {}\n", detected_base_path);

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


    void KubernetesClient::displayNodes() const{

        spdlog::info( "Listing nodes:" );
        cout << this->getNodes().dump(4) << endl;

    }


    vector<string> KubernetesClient::getNamespaceNames() const{

        std::shared_ptr<v1_namespace_list_t> namespace_list( 
                                                CoreV1API_listNamespace(
                                                    const_cast<apiClient_t*>(api_client.get()), 
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
                                                ), 
                                                v1_namespace_list_free
                                            );

        vector<string> namespaces;

        if( namespace_list ){

            listEntry_t *listEntry = NULL;
            v1_namespace_t *_namespace = NULL;
            list_ForEach(listEntry, namespace_list->items) {
                _namespace = (v1_namespace_t *)listEntry->data;
                namespaces.push_back(_namespace->metadata->name);
            }

        }else{

            spdlog::error("Cannot get any namespace names.");

        }

        return namespaces;

    }




    json KubernetesClient::getPods( const vector<string>& k8s_namespaces ) const{

        auto namespaces = this->resolveNamespaces(k8s_namespaces);

        json pods = json::array();

        for( const string& k8s_namespace : namespaces ){

            std::shared_ptr<v1_pod_list_t> pod_list( 
                                                    CoreV1API_listNamespacedPod(
                                                        const_cast<apiClient_t*>(api_client.get()), 
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
                                                    ),
                                                    v1_pod_list_free
                                                );


            //fmt::print("The return code of HTTP request={}\n", apiClient->response_code);

            if( pod_list ){

                //fmt::print("Get pod list for namespace '{}':\n", k8s_namespace);

                listEntry_t *listEntry = NULL;
                v1_pod_t *pod = NULL;

                //iterate through the list of pods

                list_ForEach(listEntry, pod_list->items) {

                    pod = (v1_pod_t *)listEntry->data;

                    json pod_json = cjson( v1_pod_convertToJSON(pod) ).toJson();

                    pod_json["apiVersion"] = "v1";
                    pod_json["kind"] = "Pod";
                    pod_json["metadata"] = cjson( v1_object_meta_convertToJSON(pod->metadata) ).toJson();
                    pod_json["spec"] = cjson( v1_pod_spec_convertToJSON(pod->spec) ).toJson();

                    //CoreV1API_listNamespacedPod doesn't parse the statuses, even though they are in the response payload: https://github.com/kubernetes-client/c/issues/221

                    pod_json["status"] = cjson( v1_pod_status_convertToJSON(pod->status) ).toJson();

                    pods.push_back(pod_json);

                }
                

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

            std::shared_ptr<v1_deployment_list_t> deployment_list( 
                                                    AppsV1API_listNamespacedDeployment(
                                                        const_cast<apiClient_t*>(api_client.get()), 
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
                                                    ),
                                                    v1_deployment_list_free
                                                );

            //fmt::print("The return code of HTTP request={}\n", apiClient->response_code);

            if( deployment_list ){

                //fmt::print("Get deployment list for namespace '{}':\n", k8s_namespace);

                listEntry_t *listEntry = NULL;
                v1_deployment_t *deployment = NULL;

                list_ForEach(listEntry, deployment_list->items) {

                    deployment = (v1_deployment_t *)listEntry->data;

                    json deployment_json = cjson( v1_deployment_convertToJSON(deployment) ).toJson();

                    deployment_json["apiVersion"] = "apps/v1";
                    deployment_json["kind"] = "Deployment";

                    //deployment_json["metadata"] = cjson( v1_object_meta_convertToJSON(deployment->metadata) ).toJson();
                    //deployment_json["spec"] = cjson( v1_deployment_spec_convertToJSON(deployment->spec) ).toJson();
                    //deployment_json["status"] = cjson( v1_deployment_status_convertToJSON(deployment->status) ).toJson();

                    deployments.push_back(deployment_json);

                }

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

            std::shared_ptr<core_v1_event_list_t> event_list( 
                                                    CoreV1API_listNamespacedEvent(
                                                        const_cast<apiClient_t*>(api_client.get()), 
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
                                                    ),
                                                    core_v1_event_list_free
                                                );


            //fmt::print("The return code of HTTP request={}\n", apiClient->response_code);

            if( event_list ){

                //fmt::print("Get event list for namespace '{}':\n", k8s_namespace);

                listEntry_t *listEntry = NULL;
                core_v1_event_t *event = NULL;
                list_ForEach(listEntry, event_list->items) {
                    event = (core_v1_event_t *)listEntry->data;

                    json event_json = cjson( core_v1_event_convertToJSON(event) ).toJson();

                    event_json["apiVersion"] = "v1";
                    event_json["kind"] = "Event";

                    events.push_back(event_json);

                }

            }else{

                fmt::print("Cannot get any events.\n");

            }

        }

        return events;

    }


    json KubernetesClient::getPodLogs( const string& k8s_namespace, const string& pod_name, const string& container ) const{


        json logs = json::object();

            // read log of the specified Pod
            //
            //char* CoreV1API_readNamespacedPodLog(apiClient_t *apiClient, char *name, char *_namespace, char *container, int *follow, int *insecureSkipTLSVerifyBackend, int *limitBytes, char *pretty, int *previous, int *sinceSeconds, int *tailLines, int *timestamps);

            std::shared_ptr<char> log( 
                                        CoreV1API_readNamespacedPodLog(
                                            const_cast<apiClient_t*>(api_client.get()), 
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
                                        )
                                    );



            if( log ){

                logs["namespace"] = k8s_namespace;
                logs["type"] = "log";
                logs["name"] = pod_name;
                logs["container"] = container;
                logs["log"] = string((char*)log.get());

            }

        return logs;


    }


    json KubernetesClient::getNodes() const{

        json nodes = json::array();

        std::shared_ptr<v1_node_list_t> node_list( 
                                    CoreV1API_listNode(
                                        const_cast<apiClient_t*>(api_client.get()), 
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
                                    ),
                                    v1_node_list_free
                                );

        if( node_list ){

            listEntry_t *listEntry = NULL;
            v1_node_t *node = NULL;
            list_ForEach(listEntry, node_list->items) {
                node = (v1_node_t *)listEntry->data;

                json node_json = cjson( v1_node_convertToJSON(node) ).toJson();

                node_json["apiVersion"] = "v1";
                node_json["kind"] = "Node";

                nodes.push_back(node_json);

            }

        }else{

            fmt::print("Cannot get any nodes.\n");

        }

        return nodes;

    }


    json KubernetesClient::createCustomResourceDefinition( const json& custom_resource_definition ) const{

        json response = json::object();

        cjson crd_cjson(custom_resource_definition);
        if( !crd_cjson ){
            fmt::print("Error before: [%s]\n", cJSON_GetErrorPtr());
            throw std::runtime_error("Cannot parse the custom resource definition JSON.");
        }

        // Now, use the cJSON object to parse into the v1_custom_resource_definition_t structure

        std::shared_ptr<v1_custom_resource_definition_t> crd( 
                                                            v1_custom_resource_definition_parseFromJSON(crd_cjson.get()), 
                                                            v1_custom_resource_definition_free 
                                                        );

        std::shared_ptr<v1_custom_resource_definition_t> created_custom_resource_definition( 
                                                            ApiextensionsV1API_createCustomResourceDefinition(
                                                                const_cast<apiClient_t*>(api_client.get()), 
                                                                crd.get(), 
                                                                NULL, 
                                                                NULL, 
                                                                NULL, 
                                                                NULL
                                                            ),
                                                            v1_custom_resource_definition_free
                                                        );

        if( created_custom_resource_definition ){
            cjson cjson_response(v1_custom_resource_definition_convertToJSON(created_custom_resource_definition.get()));
            if( !cjson_response ){
                return response;
            }
            response = cjson_response.toJson();
        }

        return response;
        
    }


    json KubernetesClient::deleteCustomResourceDefinition( const json& custom_resource_definition ) const{
            
        json response = json::object();

        //check to see if the name is specified
        if( !custom_resource_definition.contains("metadata") || !custom_resource_definition["metadata"].contains("name") || !custom_resource_definition["metadata"]["name"].is_string() || custom_resource_definition["metadata"]["name"].get<string>().empty() ){
            throw std::runtime_error("The custom resource definition must have a 'metadata.name' field that is a non-empty string.");
        }
        const string name = custom_resource_definition["metadata"]["name"].get<string>();

        std::shared_ptr<v1_status_t> status( 
                                            ApiextensionsV1API_deleteCustomResourceDefinition(
                                                const_cast<apiClient_t*>(api_client.get()), 
                                                const_cast<char*>(name.c_str()),
                                                NULL, /* pretty */
                                                NULL, /* dryRun */
                                                NULL, /* gracePeriodSeconds */
                                                NULL, /* orphanDependents */
                                                NULL, /* propagationPolicy */
                                                NULL  /* delete options */
                                            ),
                                            v1_status_free
                                        );


        if( status ){
            cjson cjson_response(v1_status_convertToJSON(status.get()));
            if( !cjson_response ){
                return response;
            }
            response = cjson_response.toJson();
        }

        return response;

    }





    json KubernetesClient::createPod( const json& pod ) const{

        json response = json::object();

        cjson pod_cjson(pod);
        if( !pod_cjson ){
            fmt::print("Error before: [%s]\n", cJSON_GetErrorPtr());
            throw std::runtime_error("Cannot parse the pod JSON.");
        }
        
        //check to see if the namespace is specified
        if( !pod.contains("metadata") || !pod["metadata"].contains("namespace") || !pod["metadata"]["namespace"].is_string() || pod["metadata"]["namespace"].get<string>().empty() ){
            throw std::runtime_error("The pod must have a 'metadata.namespace' field that is a non-empty string.");
        }
        const string k8s_namespace = pod["metadata"]["namespace"].get<string>();


        std::shared_ptr<v1_pod_t> pod_type( v1_pod_parseFromJSON(pod_cjson.get()), v1_pod_free );
        std::shared_ptr<v1_pod_t> created_pod( 
                                            CoreV1API_createNamespacedPod(
                                                const_cast<apiClient_t*>(api_client.get()), 
                                                const_cast<char*>(k8s_namespace.c_str()),
                                                pod_type.get(),
                                                NULL, /* pretty */
                                                NULL, /* dryRun */
                                                NULL, /* fieldManager */
                                                NULL  /* create options */
                                            ),
                                            v1_pod_free
                                        );


        if( created_pod ){

            cjson cjson_response(v1_pod_convertToJSON(created_pod.get()));
            if( !cjson_response ){
                return response;
            }
            response = cjson_response.toJson();

        } else {
            fmt::print("Cannot create a pod.\n");
        }

        return response;
        
    }


    json KubernetesClient::deletePod( const json& pod ) const{

        json response = json::object();

        //check to see if the namespace is specified
        if( !pod.contains("metadata") || !pod["metadata"].contains("namespace") || !pod["metadata"]["namespace"].is_string() || pod["metadata"]["namespace"].get<string>().empty() ){
            throw std::runtime_error("The pod must have a 'metadata.namespace' field that is a non-empty string.");
        }
        const string k8s_namespace = pod["metadata"]["namespace"].get<string>();

        //check to see if the name is specified
        if( !pod.contains("metadata") || !pod["metadata"].contains("name") || !pod["metadata"]["name"].is_string() || pod["metadata"]["name"].get<string>().empty() ){
            throw std::runtime_error("The pod must have a 'metadata.name' field that is a non-empty string.");
        }
        const string pod_name = pod["metadata"]["name"].get<string>();


        std::shared_ptr<v1_pod_t> pod_object(
                                            CoreV1API_deleteNamespacedPod(
                                                const_cast<apiClient_t*>(api_client.get()), 
                                                const_cast<char*>(pod_name.c_str()),
                                                const_cast<char*>(k8s_namespace.c_str()),
                                                NULL, /* pretty */                                            
                                                NULL, /* dryRun */
                                                NULL, /* gracePeriodSeconds */
                                                NULL, /* orphanDependents */
                                                NULL, /* propagationPolicy */
                                                NULL /* delete options */
                                            ), 
                                            v1_pod_free 
                                        );

        if( pod_object ){

            cjson cjson_response(v1_pod_convertToJSON(pod_object.get()));
            if( !cjson_response ){
                return response;
            }

            response = cjson_response.toJson();

        }else{
            fmt::print("Cannot delete a pod.\n");
        }

        return response;

    }










    set<string> KubernetesClient::resolveNamespaces( const vector<string>& k8s_namespaces ) const{

        // if "all" is in the list, then get all namespaces
        set<string> namespaces( k8s_namespaces.begin(), k8s_namespaces.end() );

        if( namespaces.find("all") != namespaces.end() ){
            auto all_namespaces = this->getNamespaceNames();
            namespaces.insert(all_namespaces.begin(), all_namespaces.end());
            namespaces.erase("all");
        }

        return namespaces;

    }



    json KubernetesClient::createResources( const json& resources ) const{

        //if the resources is an array, then iterate through the array and create each resource

        if( resources.is_array() ){

            json responses = json::array();

            for( const auto& resource : resources ){

                responses.push_back( this->createResource(resource) );

            }

            return responses;

        }else{

            return this->createResource(resources);

        }

    }

    
    json KubernetesClient::createResource( const json& resource ) const{

        if( !resource.is_object() ){
            throw std::runtime_error("The resource must be a JSON object.");
        }

        json response = json::object();

        // determine the kind of resource

        // ensure that the resource has a 'kind' field
        if( !resource.contains("kind") || !resource["kind"].is_string() || resource["kind"].get<string>().empty() ){
            throw std::runtime_error("The resource must have a 'kind' field that is a non-empty string.");
        }
        string kind = resource["kind"].get<string>();


        if( kind == "CustomResourceDefinition" ){
            response = this->createCustomResourceDefinition(resource);
        }else if( kind == "Pod" ){
            response = this->createPod(resource);        
        }else{
            fmt::print("The kind of resource, '{}', is not supported.\n", kind);
            throw std::runtime_error( fmt::format("The kind of resource, '{}', is not supported.", kind) );
        }

        return response;

    }



    json KubernetesClient::deleteResources( const json& resources ) const{

        //if the resources is an array, then iterate through the array and delete each resource

        if( resources.is_array() ){

            json responses = json::array();

            for( const auto& resource : resources ){

                responses.push_back( this->deleteResource(resource) );

            }

            return responses;

        }else{

            return this->deleteResource(resources);

        }

    }


    json KubernetesClient::deleteResource( const json& resource ) const{

        if( !resource.is_object() ){
            throw std::runtime_error("The resource must be a JSON object.");
        }

        json response = json::object();

        // determine the kind of resource

        // ensure that the resource has a 'kind' field
        if( !resource.contains("kind") || !resource["kind"].is_string() || resource["kind"].get<string>().empty() ){
            throw std::runtime_error("The resource must have a 'kind' field that is a non-empty string.");
        }
        string kind = resource["kind"].get<string>();


        if( kind == "CustomResourceDefinition" ){
            response = this->deleteCustomResourceDefinition(resource);
        }else if( kind == "Pod" ){
            response = this->deletePod(resource);
        }else{
            fmt::print("The kind of resource, '{}', is not supported.\n", kind);
            throw std::runtime_error( fmt::format("The kind of resource, '{}', is not supported.", kind) );
        }

        return response;

    }
    
}