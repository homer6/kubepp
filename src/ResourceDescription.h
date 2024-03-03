#pragma once

#include <set>
using std::set;

#include <string>
using std::string;

#include <map>
using std::map;


#include "json_fwd.hpp"
using json = nlohmann::json;


namespace kubepp{


    class ResourceDescription{

        public:
            ResourceDescription( const json& resource );
            ResourceDescription( const string& resource_str );

            void fromJson( const json& resource );

            string api_group;
            string api_version;
            string api_group_version;
            string kind;
            string kind_lower_plural;
            string k8s_namespace;
            string name;


            static const map<string, string> kind_to_api_group;

        private:
            
            string toLower( const string& str ) const;
           
            


    };



}