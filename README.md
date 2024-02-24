# Kubepp

A modern C++ wrapper around the official Kubernetes C client.

https://github.com/homer6/kubepp

https://github.com/kubernetes-client/c

This project also contains a CLI application, kubepp, which is an alternative to kubectl. It's mainly used for testing the modern C++ Kubernetes client, but may develop into a higher-level CLI interface.


## Building

```
cmake -S . -B build
cmake --build build --target all
```

## Running Tests

```
cmake --build build --target test
```


## Running

```
./build/kubepp logs

./build/kubepp events watch

./build/kubepp events --help
```
