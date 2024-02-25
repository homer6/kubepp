#include "cjson.h"

#include "json.hpp"
using json = nlohmann::json;



namespace kubepp{

    cjson::cjson(){
        cjson_ptr = std::shared_ptr<cJSON>( cJSON_CreateObject(), cJSON_Delete );
    }

    cjson::cjson( cJSON* cjson_ptr ){
        this->cjson_ptr = std::shared_ptr<cJSON>( cjson_ptr, cJSON_Delete );        
    }

    cjson::cjson( const cjson& cjson ){
        this->cjson_ptr = cjson.cjson_ptr;
    }

    cjson::cjson( cjson&& cjson ){
        this->cjson_ptr = std::move( cjson.cjson_ptr );
    }

    cjson::cjson( const std::string& json_str ){        
        this->cjson_ptr = std::shared_ptr<cJSON>( cJSON_Parse( json_str.c_str() ), cJSON_Delete );
    }

    cjson::cjson( const char* json_str ){
        this->cjson_ptr = std::shared_ptr<cJSON>( cJSON_Parse( json_str ), cJSON_Delete );
    }

    cjson::cjson( const json& json_obj ){
        const string json_str = json_obj.dump();
        this->cjson_ptr = std::shared_ptr<cJSON>( cJSON_Parse( json_str.c_str() ), cJSON_Delete );
    }

    cjson::~cjson(){
        this->cjson_ptr.reset();
    }

    cjson& cjson::operator=( const cjson& cjson ){
        this->cjson_ptr = cjson.cjson_ptr;
        return *this;
    }

    cjson& cjson::operator=( cjson&& cjson ){
        this->cjson_ptr = std::move( cjson.cjson_ptr );
        return *this;
    }

    cjson& cjson::operator=( const std::string& json_str ){
        this->cjson_ptr = std::shared_ptr<cJSON>( cJSON_Parse( json_str.c_str() ), cJSON_Delete );
        return *this;
    }

    cjson& cjson::operator=( const char* json_str ){
        this->cjson_ptr = std::shared_ptr<cJSON>( cJSON_Parse( json_str ), cJSON_Delete );
        return *this;
    }

    cjson& cjson::operator=( const json& json_obj ){
        const string json_str = json_obj.dump();
        this->cjson_ptr = std::shared_ptr<cJSON>( cJSON_Parse( json_str.c_str() ), cJSON_Delete );
        return *this;
    }

    cJSON* cjson::get() const{
        return this->cjson_ptr.get();
    }

    cjson::operator json() const{
        if( !this->cjson_ptr.get() ){
            return json();
        }
        std::shared_ptr<char> json_string_ptr( cJSON_PrintUnformatted( this->cjson_ptr.get() ), cJSON_free );
        return json::parse( json_string_ptr.get() );
    }

    cjson::operator std::string() const{
        if( !this->cjson_ptr.get() ){
            return string();
        }
        std::shared_ptr<char> json_string_ptr( cJSON_PrintUnformatted( this->cjson_ptr.get() ), cJSON_free );
        return string( json_string_ptr.get() );
    }

    cjson::operator bool() const{
        return this->cjson_ptr.get() != NULL;
    }

    json cjson::toJson() const{
        if( !this->cjson_ptr.get() ){
            return json();
        }            
        return *this;
    }

}