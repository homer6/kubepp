#pragma once

#include <set>
using std::set;

#include <string>
using std::string;

#include "json_fwd.hpp"
using json = nlohmann::json;


namespace kubepp{


    class ResourceDescription{

        public:
            ResourceDescription( const json& resource );

            string api_group;
            string api_version;
            string api_group_version;
            string kind;
            string kind_lower_plural;
            string k8s_namespace;
            string name;


        private:
            
            string toLower( const string& str ) const;


            // make a single instance of core_resources in static memory
            static const std::set<string> core_resources;


    };



}