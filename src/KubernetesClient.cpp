#include "KubernetesClient.h"

extern "C" {
    #include <kube_config.h>
    #include <apiClient.h>
    #include <CoreV1API.h>
}

#include <stdexcept>
#include <fmt/core.h>

#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"   // support for loading levels from the environment variable
#include "spdlog/fmt/ostr.h"  // support for user defined types



namespace kubepp{

    KubernetesClient::KubernetesClient(){

        int rc = load_kube_config(&basePath, &sslConfig, &apiKeys, NULL);
        if (rc != 0) {
            throw std::runtime_error("Cannot load kubernetes configuration.");
        }

        apiClient = apiClient_create_with_base_path(basePath, sslConfig, apiKeys);
        //apiClient = apiClient_create();
        if (!apiClient) {
            throw std::runtime_error("Cannot create a kubernetes client.");
        }

    }

    KubernetesClient::~KubernetesClient(){

        apiClient_free(apiClient);
        apiClient = nullptr;
        free_client_config(basePath, sslConfig, apiKeys);
        basePath = nullptr;
        sslConfig = nullptr;
        apiKeys = nullptr;
        apiClient_unsetupGlobalEnv();

    }

    void KubernetesClient::run(){

        spdlog::info( "Hello, kube world! (client)" );
        this->listPod();

    }



    void KubernetesClient::listPod(){

        v1_pod_list_t *pod_list = NULL;
        char *k8s_namespace = "kube-system";
        pod_list = CoreV1API_listNamespacedPod(apiClient, 
                                            k8s_namespace,   /*namespace */
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

        fmt::print("The return code of HTTP request={}\n", apiClient->response_code);

        if( pod_list ){

            fmt::print("Get pod list:\n");

            listEntry_t *listEntry = NULL;
            v1_pod_t *pod = NULL;
            list_ForEach(listEntry, pod_list->items) {
                pod = (v1_pod_t *)listEntry->data;
                fmt::print("\tThe pod name: {}\n", pod->metadata->name);
            }
            v1_pod_list_free(pod_list);
            pod_list = NULL;

        }else{

            fmt::print("Cannot get any pod.\n");

        }

    }

}