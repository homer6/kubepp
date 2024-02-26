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
        CLI::App *crds_create_app = crds_app->add_subcommand("create", "Creates a sample CRD.");
        CLI::App *crds_delete_app = crds_app->add_subcommand("delete", "Deletes the a sample CRD.");
        CLI::App *crds_list_app = crds_app->add_subcommand("list", "Lists the CRDs.");

    // CRs command
        CLI::App *crs_app = app.add_subcommand("crs", "Manage custom resources.");
        CLI::App *crs_create_app = crs_app->add_subcommand("create", "Creates a sample CR.");
        CLI::App *crs_delete_app = crs_app->add_subcommand("delete", "Deletes the a sample CR.");

    // Pods command
        CLI::App *pod_app = app.add_subcommand("pods", "Manage pods.");
        CLI::App *pod_create_app = pod_app->add_subcommand("create", "Creates a sample pod.");
        CLI::App *pod_delete_app = pod_app->add_subcommand("delete", "Deletes the a sample pod.");


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

            else if( *crds_create_app ){

                kubepp_app.crds_app.createSample();

            }else if( *crds_delete_app ){

                kubepp_app.crds_app.deleteSample();

            }else if( *crds_list_app ){

                kubepp_app.crds_app.displayCustomResourceDefinitions();

            }

        // CRs command

            else if( *crs_create_app ){

                kubepp_app.crs_app.createSample();

            }else if( *crs_delete_app ){

                kubepp_app.crs_app.deleteSample();

            }

        // Pod command

            else if( *pod_create_app ){

                kubepp_app.pod_app.createSample();

            }else if( *pod_delete_app ){

                kubepp_app.pod_app.deleteSample();

            }

    return 0;

}
