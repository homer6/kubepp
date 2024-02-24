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


namespace kubepp{


    class KubernetesClient{

        public:
            KubernetesClient( const string& base_path_str = "https://10.0.0.157:6443" );
            ~KubernetesClient();

            void run();
            void listPods( const vector<string>& k8s_namespaces = { "all" } ) const;
            void listDeployments( const vector<string>& k8s_namespaces = { "all" } ) const;

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