#pragma once


#include <string>
using std::string;

#include <vector>
using std::vector;

#include "json_fwd.hpp"
using json = nlohmann::json;


namespace kubepp{


    class Query{

        public:
            Query( const string& query_str );
            Query( const char* query_str );

            void parseQuery( const string& query_str );
            void clear();

            vector<string> select;
            vector<string> from;
            vector<string> where;
            vector<string> group_by;
            vector<string> having;
            vector<string> order_by;
            vector<string> limit;
            vector<string> offset;

            string asString() const;
            json asJson() const;


        protected:
            string implodeString( const vector<string>& vec, const string& delimiter ) const;

    };



}