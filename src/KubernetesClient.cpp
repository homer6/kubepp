#include "KubernetesClient.h"

extern "C" {
    #include <kube_config.h>
    #include <apiClient.h>
    #include <generic.h>
    #include <CoreV1API.h>
    #include <AppsV1API.h>
    #include <ApiextensionsV1API.h>
    #include <CustomObjectsAPI.h>
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



    // void KubernetesClient::displayWorkloads() const{

    //     spdlog::info( "Listing pods:" );
    //     cout << this->getPods().dump(4) << endl;

    //     spdlog::info( "Listing deployments:" );
    //     cout << this->getDeployments().dump(4) << endl;

    // }

    // void KubernetesClient::displayEvents() const{

    //     spdlog::info( "Listing events:" );
    //     cout << this->getEvents().dump(4) << endl;

    // }


    // void KubernetesClient::displayLogs() const{

    //     spdlog::info( "Listing logs:" );

    //     auto logs = this->getPodLogs( "kube-system", "svclb-traefik-06f20d2a-684jx", "lb-tcp-80" );

    //     cout << logs.dump(4) << endl;

    // }


    // void KubernetesClient::displayNodes() const{

    //     spdlog::info( "Listing nodes:" );
    //     cout << this->getNodes().dump(4) << endl;

    // }


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




    // json KubernetesClient::getPods( const vector<string>& k8s_namespaces ) const{

    //     auto namespaces = this->resolveNamespaces(k8s_namespaces);

    //     json pods = json::array();

    //     for( const string& k8s_namespace : namespaces ){

    //         // std::shared_ptr<v1_pod_list_t> pod_list( 
    //         //                                         CoreV1API_listNamespacedPod(
    //         //                                             const_cast<apiClient_t*>(api_client.get()), 
    //         //                                             const_cast<char*>(k8s_namespace.c_str()),   /*namespace */
    //         //                                             NULL,    /* pretty */
    //         //                                             NULL,    /* allowWatchBookmarks */
    //         //                                             NULL,    /* continue */
    //         //                                             NULL,    /* fieldSelector */
    //         //                                             NULL,    /* labelSelector */
    //         //                                             NULL,    /* limit */
    //         //                                             NULL,    /* resourceVersion */
    //         //                                             NULL,    /* resourceVersionMatch */
    //         //                                             NULL,    /* sendInitialEvents */
    //         //                                             NULL,    /* timeoutSeconds */
    //         //                                             NULL     /* watch */
    //         //                                         ),
    //         //                                         v1_pod_list_free
    //         //                                     );


    //         char *response = (char*)CoreV1API_listNamespacedPod(
    //             const_cast<apiClient_t*>(api_client.get()), 
    //             const_cast<char*>(k8s_namespace.c_str()),   /*namespace */
    //             NULL,    /* pretty */
    //             NULL,    /* allowWatchBookmarks */
    //             NULL,    /* continue */
    //             NULL,    /* fieldSelector */
    //             NULL,    /* labelSelector */
    //             NULL,    /* limit */
    //             NULL,    /* resourceVersion */
    //             NULL,    /* resourceVersionMatch */
    //             NULL,    /* sendInitialEvents */
    //             NULL,    /* timeoutSeconds */
    //             NULL     /* watch */
    //         );

    //         // std::shared_ptr<char> pod_list( 
    //         //                                         (char*)CoreV1API_listNamespacedPod(
    //         //                                             const_cast<apiClient_t*>(api_client.get()), 
    //         //                                             const_cast<char*>(k8s_namespace.c_str()),   /*namespace */
    //         //                                             NULL,    /* pretty */
    //         //                                             NULL,    /* allowWatchBookmarks */
    //         //                                             NULL,    /* continue */
    //         //                                             NULL,    /* fieldSelector */
    //         //                                             NULL,    /* labelSelector */
    //         //                                             NULL,    /* limit */
    //         //                                             NULL,    /* resourceVersion */
    //         //                                             NULL,    /* resourceVersionMatch */
    //         //                                             NULL,    /* sendInitialEvents */
    //         //                                             NULL,    /* timeoutSeconds */
    //         //                                             NULL     /* watch */
    //         //                                         ),
    //         //                                         free
    //         //                                     );


    //         //fmt::print("The return code of HTTP request={}\n", apiClient->response_code);

    //         if( response ){

    //             //fmt::print("Get pod list for namespace '{}':\n", k8s_namespace);

                
    //             //std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    //             //std::string my_string = converter.to_bytes( (wchar_t*) pod_list.get() );

    //             //string my_string( cjson(pod_list.get()) );


    //             //output the string as hexidecimal
    //             // for( char c : my_string ){

    //             //     cout << std::hex << (int)c << " ";

    //             // }

    //             //return json::object();


    //             //json pod_list_json = json::parse(my_string);

    //             json pod_list_json = cjson( response ).toJson();
    // //            cout << pod_list_json.dump(4) << endl;


    //             if( pod_list_json.contains("items") && pod_list_json["items"].is_array() ){
    //                 for( json pod_json : pod_list_json["items"] ){
    //                     pod_json["apiVersion"] = "v1";
    //                     pod_json["kind"] = "Pod";
    //                     pods.push_back(pod_json);
    //                 }
    //             }

    //             /*
    //             listEntry_t *listEntry = NULL;
    //             v1_pod_t *pod = NULL;

    //             //iterate through the list of pods

    //             list_ForEach(listEntry, pod_list->items) {

    //                 pod = (v1_pod_t *)listEntry->data;

    //                 //json pod_json = json::object();

    //                 json pod_json = cjson( v1_pod_convertToJSON(pod) ).toJson();

    //                 //pod_json["apiVersion"] = "v1";
    //                 //pod_json["kind"] = "Pod";
    //                 //pod_json["metadata"] = cjson( v1_object_meta_convertToJSON(pod->metadata) ).toJson();
    //                 //pod_json["spec"] = cjson( v1_pod_spec_convertToJSON(pod->spec) ).toJson();

    //                 //pod_json["status"] = cjson( v1_pod_status_convertToJSON(pod->status) ).toJson();

    //                 pods.push_back(pod_json);

    //             }
                
    //             */
    //         }else{

    //             fmt::print("Cannot get any pod.\n");

    //         }

    //     }

    //     return pods;


    // }





    // json KubernetesClient::getDeployments( const vector<string>& k8s_namespaces ) const{

    //     auto namespaces = this->resolveNamespaces(k8s_namespaces);

    //     json deployments = json::array();

    //     for( const string& k8s_namespace : namespaces ){

    //         std::shared_ptr<v1_deployment_list_t> deployment_list( 
    //                                                 AppsV1API_listNamespacedDeployment(
    //                                                     const_cast<apiClient_t*>(api_client.get()), 
    //                                                     const_cast<char*>(k8s_namespace.c_str()),   /*namespace */
    //                                                     NULL,    /* pretty */
    //                                                     NULL,    /* allowWatchBookmarks */
    //                                                     NULL,    /* continue */
    //                                                     NULL,    /* fieldSelector */
    //                                                     NULL,    /* labelSelector */
    //                                                     NULL,    /* limit */
    //                                                     NULL,    /* resourceVersion */
    //                                                     NULL,    /* resourceVersionMatch */
    //                                                     NULL,    /* sendInitialEvents */
    //                                                     NULL,    /* timeoutSeconds */
    //                                                     NULL     /* watch */
    //                                                 ),
    //                                                 v1_deployment_list_free
    //                                             );

    //         //fmt::print("The return code of HTTP request={}\n", apiClient->response_code);

    //         if( deployment_list ){

    //             //fmt::print("Get deployment list for namespace '{}':\n", k8s_namespace);

    //             listEntry_t *listEntry = NULL;
    //             v1_deployment_t *deployment = NULL;

    //             list_ForEach(listEntry, deployment_list->items) {

    //                 deployment = (v1_deployment_t *)listEntry->data;

    //                 json deployment_json = cjson( v1_deployment_convertToJSON(deployment) ).toJson();

    //                 deployment_json["apiVersion"] = "apps/v1";
    //                 deployment_json["kind"] = "Deployment";

    //                 //deployment_json["metadata"] = cjson( v1_object_meta_convertToJSON(deployment->metadata) ).toJson();
    //                 //deployment_json["spec"] = cjson( v1_deployment_spec_convertToJSON(deployment->spec) ).toJson();
    //                 //deployment_json["status"] = cjson( v1_deployment_status_convertToJSON(deployment->status) ).toJson();

    //                 deployments.push_back(deployment_json);

    //             }

    //         }else{

    //             fmt::print("Cannot get any deployment.\n");

    //         }

    //     }

    //     return deployments;

    // }


    // json KubernetesClient::getEvents( const vector<string>& k8s_namespaces ) const{

    //     auto namespaces = this->resolveNamespaces(k8s_namespaces);

    //     json events = json::array();

    //     for( const string& k8s_namespace : namespaces ){

    //         std::shared_ptr<core_v1_event_list_t> event_list( 
    //                                                 CoreV1API_listNamespacedEvent(
    //                                                     const_cast<apiClient_t*>(api_client.get()), 
    //                                                     const_cast<char*>(k8s_namespace.c_str()),   /*namespace */
    //                                                     NULL,    /* pretty */
    //                                                     NULL,    /* allowWatchBookmarks */
    //                                                     NULL,    /* continue */
    //                                                     NULL,    /* fieldSelector */
    //                                                     NULL,    /* labelSelector */
    //                                                     NULL,    /* limit */
    //                                                     NULL,    /* resourceVersion */
    //                                                     NULL,    /* resourceVersionMatch */
    //                                                     NULL,    /* sendInitialEvents */
    //                                                     NULL,    /* timeoutSeconds */
    //                                                     NULL     /* watch */
    //                                                 ),
    //                                                 core_v1_event_list_free
    //                                             );


    //         //fmt::print("The return code of HTTP request={}\n", apiClient->response_code);

    //         if( event_list ){

    //             //fmt::print("Get event list for namespace '{}':\n", k8s_namespace);

    //             listEntry_t *listEntry = NULL;
    //             core_v1_event_t *event = NULL;
    //             list_ForEach(listEntry, event_list->items) {
    //                 event = (core_v1_event_t *)listEntry->data;

    //                 json event_json = cjson( core_v1_event_convertToJSON(event) ).toJson();

    //                 event_json["apiVersion"] = "v1";
    //                 event_json["kind"] = "Event";

    //                 events.push_back(event_json);

    //             }

    //         }else{

    //             fmt::print("Cannot get any events.\n");

    //         }

    //     }

    //     return events;

    // }


    json KubernetesClient::getPodLogs( const string& k8s_namespace, const string& pod_name, const string& container ) const{

        json logs = json::object();

        char* log_string = CoreV1API_readNamespacedPodLog(
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
                                    );

        //fmt::print("The return code of HTTP request={}\n", api_client->response_code);

        logs["namespace"] = k8s_namespace;
        logs["type"] = "log";
        logs["name"] = pod_name;
        logs["container"] = container;
        logs["log"] = "";

        if( log_string ){
            logs["log"] = string(log_string);
            free(log_string);
        }

        return logs;

    }


    // json KubernetesClient::getNodes() const{

    //     json nodes = json::array();

    //     std::shared_ptr<v1_node_list_t> node_list( 
    //                                 CoreV1API_listNode(
    //                                     const_cast<apiClient_t*>(api_client.get()), 
    //                                     NULL,    /* pretty */
    //                                     NULL,    /* allowWatchBookmarks */
    //                                     NULL,    /* continue */
    //                                     NULL,    /* fieldSelector */
    //                                     NULL,    /* labelSelector */
    //                                     NULL,    /* limit */
    //                                     NULL,    /* resourceVersion */
    //                                     NULL,    /* resourceVersionMatch */
    //                                     NULL,    /* sendInitialEvents */
    //                                     NULL,    /* timeoutSeconds */
    //                                     NULL     /* watch */
    //                                 ),
    //                                 v1_node_list_free
    //                             );

    //     if( node_list ){

    //         listEntry_t *listEntry = NULL;
    //         v1_node_t *node = NULL;
    //         list_ForEach(listEntry, node_list->items) {
    //             node = (v1_node_t *)listEntry->data;

    //             json node_json = cjson( v1_node_convertToJSON(node) ).toJson();

    //             node_json["apiVersion"] = "v1";
    //             node_json["kind"] = "Node";

    //             nodes.push_back(node_json);

    //         }

    //     }else{

    //         fmt::print("Cannot get any nodes.\n");

    //     }

    //     return nodes;

    // }



    // json KubernetesClient::getCustomResourceDefinition( const string& name ) const{

    //     json crd_json = json::object();

    //     /*
    //         // read the specified CustomResourceDefinition
    //         //
    //         v1_custom_resource_definition_t*
    //         ApiextensionsV1API_readCustomResourceDefinition(apiClient_t *apiClient, char *name, char *pretty);
    //     */

    //     std::shared_ptr<v1_custom_resource_definition_t> crd( 
    //                                 ApiextensionsV1API_readCustomResourceDefinition(
    //                                     const_cast<apiClient_t*>(api_client.get()), 
    //                                     const_cast<char*>(name.c_str()),
    //                                     NULL    /* pretty */
    //                                 ),
    //                                 v1_custom_resource_definition_free
    //                             );

    //     if( crd ){

    //         crd_json = cjson( v1_custom_resource_definition_convertToJSON(crd.get()) ).toJson();

    //     }else{

    //         fmt::print("Cannot get this custom resource definition.\n");

    //     }

    //     return crd_json;

    // }


    // json KubernetesClient::createCustomResourceDefinition( const json& custom_resource_definition ) const{

    //     json response = json::object();

    //     cjson crd_cjson(custom_resource_definition);
    //     if( !crd_cjson ){
    //         fmt::print("Error before: [%s]\n", cJSON_GetErrorPtr());
    //         throw std::runtime_error("Cannot parse the custom resource definition JSON.");
    //     }

    //     cout << crd_cjson << endl;

    //     // Now, use the cJSON object to parse into the v1_custom_resource_definition_t structure

    //     std::shared_ptr<v1_custom_resource_definition_t> crd( 
    //                                                         v1_custom_resource_definition_parseFromJSON(crd_cjson.get()), 
    //                                                         v1_custom_resource_definition_free 
    //                                                     );

    //     std::shared_ptr<v1_custom_resource_definition_t> created_custom_resource_definition( 
    //                                                         ApiextensionsV1API_createCustomResourceDefinition(
    //                                                             const_cast<apiClient_t*>(api_client.get()), 
    //                                                             crd.get(), 
    //                                                             NULL, 
    //                                                             NULL, 
    //                                                             NULL, 
    //                                                             NULL
    //                                                         ),
    //                                                         v1_custom_resource_definition_free
    //                                                     );

    //     if( created_custom_resource_definition ){
    //         cjson cjson_response(v1_custom_resource_definition_convertToJSON(created_custom_resource_definition.get()));
    //         if( !cjson_response ){
    //             return response;
    //         }
    //         response = cjson_response.toJson();
    //     }

    //     return response;
        
    // }


    // json KubernetesClient::deleteCustomResourceDefinition( const json& custom_resource_definition ) const{
            
    //     json response = json::object();

    //     //check to see if the name is specified
    //     if( !custom_resource_definition.contains("metadata") || !custom_resource_definition["metadata"].contains("name") || !custom_resource_definition["metadata"]["name"].is_string() || custom_resource_definition["metadata"]["name"].get<string>().empty() ){
    //         throw std::runtime_error("The custom resource definition must have a 'metadata.name' field that is a non-empty string.");
    //     }
    //     const string name = custom_resource_definition["metadata"]["name"].get<string>();

    //     std::shared_ptr<v1_status_t> status( 
    //                                         ApiextensionsV1API_deleteCustomResourceDefinition(
    //                                             const_cast<apiClient_t*>(api_client.get()), 
    //                                             const_cast<char*>(name.c_str()),
    //                                             NULL, /* pretty */
    //                                             NULL, /* dryRun */
    //                                             NULL, /* gracePeriodSeconds */
    //                                             NULL, /* orphanDependents */
    //                                             NULL, /* propagationPolicy */
    //                                             NULL  /* delete options */
    //                                         ),
    //                                         v1_status_free
    //                                     );


    //     if( status ){
    //         cjson cjson_response(v1_status_convertToJSON(status.get()));
    //         if( !cjson_response ){
    //             return response;
    //         }
    //         response = cjson_response.toJson();
    //     }

    //     return response;

    // }


    // json KubernetesClient::getCustomResourceDefinitions() const{

    //     json crds = json::array();

    //     std::shared_ptr<v1_custom_resource_definition_list_t> crd_list( 
    //                                 ApiextensionsV1API_listCustomResourceDefinition(
    //                                     const_cast<apiClient_t*>(api_client.get()), 
    //                                     NULL,    /* pretty */
    //                                     NULL,    /* allowWatchBookmarks */
    //                                     NULL,    /* continue */
    //                                     NULL,    /* fieldSelector */
    //                                     NULL,    /* labelSelector */
    //                                     NULL,    /* limit */
    //                                     NULL,    /* resourceVersion */
    //                                     NULL,    /* resourceVersionMatch */
    //                                     NULL,    /* sendInitialEvents */
    //                                     NULL,    /* timeoutSeconds */
    //                                     NULL     /* watch */
    //                                 ),
    //                                 v1_custom_resource_definition_list_free
    //                             );

    //     if( crd_list ){

    //         listEntry_t *listEntry = NULL;
    //         v1_custom_resource_definition_t *crd = NULL;
    //         list_ForEach(listEntry, crd_list->items) {
    //             crd = (v1_custom_resource_definition_t *)listEntry->data;

    //             json crd_json = cjson( v1_custom_resource_definition_convertToJSON(crd) ).toJson();

    //             crd_json["apiVersion"] = "apiextensions.k8s.io/v1";
    //             crd_json["kind"] = "CustomResourceDefinition";

    //             crds.push_back(crd_json);

    //         }

    //     }else{

    //         fmt::print("Cannot get any custom resource definitions.\n");

    //     }

    //     return crds;

    // }




    // json KubernetesClient::createCustomResource( const json& custom_resource ) const{

    //     json response = json::object();

    //     cjson custom_resource_cjson(custom_resource);
    //     if( !custom_resource_cjson ){
    //         fmt::print("Error before: [%s]\n", cJSON_GetErrorPtr());
    //         throw std::runtime_error("Cannot parse the custom resource JSON.");
    //     }

    //     //check to see if the namespace is specified (optional for cluster-scoped resources)
    //     string k8s_namespace;
    //     if( custom_resource.contains("metadata") && custom_resource["metadata"].contains("namespace") && custom_resource["metadata"]["namespace"].is_string() && !custom_resource["metadata"]["namespace"].get<string>().empty() ){
    //         k8s_namespace = custom_resource["metadata"]["namespace"].get<string>();
    //     }

    //     //check to see if the kind is specified
    //     if( !custom_resource.contains("kind") || !custom_resource["kind"].is_string() || custom_resource["kind"].get<string>().empty() ){
    //         throw std::runtime_error("The custom resource must have a 'kind' field that is a non-empty string.");
    //     }
    //     const string kind = custom_resource["kind"].get<string>();

    //     //check to see if the apiVersion is specified
    //     if( !custom_resource.contains("apiVersion") || !custom_resource["apiVersion"].is_string() || custom_resource["apiVersion"].get<string>().empty() ){
    //         throw std::runtime_error("The custom resource must have a 'apiVersion' field that is a non-empty string.");
    //     }
    //     const string api_version = custom_resource["apiVersion"].get<string>();


    //     json all_crds = this->getCustomResourceDefinitions();

    //     // get the plural, group, and version from the custom resource definition (match with kind and api_version)
    //     // don't just use the first version, make sure they match
    //     string plural;
    //     string group;
    //     string version;



    //     /*

    //     CR:

    //             json cr = R"({
    //                 "apiVersion": "example.com/v1",
    //                 "kind": "ExampleResource",
    //                 "metadata": {
    //                     "name": "my-example-resource",
    //                     "namespace": "default"
    //                 },
    //                 "spec": {
    //                     "field1": "value1",
    //                     "field2": true
    //                 }
    //             })"_json;



    //     CRD: 
    //     "spec": {
    //         "conversion": {
    //             "strategy": "None"
    //         },
    //         "group": "example.com",
    //         "names": {
    //             "kind": "ExampleResource",
    //             "listKind": "ExampleResourceList",
    //             "plural": "exampleresources",
    //             "shortNames": [
    //                 "exr"
    //             ],
    //             "singular": "exampleresource"
    //         },
    //         "scope": "Namespaced",
    //         "versions": [
    //             {
    //                 "name": "v1",
    //                 "schema": {
    //                     "openAPIV3Schema": {
    //                         "type": "object"
    //                     }
    //                 },
    //                 "served": true,
    //                 "storage": true
    //             }
    //         ]
    //     },

        
    //     */

    //     for( const auto& crd : all_crds ){

    //         if( crd.contains("spec") && crd["spec"].contains("group") && crd["spec"]["group"].is_string() && crd["spec"]["group"].get<string>() == api_version.substr(0, api_version.find("/")) ){

    //             group = crd["spec"]["group"].get<string>();

    //             if( crd["spec"].contains("names") && crd["spec"]["names"].contains("kind") && crd["spec"]["names"]["kind"].is_string() && crd["spec"]["names"]["kind"].get<string>() == kind ){

    //                 if( crd["spec"]["names"].contains("plural") && crd["spec"]["names"]["plural"].is_string() && !crd["spec"]["names"]["plural"].get<string>().empty() ){
    //                     plural = crd["spec"]["names"]["plural"].get<string>();
    //                 }

    //                 if( crd["spec"].contains("versions") && crd["spec"]["versions"].is_array() ){

    //                     for( const auto& version_obj : crd["spec"]["versions"] ){

    //                         if( version_obj.contains("name") && version_obj["name"].is_string() && version_obj["name"].get<string>() == api_version.substr(api_version.find("/")+1) ){

    //                             version = version_obj["name"].get<string>();

    //                         }

    //                     }

    //                 }

    //             }

    //         }

    //     }

    //     cout << "The plural: " << plural << endl;
    //     cout << "The group: " << group << endl;
    //     cout << "The version: " << version << endl;
    //     cout << "The namespace: " << k8s_namespace << endl;
    //     cout << "The kind: " << kind << endl;
    //     cout << "The apiVersion: " << api_version << endl;


    //     /*

    //     // Creates a cluster scoped Custom object
    //     //
    //     object_t*
    //     CustomObjectsAPI_createClusterCustomObject(apiClient_t *apiClient, char *group, char *version, char *plural, object_t *body, char *pretty, char *dryRun, char *fieldManager, char *fieldValidation);


    //     // Creates a namespace scoped Custom object
    //     //
    //     object_t*
    //     CustomObjectsAPI_createNamespacedCustomObject(apiClient_t *apiClient, char *group, char *version, char *_namespace, char *plural, object_t *body, char *pretty, char *dryRun, char *fieldManager, char *fieldValidation);
        
    //     */

    //     if( k8s_namespace.empty() ){

    //         //cluster-scoped custom resource

    //         std::shared_ptr<object_t> custom_resource_type( object_parseFromJSON(custom_resource_cjson.get()), object_free );
    //         std::shared_ptr<object_t> created_custom_resource(
    //                                             CustomObjectsAPI_createClusterCustomObject(
    //                                                 const_cast<apiClient_t*>(api_client.get()), /* apiClient */
    //                                                 const_cast<char*>(group.c_str()),           /* group */
    //                                                 const_cast<char*>(version.c_str()),         /* version */
    //                                                 const_cast<char*>(plural.c_str()),          /* plural */
    //                                                 custom_resource_type.get(),                 /* body */
    //                                                 NULL, /* pretty */
    //                                                 NULL, /* dryRun */
    //                                                 NULL, /* fieldManager */
    //                                                 NULL  /* create options */
    //                                             ),
    //                                             object_free
    //                                         );
            
    //         if( created_custom_resource ){
                    
    //             cjson cjson_response(object_convertToJSON(created_custom_resource.get()));
    //             if( !cjson_response ){
    //                 return response;
    //             }
    //             response = cjson_response.toJson();

    //         } else {
    //             fmt::print("Cannot create a cluster-scoped custom resource.\n");
    //         }

    //     }else{

    //         //namespace-scoped custom resource
            
    //         std::shared_ptr<object_t> custom_resource_type( object_parseFromJSON(custom_resource_cjson.get()), object_free );
    //         std::shared_ptr<object_t> created_custom_resource(
    //                                             CustomObjectsAPI_createNamespacedCustomObject(
    //                                                 const_cast<apiClient_t*>(api_client.get()), /* apiClient */
    //                                                 const_cast<char*>(group.c_str()),           /* group */
    //                                                 const_cast<char*>(version.c_str()),         /* version */
    //                                                 const_cast<char*>(k8s_namespace.c_str()),   /* namespace */
    //                                                 const_cast<char*>(plural.c_str()),          /* plural */
    //                                                 custom_resource_type.get(),                 /* body */
    //                                                 NULL, /* pretty */
    //                                                 NULL, /* dryRun */
    //                                                 NULL, /* fieldManager */
    //                                                 NULL  /* create options */
    //                                             ),
    //                                             object_free
    //                                         );
            
    //         if( created_custom_resource ){
                    
    //             cjson cjson_response(object_convertToJSON(created_custom_resource.get()));
    //             if( !cjson_response ){
    //                 return response;
    //             }
    //             response = cjson_response.toJson();

    //         } else {
    //             fmt::print("Cannot create a cluster-scoped custom resource.\n");
    //         }

    //     }

    //     return response;

    // }



    // json KubernetesClient::deleteCustomResource( const json& custom_resource ) const{

    //     json response = json::object();

    //     //check to see if the name is specified
    //     if( !custom_resource.contains("metadata") || !custom_resource["metadata"].contains("name") || !custom_resource["metadata"]["name"].is_string() || custom_resource["metadata"]["name"].get<string>().empty() ){
    //         throw std::runtime_error("The custom resource must have a 'metadata.name' field that is a non-empty string.");
    //     }
    //     const string name = custom_resource["metadata"]["name"].get<string>();


    //     //check to see if the namespace is specified (optional for cluster-scoped resources)
    //     string k8s_namespace;
    //     if( custom_resource.contains("metadata") && custom_resource["metadata"].contains("namespace") && custom_resource["metadata"]["namespace"].is_string() && !custom_resource["metadata"]["namespace"].get<string>().empty() ){
    //         k8s_namespace = custom_resource["metadata"]["namespace"].get<string>();
    //     }

    //     //check to see if the kind is specified
    //     if( !custom_resource.contains("kind") || !custom_resource["kind"].is_string() || custom_resource["kind"].get<string>().empty() ){
    //         throw std::runtime_error("The custom resource must have a 'kind' field that is a non-empty string.");
    //     }
    //     const string kind = custom_resource["kind"].get<string>();

    //     //check to see if the apiVersion is specified
    //     if( !custom_resource.contains("apiVersion") || !custom_resource["apiVersion"].is_string() || custom_resource["apiVersion"].get<string>().empty() ){
    //         throw std::runtime_error("The custom resource must have a 'apiVersion' field that is a non-empty string.");
    //     }
    //     const string api_version = custom_resource["apiVersion"].get<string>();




    //     json all_crds = this->getCustomResourceDefinitions();

    //     // get the plural, group, and version from the custom resource definition (match with kind and api_version)
    //     // don't just use the first version, make sure they match
    //     string plural;
    //     string group;
    //     string version;



    //     /*

    //     CR:

    //             json cr = R"({
    //                 "apiVersion": "example.com/v1",
    //                 "kind": "ExampleResource",
    //                 "metadata": {
    //                     "name": "my-example-resource",
    //                     "namespace": "default"
    //                 },
    //                 "spec": {
    //                     "field1": "value1",
    //                     "field2": true
    //                 }
    //             })"_json;



    //     CRD: 
    //     "spec": {
    //         "conversion": {
    //             "strategy": "None"
    //         },
    //         "group": "example.com",
    //         "names": {
    //             "kind": "ExampleResource",
    //             "listKind": "ExampleResourceList",
    //             "plural": "exampleresources",
    //             "shortNames": [
    //                 "exr"
    //             ],
    //             "singular": "exampleresource"
    //         },
    //         "scope": "Namespaced",
    //         "versions": [
    //             {
    //                 "name": "v1",
    //                 "schema": {
    //                     "openAPIV3Schema": {
    //                         "type": "object"
    //                     }
    //                 },
    //                 "served": true,
    //                 "storage": true
    //             }
    //         ]
    //     },

        
    //     */

    //     for( const auto& crd : all_crds ){

    //         if( crd.contains("spec") && crd["spec"].contains("group") && crd["spec"]["group"].is_string() && crd["spec"]["group"].get<string>() == api_version.substr(0, api_version.find("/")) ){

    //             group = crd["spec"]["group"].get<string>();

    //             if( crd["spec"].contains("names") && crd["spec"]["names"].contains("kind") && crd["spec"]["names"]["kind"].is_string() && crd["spec"]["names"]["kind"].get<string>() == kind ){

    //                 if( crd["spec"]["names"].contains("plural") && crd["spec"]["names"]["plural"].is_string() && !crd["spec"]["names"]["plural"].get<string>().empty() ){
    //                     plural = crd["spec"]["names"]["plural"].get<string>();
    //                 }

    //                 if( crd["spec"].contains("versions") && crd["spec"]["versions"].is_array() ){

    //                     for( const auto& version_obj : crd["spec"]["versions"] ){

    //                         if( version_obj.contains("name") && version_obj["name"].is_string() && version_obj["name"].get<string>() == api_version.substr(api_version.find("/")+1) ){

    //                             version = version_obj["name"].get<string>();

    //                         }

    //                     }

    //                 }

    //             }

    //         }

    //     }

    //     cout << "The plural: " << plural << endl;
    //     cout << "The group: " << group << endl;
    //     cout << "The version: " << version << endl;
    //     cout << "The namespace: " << k8s_namespace << endl;
    //     cout << "The kind: " << kind << endl;
    //     cout << "The apiVersion: " << api_version << endl;
    //     cout << "The name: " << name << endl;






    //     if( k8s_namespace.empty() ){

    //         //cluster-scoped custom resource

    //         std::shared_ptr<object_t> deleted_custom_resource( 
    //                                             CustomObjectsAPI_deleteClusterCustomObject(
    //                                                 const_cast<apiClient_t*>(api_client.get()), /* apiClient */
    //                                                 const_cast<char*>(group.c_str()),           /* group */
    //                                                 const_cast<char*>(version.c_str()),     /* version */
    //                                                 const_cast<char*>(plural.c_str()),          /* plural */
    //                                                 const_cast<char*>(name.c_str()),            /* name */
    //                                                 NULL, /* gracePeriodSeconds */
    //                                                 NULL, /* orphanDependents */
    //                                                 NULL, /* propagationPolicy */
    //                                                 NULL, /* dryRun */
    //                                                 NULL  /* delete options */
    //                                             ),
    //                                             object_free
    //                                         );

    //         if( deleted_custom_resource ){
    //             cjson cjson_response( object_convertToJSON(deleted_custom_resource.get()) );
    //             if( !cjson_response ){
    //                 return response;
    //             }
    //             response = cjson_response.toJson();
    //         }

    //     }else{

    //         //namespace-scoped custom resource

    //         std::shared_ptr<object_t> deleted_custom_resource( 
    //                                             CustomObjectsAPI_deleteNamespacedCustomObject(
    //                                                 const_cast<apiClient_t*>(api_client.get()), /* apiClient */
    //                                                 const_cast<char*>(group.c_str()),           /* group */
    //                                                 const_cast<char*>(version.c_str()),     /* version */
    //                                                 const_cast<char*>(k8s_namespace.c_str()),   /* namespace */
    //                                                 const_cast<char*>(plural.c_str()),          /* plural */
    //                                                 const_cast<char*>(name.c_str()),            /* name */
    //                                                 NULL, /* gracePeriodSeconds */
    //                                                 NULL, /* orphanDependents */
    //                                                 NULL, /* propagationPolicy */
    //                                                 NULL, /* dryRun */
    //                                                 NULL  /* delete options */
    //                                             ),
    //                                             object_free
    //                                         );

    //         if( deleted_custom_resource ){
    //             cjson cjson_response( object_convertToJSON(deleted_custom_resource.get()) );
    //             if( !cjson_response ){
    //                 return response;
    //             }
    //             response = cjson_response.toJson();
    //         }

    //     }


    //     return response;

    // }




    // json KubernetesClient::createPod( const json& pod ) const{

    //     json response = json::object();

    //     cjson pod_cjson(pod);
    //     if( !pod_cjson ){
    //         fmt::print("Error before: [%s]\n", cJSON_GetErrorPtr());
    //         throw std::runtime_error("Cannot parse the pod JSON.");
    //     }
        
    //     //check to see if the namespace is specified
    //     if( !pod.contains("metadata") || !pod["metadata"].contains("namespace") || !pod["metadata"]["namespace"].is_string() || pod["metadata"]["namespace"].get<string>().empty() ){
    //         throw std::runtime_error("The pod must have a 'metadata.namespace' field that is a non-empty string.");
    //     }
    //     const string k8s_namespace = pod["metadata"]["namespace"].get<string>();


    //     std::shared_ptr<v1_pod_t> pod_type( v1_pod_parseFromJSON(pod_cjson.get()), v1_pod_free );
    //     std::shared_ptr<v1_pod_t> created_pod( 
    //                                         CoreV1API_createNamespacedPod(
    //                                             const_cast<apiClient_t*>(api_client.get()), 
    //                                             const_cast<char*>(k8s_namespace.c_str()),
    //                                             pod_type.get(),
    //                                             NULL, /* pretty */
    //                                             NULL, /* dryRun */
    //                                             NULL, /* fieldManager */
    //                                             NULL  /* create options */
    //                                         ),
    //                                         v1_pod_free
    //                                     );


    //     if( created_pod ){

    //         cjson cjson_response(v1_pod_convertToJSON(created_pod.get()));
    //         if( !cjson_response ){
    //             return response;
    //         }
    //         response = cjson_response.toJson();

    //     } else {
    //         fmt::print("Cannot create a pod.\n");
    //     }

    //     return response;
        
    // }


    // json KubernetesClient::deletePod( const json& pod ) const{

    //     json response = json::object();

    //     //check to see if the namespace is specified
    //     if( !pod.contains("metadata") || !pod["metadata"].contains("namespace") || !pod["metadata"]["namespace"].is_string() || pod["metadata"]["namespace"].get<string>().empty() ){
    //         throw std::runtime_error("The pod must have a 'metadata.namespace' field that is a non-empty string.");
    //     }
    //     const string k8s_namespace = pod["metadata"]["namespace"].get<string>();

    //     //check to see if the name is specified
    //     if( !pod.contains("metadata") || !pod["metadata"].contains("name") || !pod["metadata"]["name"].is_string() || pod["metadata"]["name"].get<string>().empty() ){
    //         throw std::runtime_error("The pod must have a 'metadata.name' field that is a non-empty string.");
    //     }
    //     const string pod_name = pod["metadata"]["name"].get<string>();


    //     std::shared_ptr<v1_pod_t> pod_object(
    //                                         CoreV1API_deleteNamespacedPod(
    //                                             const_cast<apiClient_t*>(api_client.get()), 
    //                                             const_cast<char*>(pod_name.c_str()),
    //                                             const_cast<char*>(k8s_namespace.c_str()),
    //                                             NULL, /* pretty */                                            
    //                                             NULL, /* dryRun */
    //                                             NULL, /* gracePeriodSeconds */
    //                                             NULL, /* orphanDependents */
    //                                             NULL, /* propagationPolicy */
    //                                             NULL /* delete options */
    //                                         ), 
    //                                         v1_pod_free 
    //                                     );

    //     if( pod_object ){

    //         cjson cjson_response(v1_pod_convertToJSON(pod_object.get()));
    //         if( !cjson_response ){
    //             return response;
    //         }

    //         response = cjson_response.toJson();

    //     }else{
    //         fmt::print("Cannot delete a pod.\n");
    //     }

    //     return response;

    // }



    json KubernetesClient::createGenericResource( const ResourceDescription& resource_description, const json& resource ) const{

        json response = json::object();

        const string resource_string = resource.dump();


        //check to see if the kind is specified
        if( !resource.contains("kind") || !resource["kind"].is_string() || resource["kind"].get<string>().empty() ){
            throw std::runtime_error("The resource must have a 'kind' field that is a non-empty string.");
        }

        //check to see if the apiVersion is specified
        if( !resource.contains("apiVersion") || !resource["apiVersion"].is_string() || resource["apiVersion"].get<string>().empty() ){
            throw std::runtime_error("The resource must have a 'apiVersion' field that is a non-empty string.");
        }


        auto generic_client = this->createGenericClient(resource_description);


        if( resource_description.k8s_namespace.empty() ){

            //cluster-scoped custom resource

            std::shared_ptr<char> api_response(
                                                Generic_createResource(
                                                    generic_client.get(), 
                                                    resource_string.c_str()
                                                ),
                                                free
                                            );

            if( api_response ){
                cjson cjson_response(api_response.get());
                if( cjson_response ){
                    response = cjson_response.toJson();
                }
                
            }

        }else{


            //namespace-scoped custom resource

            std::shared_ptr<char> api_response(
                                                Generic_createNamespacedResource(
                                                    generic_client.get(), 
                                                    resource_description.k8s_namespace.c_str(), 
                                                    resource_string.c_str()
                                                ),
                                                free
                                            );

            if( api_response ){
                cjson cjson_response(api_response.get());
                if( cjson_response ){
                    response = cjson_response.toJson();
                }                
            }

        }

        return response;

    }




    json KubernetesClient::deleteGenericResource( const ResourceDescription& resource_description, const json& resource ) const{

        json response = json::object();

        //check to see if the name is specified
        if( !resource.contains("metadata") || !resource["metadata"].contains("name") || !resource["metadata"]["name"].is_string() || resource["metadata"]["name"].get<string>().empty() ){
            throw std::runtime_error("The resource must have a 'metadata.name' field that is a non-empty string.");
        }


        auto generic_client = this->createGenericClient(resource_description);


        if( resource_description.k8s_namespace.empty() ){

            //cluster-scoped custom resource

            std::shared_ptr<char> api_response(
                                                Generic_deleteResource(
                                                    generic_client.get(), 
                                                    resource_description.name.c_str()
                                                ),
                                                free
                                            );

            if( api_response ){
                cjson cjson_response(api_response.get());
                if( cjson_response ){
                    response = cjson_response.toJson();
                }
                
            }

        }else{

            //namespace-scoped custom resource

            std::shared_ptr<char> api_response(
                                                Generic_deleteNamespacedResource(
                                                    generic_client.get(), 
                                                    resource_description.k8s_namespace.c_str(), 
                                                    resource_description.name.c_str()
                                                ),
                                                free
                                            );

            if( api_response ){

                cjson cjson_response(api_response.get());
                if( cjson_response ){
                    response = cjson_response.toJson();
                }

            }

        }

        return response;

    }




    json KubernetesClient::getGenericResource( const ResourceDescription& resource_description ) const{

        json response = json::object();

        auto generic_client = this->createGenericClient(resource_description);

        if( resource_description.k8s_namespace.empty() ){

            //cluster-scoped custom resource

            std::shared_ptr<char> api_response(
                                                Generic_readResource(
                                                    generic_client.get(), 
                                                    resource_description.name.c_str()
                                                ),
                                                free
                                            );

            if( api_response ){
                cjson cjson_response(api_response.get());
                if( cjson_response ){
                    response = cjson_response.toJson();
                }
                
            }

        }else{

            //namespace-scoped custom resource

            std::shared_ptr<char> api_response(
                                                Generic_readNamespacedResource(
                                                    generic_client.get(), 
                                                    resource_description.k8s_namespace.c_str(), 
                                                    resource_description.name.c_str()
                                                ),
                                                free
                                            );

            if( api_response ){

                cjson cjson_response(api_response.get());
                if( cjson_response ){
                    response = cjson_response.toJson();
                }

            }

        }

        return response;


    }




    json KubernetesClient::getGenericResources( const ResourceDescription& resource_description ) const{

        json response = json::array();

        auto generic_client = this->createGenericClient(resource_description);

        if( resource_description.k8s_namespace.empty() ){

            //cluster-scoped custom resource

            std::shared_ptr<char> api_response(
                                                Generic_list(
                                                    generic_client.get()
                                                ),
                                                free
                                            );

            if( api_response ){
                cjson cjson_response(api_response.get());
                if( cjson_response ){
                    response = cjson_response.toJson();
                }
                
            }

        }else{

            //namespace-scoped custom resource

            std::shared_ptr<char> api_response(
                                                Generic_listNamespaced(
                                                    generic_client.get(), 
                                                    resource_description.k8s_namespace.c_str()
                                                ),
                                                free
                                            );

            if( api_response ){

                cjson cjson_response(api_response.get());
                if( cjson_response ){
                    response = cjson_response.toJson();
                }

            }

        }

        return response;

    }




    json KubernetesClient::replaceGenericResource( const ResourceDescription& resource_description, const json& resource ) const{

        json response = json::object();


        //check to see if the kind is specified
        if( !resource.contains("kind") || !resource["kind"].is_string() || resource["kind"].get<string>().empty() ){
            throw std::runtime_error("The resource must have a 'kind' field that is a non-empty string.");
        }

        //check to see if the apiVersion is specified
        if( !resource.contains("apiVersion") || !resource["apiVersion"].is_string() || resource["apiVersion"].get<string>().empty() ){
            throw std::runtime_error("The resource must have a 'apiVersion' field that is a non-empty string.");
        }

        const string resource_string = resource.dump();



        auto generic_client = this->createGenericClient(resource_description);


        if( resource_description.k8s_namespace.empty() ){

            //cluster-scoped custom resource

            std::shared_ptr<char> api_response(
                                                Generic_replaceResource(
                                                    generic_client.get(),
                                                    resource_description.name.c_str(),
                                                    resource_string.c_str()
                                                ),
                                                free
                                            );

            if( api_response ){
                cjson cjson_response(api_response.get());
                if( cjson_response ){
                    response = cjson_response.toJson();
                }
                
            }

        }else{

            //namespace-scoped custom resource

            std::shared_ptr<char> api_response(
                                                Generic_replaceNamespacedResource(
                                                    generic_client.get(), 
                                                    resource_description.k8s_namespace.c_str(),
                                                    resource_description.name.c_str(),
                                                    resource_string.c_str()
                                                ),
                                                free
                                            );

            if( api_response ){

                cjson cjson_response(api_response.get());
                if( cjson_response ){
                    response = cjson_response.toJson();
                }

            }

        }

        return response;

    }



    json KubernetesClient::patchGenericResource( const ResourceDescription& resource_description, const json& patch ) const{

        json response = json::object();

        const string patch_string = patch.dump();


        auto generic_client = this->createGenericClient(resource_description);

        /*
        
        const char *patchBody = "[{\"op\": \"replace\", \"path\": \"/metadata/labels/foo\", \"value\": \"qux\" }]";
        list_t *contentType = list_createList();
        // Kubernetes supports multiple content types:
        list_addElement(contentType, "application/json-patch+json");
        // list_addElement(contentType, "application/merge-patch+json");
        // list_addElement(contentType, "application/strategic-merge-patch+json");
        // list_addElement(contentType, "application/apply-patch+yaml");
        list_freeList(contentType);
        free(patch);

        */


        const string patch_content_type = "application/json-patch+json";// "application/merge-patch+json";

        //create a list_t* for the content-type, wrapped in shared_ptr
        std::shared_ptr<list_t> patch_content_type_list( list_createList(), list_freeList );
        list_addElement(patch_content_type_list.get(), (void*)patch_content_type.c_str());
        

        if( resource_description.k8s_namespace.empty() ){

            //cluster-scoped custom resource

            std::shared_ptr<char> api_response(
                                                Generic_patchResource(
                                                    generic_client.get(), 
                                                    resource_description.name.c_str(),
                                                    patch_string.c_str(),
                                                    NULL, /* queryParameters */
                                                    NULL, /* headerParameters */
                                                    NULL, /* formParameters */
                                                    NULL, /* headerType */
                                                    const_cast<list_t*>(patch_content_type_list.get())  /* contentType */
                                                ),
                                                free
                                            );

            if( api_response ){
                cjson cjson_response(api_response.get());
                if( cjson_response ){
                    response = cjson_response.toJson();
                }                
            }


        }else{

            //namespace-scoped custom resource

            std::shared_ptr<char> api_response(
                                                Generic_patchNamespacedResource(
                                                    generic_client.get(), 
                                                    resource_description.k8s_namespace.c_str(),
                                                    resource_description.name.c_str(),
                                                    patch_string.c_str(),
                                                    NULL, /* queryParameters */
                                                    NULL, /* headerParameters */
                                                    NULL, /* formParameters */
                                                    NULL, /* headerType */
                                                    const_cast<list_t*>(patch_content_type_list.get())  /* contentType */
                                                ),
                                                free
                                            );

            if( api_response ){

                cjson cjson_response(api_response.get());
                if( cjson_response ){
                    response = cjson_response.toJson();
                }

            }

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
        
        const ResourceDescription resource_description(resource);

        response = this->createGenericResource(resource_description, resource);

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

        const ResourceDescription resource_description(resource);

        return this->deleteGenericResource(resource_description, resource);

    }
    

   

    std::shared_ptr<genericClient_t> KubernetesClient::createGenericClient( const ResourceDescription& resource_description ) const{

        std::shared_ptr<genericClient_t> generic_client( 
                            genericClient_create( 
                                api_client.get(), 
                                resource_description.api_group.c_str(),
                                resource_description.api_version.c_str(),
                                resource_description.kind_lower_plural.c_str()
                            ), 
                            genericClient_free 
                        );

        return generic_client;
        
    }



    json KubernetesClient::runQuery( const Query& query ) const{

        json results = json::array();

        for( const string& from : query.from ){
            
            json these_results = this->getGenericResources( from );

            results.push_back(these_results);

        }

        return results;

    }





}

