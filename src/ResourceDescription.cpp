#include "ResourceDescription.h"

#include "json.hpp"
using json = nlohmann::json;



namespace kubepp{


    // make a single instance of core_resources in static memory

    const set<string> ResourceDescription::core_resources = {"Pod", "Service", "ReplicationController", "Deployment", "StatefulSet", "DaemonSet", "Job", "CronJob", "Namespace", "ConfigMap", "Secret", "PersistentVolume", "PersistentVolumeClaim", "StorageClass", "ServiceAccount", "Role", "ClusterRole", "RoleBinding", "ClusterRoleBinding"};


    ResourceDescription::ResourceDescription( const json& resource ){

        if( !resource.is_object() ){
            throw std::runtime_error("The resource must be a JSON object.");
        }

        // ensure that the resource has a 'apiVersion' field
        if( !resource.contains("apiVersion") || !resource["apiVersion"].is_string() || resource["apiVersion"].get<string>().empty() ){
            throw std::runtime_error("The resource must have a 'apiVersion' field that is a non-empty string.");
        }
        this->api_group_version = resource["apiVersion"].get<string>();


        // ensure that the resource has a 'kind' field
        if( !resource.contains("kind") || !resource["kind"].is_string() || resource["kind"].get<string>().empty() ){
            throw std::runtime_error("The resource must have a 'kind' field that is a non-empty string.");
        }
        this->kind = resource["kind"].get<string>();
        this->kind_lower_plural = this->toLower(kind) + "s";
 
        this->api_group = this->api_group_version.substr(0, this->api_group_version.find("/"));
        this->api_version = this->api_group_version.substr(this->api_group_version.find("/")+1);

        if( ResourceDescription::core_resources.contains(kind) ){
            // the core api resources have an empty api_group
            this->api_group = "";
        }

        // get the name and namespace
        if( resource.contains("metadata") && resource["metadata"].is_object() ){
            if( resource["metadata"].contains("namespace") && resource["metadata"]["namespace"].is_string() && !resource["metadata"]["namespace"].get<string>().empty() ){
                this->k8s_namespace = resource["metadata"]["namespace"].get<string>();
            }
            if( resource["metadata"].contains("name") && resource["metadata"]["name"].is_string() && !resource["metadata"]["name"].get<string>().empty() ){
                this->name = resource["metadata"]["name"].get<string>();
            }
        }

    }


    string ResourceDescription::toLower( const string& str ) const{
        string lower_str = str;
        std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);
        return lower_str;
    }


}