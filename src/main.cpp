#include "KubeppApp.h"
#include <CLI/CLI.hpp>
#include <iostream>


int main(int argc, char **argv) {

    // Create logger (defaults to stdout and log level of INFO)
    //auto logger = spdlog::stdout_color_mt("console");


    kubepp::KubeppApp kubepp_app;
    //std::cout << kubepp_app.run() << std::endl;

    CLI::App app{"Kubepp - Connecting to Kubernetes."};
    app.require_subcommand(1);


    // Events command

        CLI::App *events_app = app.add_subcommand("events", "Manage events.");

        // Watch
        //CLI::App *events_watch = events_app->add_subcommand("watch", "Watches events from the current Kubernetes cluster.");


    // Logs command
        CLI::App *logs_app = app.add_subcommand("logs", "Manage logs.");

    // Workloads command
        CLI::App *workloads_app = app.add_subcommand("workloads", "Manage workloads.");

    // Nodes command
        CLI::App *nodes_app = app.add_subcommand("nodes", "Manage nodes.");


    // CRDs command
        CLI::App *crds_app = app.add_subcommand("crds", "Manage custom resource definitions.");


    // parse the command line arguments

        CLI11_PARSE(app, argc, argv);

        // Events command

            if( *events_app ){
                kubepp_app.events_app.run();
            }


        // Logs command

            else if( *logs_app ){

                kubepp_app.logs_app.run();

            }

        // Workloads command

            else if( *workloads_app ){

                kubepp_app.workloads_app.run();

            }

        // Nodes command

            else if( *nodes_app ){

                kubepp_app.nodes_app.run();

            }

        // CRDs command

            else if( *crds_app ){

                kubepp_app.crds_app.run();

            }

    return 0;

}
