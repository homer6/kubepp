#pragma once

extern "C" {
    #include <apiClient.h>
}



namespace kubepp{


    class KubernetesClient{

        public:
            KubernetesClient();
            ~KubernetesClient();

            void run();
            void listPod();


        protected:
            apiClient_t *apiClient = nullptr;
            char *basePath = nullptr;
            sslConfig_t *sslConfig = nullptr;
            list_t *apiKeys = nullptr;

    };



}