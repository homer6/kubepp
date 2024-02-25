#pragma once

extern "C" {
    #include <kube_config.h>
    #include <apiClient.h>
}

#include <string>
using std::string;

#include <memory>

#include "json_fwd.hpp"
using json = nlohmann::json;



namespace kubepp {

    //This class is a modern c++ wrapper for cJSON that can cast to and from nhlohmann::json
    //It uses a smartptr to manage the cJSON object with a custom deleter

    class cjson{

        public:
            cjson();

            //takes ownership of the cJSON object
            cjson( cJSON* cjson_ptr );


            cjson( const cjson& cjson );
            cjson( cjson&& cjson );
            cjson( const string& json_str );
            cjson( const char* json_str );
            cjson( const json& json_obj );
            ~cjson();

            cjson& operator=( const cjson& cjson );
            cjson& operator=( cjson&& cjson );
            cjson& operator=( const string& json_str );
            cjson& operator=( const char* json_str );
            cjson& operator=( const json& json_obj );

            cJSON* get() const;

            operator json() const;
            operator string() const;
            operator bool() const;

            json toJson() const;

        private:
            std::shared_ptr<cJSON> cjson_ptr;

    };


}