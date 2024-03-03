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

            if( from == "*" ){

                for( const auto& pair : ResourceDescription::kind_to_api_group ){
 
                    json these_results = this->getGenericResources( pair.first );

                    ResourceDescription resource_description( pair.first );

                    if( these_results.contains("items") && these_results["items"].is_array() ){
                        for( json result : these_results["items"] ){
                            result["apiVersion"] = resource_description.api_group_version;
                            result["kind"] = resource_description.kind;
                            results.push_back(result);
                        }
                    }

                }

            }else{

                json these_results = this->getGenericResources( from );

                ResourceDescription resource_description(from);

                if( these_results.contains("items") && these_results["items"].is_array() ){
                    for( json result : these_results["items"] ){
                        result["apiVersion"] = resource_description.api_group_version;
                        result["kind"] = resource_description.kind;
                        results.push_back(result);
                    }
                }

            }
            

        }

        return results;

    }





}

