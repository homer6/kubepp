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
        // fmt::print("Length of detected base path: {}\n", strlen(detected_base_path));
        // cout << endl;
        // fmt::print("The base path: {}\n", base_path);        
        // fmt::print("Length of base path: {}\n", strlen(base_path.c_str()));

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
                    
                    json pod_json = json::object();

                    pod_json["namespace"] = k8s_namespace;
                    pod_json["type"] = "pod";
                    pod_json["name"] = string(pod->metadata->name);
                    pod_json["containers"] = containers;

                    json container_statuses = json::array();
                    listEntry_t *container_status_list_entry = NULL;
                    v1_container_status_t *container_status = NULL;
                    list_ForEach(container_status_list_entry, pod->status->container_statuses) {
                        container_status = (v1_container_status_t *)container_status_list_entry->data;

                        json container_status_json = json::object();
                        container_status_json["name"] = string(container_status->name);
                        container_status_json["ready"] = container_status->ready;
                        
                        container_statuses.push_back(container_status_json);
                    }
                    pod_json["container_statuses"] = container_statuses;

                    
                    pods.push_back(pod_json);

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

                listEntry_t *listEntry = NULL;
                v1_deployment_t *deployment = NULL;
                list_ForEach(listEntry, deployment_list->items) {
                    deployment = (v1_deployment_t *)listEntry->data;

                    json deployment_json = json::object();
                    deployment_json["namespace"] = k8s_namespace;
                    deployment_json["type"] = "deployment";
                    deployment_json["name"] = string(deployment->metadata->name);
                    deployment_json["replicas"] = deployment->spec->replicas;
                    deployment_json["available_replicas"] = deployment->status->available_replicas;
                    deployment_json["unavailable_replicas"] = deployment->status->unavailable_replicas;
                    
                    deployments.push_back(deployment_json);
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


    json KubernetesClient::getNodes() const{

        json nodes = json::array();

        v1_node_list_t *node_list = NULL;
        node_list = CoreV1API_listNode(const_cast<apiClient_t*>(api_client.get()), 
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

        if( node_list ){

            listEntry_t *listEntry = NULL;
            v1_node_t *node = NULL;
            list_ForEach(listEntry, node_list->items) {
                node = (v1_node_t *)listEntry->data;

                json node_json = json::object();
                node_json["type"] = "node";
                node_json["name"] = string(node->metadata->name);
                node_json["creation_timestamp"] = string(node->metadata->creation_timestamp);


                node_json["spec"] = json::object();
                node_json["spec"]["pod_cidr"] = string(node->spec->pod_cidr);
                node_json["spec"]["provider_id"] = string(node->spec->provider_id);
                node_json["spec"]["unschedulable"] = node->spec->unschedulable;


                // list the taints
                node_json["taints"] = json::array();
                listEntry_t *taint_list_entry = NULL;
                v1_taint_t *taint = NULL;
                list_ForEach(taint_list_entry, node->spec->taints) {
                    taint = (v1_taint_t *)taint_list_entry->data;
                    node_json["taints"].push_back( json{ {"key", string(taint->key)}, {"value", string(taint->value)}, {"effect", string(taint->effect)} } );
                }


                // list the conditions
                node_json["conditions"] = json::array();
                listEntry_t *condition_list_entry = NULL;
                v1_node_condition_t *condition = NULL;
                list_ForEach(condition_list_entry, node->status->conditions) {
                    condition = (v1_node_condition_t *)condition_list_entry->data;
                    node_json["conditions"].push_back( json{ {"type", string(condition->type)}, {"status", string(condition->status)} } );
                }


                // list the node info
                node_json["node_info"] = json::object();
                node_json["node_info"]["architecture"] = string(node->status->node_info->architecture);
                node_json["node_info"]["boot_id"] = string(node->status->node_info->boot_id);
                node_json["node_info"]["container_runtime_version"] = string(node->status->node_info->container_runtime_version);
                node_json["node_info"]["kernel_version"] = string(node->status->node_info->kernel_version);
                node_json["node_info"]["kube_proxy_version"] = string(node->status->node_info->kube_proxy_version);
                node_json["node_info"]["kubelet_version"] = string(node->status->node_info->kubelet_version);
                node_json["node_info"]["machine_id"] = string(node->status->node_info->machine_id);
                node_json["node_info"]["operating_system"] = string(node->status->node_info->operating_system);
                node_json["node_info"]["os_image"] = string(node->status->node_info->os_image);
                node_json["node_info"]["system_uuid"] = string(node->status->node_info->system_uuid);


                // list the addresses
                node_json["addresses"] = json::array();
                listEntry_t *address_list_entry = NULL;
                v1_node_address_t *address = NULL;
                list_ForEach(address_list_entry, node->status->addresses) {
                    address = (v1_node_address_t *)address_list_entry->data;
                    node_json["addresses"].push_back( json{ {"type", string(address->type)}, {"address", string(address->address)} } );
                }

                nodes.push_back(node_json);
                
            }
            v1_node_list_free(node_list);
            node_list = NULL;

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
        v1_custom_resource_definition_t* crd = v1_custom_resource_definition_parseFromJSON(crd_cjson.get());
        v1_custom_resource_definition_t* created_custom_resource_definition = ApiextensionsV1API_createCustomResourceDefinition(const_cast<apiClient_t*>(api_client.get()), crd, NULL, NULL, NULL, NULL);

        if( created_custom_resource_definition ){
            cjson cjson_response(v1_custom_resource_definition_convertToJSON(created_custom_resource_definition));
            if( !cjson_response ){
                return response;
            }
            response = cjson_response.toJson();

            // Free the created custom resource definition object
            v1_custom_resource_definition_free(created_custom_resource_definition);
        }

        // Free the original CRD object
        v1_custom_resource_definition_free(crd);

        return response;
        
    }


    json KubernetesClient::deleteCustomResourceDefinition( const json& custom_resource_definition ) const{
            
        json response = json::object();

        //check to see if the name is specified
        if( !custom_resource_definition.contains("metadata") || !custom_resource_definition["metadata"].contains("name") || !custom_resource_definition["metadata"]["name"].is_string() || custom_resource_definition["metadata"]["name"].get<string>().empty() ){
            throw std::runtime_error("The custom resource definition must have a 'metadata.name' field that is a non-empty string.");
        }
        const string name = custom_resource_definition["metadata"]["name"].get<string>();

        v1_status_t* status = ApiextensionsV1API_deleteCustomResourceDefinition(
                                            const_cast<apiClient_t*>(api_client.get()), 
                                            const_cast<char*>(name.c_str()),
                                            NULL, /* pretty */
                                            NULL, /* dryRun */
                                            NULL, /* gracePeriodSeconds */
                                            NULL, /* orphanDependents */
                                            NULL, /* propagationPolicy */
                                            NULL  /* delete options */
        );


        if( status ){
            cjson cjson_response(v1_status_convertToJSON(status));
            if( !cjson_response ){
                return response;
            }
            response = cjson_response.toJson();

            // Free the status object
            v1_status_free(status);
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

        // Now, use the cJSON object to parse into the v1_pod_t structure
        v1_pod_t* pod_type = v1_pod_parseFromJSON(pod_cjson.get());
        v1_pod_t* created_pod = CoreV1API_createNamespacedPod(
                                            const_cast<apiClient_t*>(api_client.get()),                                            
                                            const_cast<char*>(k8s_namespace.c_str()),
                                            pod_type,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL
        );

        if( created_pod ){

            cjson cjson_response(v1_pod_convertToJSON(created_pod));
            if( !cjson_response ){
                return response;
            }
            response = cjson_response.toJson();

            // Free the created pod object
            v1_pod_free(created_pod);

        } else {
            fmt::print("Cannot create a pod.\n");
        }

        // Free the original pod object
        v1_pod_free(pod_type);

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


        v1_pod_t* pod_object = CoreV1API_deleteNamespacedPod(
                                            const_cast<apiClient_t*>(api_client.get()), 
                                            const_cast<char*>(pod_name.c_str()),
                                            const_cast<char*>(k8s_namespace.c_str()),
                                            NULL, /* pretty */                                            
                                            NULL, /* dryRun */
                                            NULL, /* gracePeriodSeconds */
                                            NULL, /* orphanDependents */
                                            NULL, /* propagationPolicy */
                                            NULL /* delete options */  
        );


        if( pod_object ){

            cjson cjson_response(v1_pod_convertToJSON(pod_object));
            if( !cjson_response ){
                return response;
            }

            // Convert cJSON to nlohmann JSON
            response = cjson_response.toJson();

            // Free the pod object
            v1_pod_free(pod_object);  

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