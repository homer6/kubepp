#pragma once

#include <string>
using std::string;

#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"   // support for loading levels from the environment variable
#include "spdlog/fmt/ostr.h"  // support for user defined types

#include "apps/EventsApp.h"
#include "apps/LogsApp.h"
#include "apps/WorkloadsApp.h"
#include "apps/NodesApp.h"
#include "apps/CustomResourceDefinitionsApp.h"
#include "apps/CustomResourceApp.h"
#include "apps/PodApp.h"
#include "apps/ExportApp.h"



namespace kubepp {
    
    class KubeppApp {

        public:

            string run(){

                spdlog::info( "KubeppApp is running!" );
                return "KubeppApp is running!";
                
            }

        public:
            apps::EventsApp events_app;
            apps::LogsApp logs_app;
            apps::WorkloadsApp workloads_app;
            apps::NodesApp nodes_app;
            apps::CustomResourceDefinitionsApp crds_app;
            apps::CustomResourceApp crs_app;
            apps::PodApp pod_app;
            apps::ExportApp export_app;
            
    };

}
