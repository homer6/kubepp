#include "ResourceDescription.h"

#include "json.hpp"
using json = nlohmann::json;

#include <map>
#include <string>


namespace kubepp{


    const map<string,string> ResourceDescription::kindToApiGroup{
        {"Binding", "v1"},
        {"ComponentStatus", "v1"},
        {"ConfigMap", "v1"},
        {"Endpoints", "v1"},
        {"Event", "v1"},
        {"LimitRange", "v1"},
        {"Namespace", "v1"},
        {"Node", "v1"},
        {"PersistentVolumeClaim", "v1"},
        {"PersistentVolume", "v1"},
        {"Pod", "v1"},
        {"PodTemplate", "v1"},
        {"ReplicationController", "v1"},
        {"ResourceQuota", "v1"},
        {"Secret", "v1"},
        {"ServiceAccount", "v1"},
        {"Service", "v1"},
        {"MutatingWebhookConfiguration", "admissionregistration.k8s.io/v1"},
        {"ValidatingWebhookConfiguration", "admissionregistration.k8s.io/v1"},
        {"CustomResourceDefinition", "apiextensions.k8s.io/v1"},
        {"APIService", "apiregistration.k8s.io/v1"},
        {"ControllerRevision", "apps/v1"},
        {"DaemonSet", "apps/v1"},
        {"Deployment", "apps/v1"},
        {"ReplicaSet", "apps/v1"},
        {"StatefulSet", "apps/v1"},
        {"SelfSubjectReview", "authentication.k8s.io/v1"},
        {"TokenReview", "authentication.k8s.io/v1"},
        {"LocalSubjectAccessReview", "authorization.k8s.io/v1"},
        {"SelfSubjectAccessReview", "authorization.k8s.io/v1"},
        {"SelfSubjectRulesReview", "authorization.k8s.io/v1"},
        {"SubjectAccessReview", "authorization.k8s.io/v1"},
        {"HorizontalPodAutoscaler", "autoscaling/v2"},
        {"CronJob", "batch/v1"},
        {"Job", "batch/v1"},
        {"CertificateSigningRequest", "certificates.k8s.io/v1"},
        {"Lease", "coordination.k8s.io/v1"},
        {"EndpointSlice", "discovery.k8s.io/v1"},
        {"Event", "events.k8s.io/v1"},
        {"FlowSchema", "flowcontrol.apiserver.k8s.io/v1beta3"},
        {"PriorityLevelConfiguration", "flowcontrol.apiserver.k8s.io/v1beta3"},
        {"IngressClass", "networking.k8s.io/v1"},
        {"Ingress", "networking.k8s.io/v1"},
        {"NetworkPolicy", "networking.k8s.io/v1"},
        {"RuntimeClass", "node.k8s.io/v1"},
        {"PodDisruptionBudget", "policy/v1"},
        {"ClusterRoleBinding", "rbac.authorization.k8s.io/v1"},
        {"ClusterRole", "rbac.authorization.k8s.io/v1"},
        {"RoleBinding", "rbac.authorization.k8s.io/v1"},
        {"Role", "rbac.authorization.k8s.io/v1"},
        {"PriorityClass", "scheduling.k8s.io/v1"},
        {"CronTab", "stable.example.com/v1"},
        {"CSIDriver", "storage.k8s.io/v1"},
        {"CSINode", "storage.k8s.io/v1"},
        {"CSIStorageCapacity", "storage.k8s.io/v1"},
        {"StorageClass", "storage.k8s.io/v1"},
        {"VolumeAttachment", "storage.k8s.io/v1"}
    };





    ResourceDescription::ResourceDescription( const json& resource ){

        this->fromJson(resource);

    }


    void ResourceDescription::fromJson( const json& resource ){

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

        if( !ResourceDescription::kindToApiGroup.contains(this->kind) ){
            const string looked_up_api_group = ResourceDescription::kindToApiGroup.at(this->kind);
            if( looked_up_api_group == "v1" ){
                // the core api resources have an empty api_group
                this->api_group = "";
            }
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



    ResourceDescription::ResourceDescription( const string& resource_str ){

        
        if( resource_str.empty() ){
            throw std::runtime_error("The resource string must be a non-empty string.");
        }

        json resource = json::object();

        if( ResourceDescription::kindToApiGroup.count(resource_str) ){

            //search by Kind only

            const string looked_up_api_group = ResourceDescription::kindToApiGroup.at(resource_str);
            
            resource["apiVersion"] = looked_up_api_group;
            resource["kind"] = resource_str;

            this->fromJson(resource);

        }else{

            //search by apiVersion:Kind format
            //split the string by the colon

            const size_t colon_pos = resource_str.find(":");
            if( colon_pos == string::npos ){
                throw std::runtime_error("The resource is not found. Resource string must be in the format 'apiVersion:Kind' or 'Kind'. The resource string is: " + resource_str + ".");
            }

            const string api_group_version = resource_str.substr(0, colon_pos);
            const string kind = resource_str.substr(colon_pos+1);

            resource["apiVersion"] = api_group_version;
            resource["kind"] = kind;
            
            this->fromJson(resource);

        }



    }


}