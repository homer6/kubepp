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
            

            /* Deletes many resources or one resource. Accepts and array or an object. Returns the json responses.*/
            json deleteResources( const json& resources ) const;

            /* Deletes a single resource. Accepts an object. Returns the json response.*/
            json deleteResource( const json& resource ) const;


            
            /* Prefer using the createResources method instead, which will detect the type of resources automatically. */
            json getCustomResourceDefinition( const string& name ) const;
            json createCustomResourceDefinition( const json& custom_resource_definition ) const;
            json deleteCustomResourceDefinition( const json& custom_resource_definition ) const;
            json getCustomResourceDefinitions() const;

            //this works for both namespaced-scoped and cluster-scoped custom resources
            //resource without a namespace are treated as cluster-scoped
            json createCustomResource( const json& custom_resource ) const;
            json deleteCustomResource( const json& custom_resource ) const;

            json createPod( const json& pod ) const;
            json deletePod( const json& pod ) const;



            json createGenericResource( const string& group, const string& version, const string& plural, const json& resource ) const;
            json deleteGenericResource( const string& group, const string& version, const string& plural, const json& resource ) const;
            json getGenericResource( const string& group, const string& version, const string& plural, const string& name, const string k8s_namespace = "" ) const;
            json getGenericResources( const string& group, const string& version, const string& plural, const string k8s_namespace = "" ) const;
            //doesn't work yet; needs this fix applied in the c client:
            json replaceGenericResource( const string& group, const string& version, const string& plural, const json& resource ) const;
            json patchGenericResource( const string& group, const string& version, const string& plural, const string& name, const json& patch, const string k8s_namespace = "" ) const;


            //doesn't work yet; needs this fix applied in the c client: https://github.com/kubernetes-client/c/issues/222
            //this method works if the fix is applied to the c client
            json getPodLogs( const string& k8s_namespace, const string& pod_name, const string& container ) const;

            vector<string> getNamespaceNames() const;
            set<string> resolveNamespaces( const vector<string>& k8s_namespaces = { "all" } ) const;


            string toLower( const string& str ) const;



        protected:
            std::shared_ptr<apiClient_t> api_client;
            char* detected_base_path = NULL;
            string base_path;

            //todo: move to smart pointers
            sslConfig_t *sslConfig = nullptr;
            list_t *apiKeys = nullptr;

            const set<string> core_resources = {"Pod", "Service", "ReplicationController", "Deployment", "StatefulSet", "DaemonSet", "Job", "CronJob", "Namespace", "ConfigMap", "Secret", "PersistentVolume", "PersistentVolumeClaim", "StorageClass", "ServiceAccount", "Role", "ClusterRole", "RoleBinding", "ClusterRoleBinding"};  

    };



}