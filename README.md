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



## Building (Ubuntu)

```bash
sudo apt install libspdlog-dev libfmt-dev
cmake .
make -j4
sudo make install
```

## Building (MacOS)

```bash
git clone https://github.com/kubernetes-client/c
export CLIENT_REPO_ROOT=${PWD}/c

brew install openssl curl libwebsockets uncrustify spdlog

git clone https://github.com/yaml/libyaml
cd libyaml
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DBUILD_TESTING=OFF -DBUILD_SHARED_LIBS=ON ..
make
sudo make install
cd ../..


cd ${CLIENT_REPO_ROOT}/kubernetes
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
make -j8
sudo make -j8 install

cd ../../..
git clone https://github.com/homer6/kubepp.git
cd kubepp
cmake .
sudo make -j8 install
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

## Sponsorship

Kubepp's development is sponsored by https://github.com/homer6/kubify