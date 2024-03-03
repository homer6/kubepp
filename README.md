# Kubepp

A modern C++ wrapper around the official Kubernetes C client. Pronounced "kube-plus-plus."

https://github.com/homer6/kubepp

https://github.com/kubernetes-client/c

This project also contains a CLI application, kubepp, which is an alternative to kubectl. It's mainly used for testing the modern C++ Kubernetes client, but may develop into a higher-level CLI interface.



## Example (Client)

```c++
#include <kubepp/KubernetesClient.h>
using kubepp::KubernetesClient;

#include <kubepp/json.hpp>
using json = nlohmann::json;


KubernetesClient kube_client;

// get several resource types
    json workloads = kube_client.runQuery( "SELECT * FROM Pod, Deployment, stable.example.com/v1:CronTab" );
    cout << workloads.dump(4) << endl;


// get all resources
    json all_resources = kube_client.runQuery( "SELECT * FROM *" );
    cout << all_resources.dump(4) << endl;


// create, then delete a CustomResource
    json cr = R"({
        "apiVersion": "stable.example.com/v1",
        "kind": "CronTab",
        "metadata": {
            "name": "my-crontab",
            "namespace": "default"
        },
        "spec": {
            "cronSpec": "*/5 * * * *",
            "image": "my-cron-image:latest",
            "replicas": 3
        }
    })"_json;

    // supports one or many resources
    json cr_response = kube_client.createResources( cr );
    cout << cr_response.dump(4) << endl;

    json delete_cr_response = kube_client.deleteResources( cr );
    cout << delete_cr_response.dump(4) << endl;
```

## Example (CLI)

```bash

kubepp export resources > all_resources.json

kubepp export api > all_kinds.json

```



## Building

```bash
cmake .
make -j4
sudo make install
```

## Including with cmake

```cmake
include_directories(
    "/usr/local/include/kubernetes/include"
    "/usr/local/include/kubernetes/config"
    "/usr/local/include/kubernetes/api"
)

find_package(kubepp_lib REQUIRED)


# link to your binary
target_link_libraries(your-binary PRIVATE kubepp_lib::kubepp_lib)
```


## Running Tests

```bash
cmake --build build --target test
```


## Running

```bash
export KUBECONFIG="/etc/rancher/k3s/k3s.yaml"

kubepp logs

kubepp events watch

kubepp events --help

kubepp --help

kubectl api-resources
kubectl api-versions

kubepp export resources > all_resources.json
kubepp export api > all_kinds.json
```


## Debug


```bash
KUBECONFIG="/etc/rancher/k3s/k3s.yaml" sudo -E gdb --args ./build/kubepp logs
(gdb) set environment KUBECONFIG="/etc/rancher/k3s/k3s.yaml"
(gdb) run
```

