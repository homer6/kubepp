#pragma once

extern "C" {
    #include <apiClient.h>
}

#include <string>
using std::string;

#include <vector>
using std::vector;

#include <set>
using std::set;

#include <memory>

#include "json_fwd.hpp"
using json = nlohmann::json;


namespace kubepp{


    class KubernetesClient{

        public:
            KubernetesClient( const string& base_path_str = "https://10.0.0.157:6443" );
            ~KubernetesClient();

            void displayWorkloads() const;
            void displayEvents() const;
            void displayLogs() const;
            void displayNodes() const;
            
            json getPods( const vector<string>& k8s_namespaces = { "all" } ) const;
            json getDeployments( const vector<string>& k8s_namespaces = { "all" } ) const;
            json getEvents( const vector<string>& k8s_namespaces = { "all" } ) const;
            json getNodes() const;



            /* Creates many resources or one resource. Accepts and array or an object. Returns the json responses.*/
            json createResources( const json& resources ) const;
            
            /* Creates a single resource. Accepts an object. Returns the json response.*/
            json createResource( const json& resource ) const;
            

            
            /* Prefer using the createResources method instead, which will detect the type of resources automatically. */
            json createCustomResourceDefinition( const json& custom_resource_definition ) const;
            json createPod( const json& pod ) const;


            //doesn't work yet; needs debugging
            json getPodLogs( const string& k8s_namespace, const string& pod_name, const string& container ) const;

            vector<string> getNamespaces() const;
            set<string> resolveNamespaces( const vector<string>& k8s_namespaces = { "all" } ) const;



        protected:
            std::shared_ptr<apiClient_t> api_client;
            char* detected_base_path = NULL;
            string base_path;

            //todo: move to smart pointers
            sslConfig_t *sslConfig = nullptr;
            list_t *apiKeys = nullptr;

    };



}