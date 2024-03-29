#pragma once

extern "C" {
    #include <apiClient.h>
    #include <generic.h>
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


#include "ResourceDescription.h"
#include "Query.h"


namespace kubepp{


    class KubernetesClient{

        public:
            KubernetesClient( const string& base_path_str = "https://10.0.0.157:6443" );
            ~KubernetesClient();

            /* Creates many resources or one resource. Accepts and array or an object. Returns the json responses.*/
            json createResources( const json& resources ) const;

            /* Deletes many resources or one resource. Accepts and array or an object. Returns the json responses.*/
            json deleteResources( const json& resources ) const;


            json runQuery( const Query& query ) const;


            json createGenericResource( const ResourceDescription& resource_description, const json& resource ) const;
            json deleteGenericResource( const ResourceDescription& resource_description, const json& resource ) const;
            json getGenericResource( const ResourceDescription& resource_description ) const;
            json getGenericResources( const ResourceDescription& resource_description ) const;

            //doesn't work yet; needs this fix applied in the c client:
            json replaceGenericResource( const ResourceDescription& resource_description, const json& resource ) const;

            //works
            json patchGenericResource( const ResourceDescription& resource_description, const json& patch ) const;


            //doesn't work yet; needs this fix applied in the c client: https://github.com/kubernetes-client/c/issues/222
            //this method works if the fix is applied to the c client
            json getPodLogs( const string& k8s_namespace, const string& pod_name, const string& container ) const;

            vector<string> getNamespaceNames() const;
            set<string> resolveNamespaces( const vector<string>& k8s_namespaces = { "all" } ) const;


            json getApiResources() const;


        protected:

            /* Creates a single resource. Accepts an object. Returns the json response. Prefer using createResources instead of this method.*/
            json createResource( const json& resource ) const;

            /* Deletes a single resource. Accepts an object. Returns the json response. Prefer using deleteResources instead of this method.*/
            json deleteResource( const json& resource ) const;
            
            std::shared_ptr<genericClient_t> createGenericClient( const ResourceDescription& resource_description ) const;


            std::shared_ptr<apiClient_t> api_client;
            char* detected_base_path = NULL;
            string base_path;

            //todo: move to smart pointers
            sslConfig_t *sslConfig = nullptr;
            list_t *apiKeys = nullptr;








    };



}