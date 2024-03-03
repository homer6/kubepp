#include "Query.h"

#include <sstream>

#include "json.hpp"
using json = nlohmann::json;

namespace kubepp{


    Query::Query( const string& query_str ){

        this->parseQuery( query_str );

    }

    Query::Query( const char* query_str ){

        this->parseQuery( string(query_str) );

    }



    void Query::parseQuery( const string& query_str ){



        // parse the query string and populate the vectors
        // select, from, where, group_by, having, order_by, limit, offset

        this->clear();

        // split the query string into tokens
        vector<string> tokens;
        string token;
        std::istringstream token_stream(query_str);

        // split the string by space or comma
        while( std::getline(token_stream, token, ' ') ){

            // if has comma, split again
            if( token.find(",") != string::npos ){
                std::istringstream token_stream2(token);
                string token2;
                while( std::getline(token_stream2, token2, ',') ){
                    tokens.push_back(token2);
                }
                continue;
            }

            tokens.push_back(token);

        }




        // parse the tokens

        bool select_flag = false;
        bool from_flag = false;
        bool where_flag = false;
        bool group_by_flag = false;
        bool having_flag = false;
        bool order_by_flag = false;

        for( auto& t : tokens ){

            if( t == "SELECT" || t == "select" ){
                select_flag = true;
                continue;
            }
            if( t == "FROM" || t == "from" ){
                select_flag = false;
                from_flag = true;
                continue;
            }
            if( t == "WHERE" || t == "where" ){
                from_flag = false;
                where_flag = true;
                continue;
            }
            if( (t == "GROUP" || t == "group") && !group_by_flag ){
                where_flag = false;
                group_by_flag = true;
                continue;
            }
            if( t == "HAVING" || t == "having"){
                group_by_flag = false;
                having_flag = true;
                continue;
            }
            if( t == "ORDER" || t == "order" ){
                having_flag = false;
                order_by_flag = true;
                continue;
            }
            if( t == "LIMIT" || t == "limit" ){
                order_by_flag = false;
                this->limit.push_back( tokens.back() );
                continue;
            }
            if( t == "OFFSET" || t == "offset" ){
                this->offset.push_back( tokens.back() );
                continue;
            }

            if( select_flag ){
                this->select.push_back(t);
            }
            if( from_flag ){
                this->from.push_back(t);
            }
            if( where_flag ){
                this->where.push_back(t);
            }
            if( group_by_flag ){
                this->group_by.push_back(t);
            }
            if( having_flag ){
                this->having.push_back(t);
            }
            if( order_by_flag ){
                this->order_by.push_back(t);
            }

        }        



    }


    void Query::clear(){

        this->select.clear();
        this->from.clear();
        this->where.clear();
        this->group_by.clear();
        this->having.clear();
        this->order_by.clear();
        this->limit.clear();
        this->offset.clear();

    }



    string Query::asString() const{

        //put commas between the elements of the vectors; ensure that the last element does not have a comma after it

        string query_str = "SELECT ";
        query_str += this->implodeString(this->select, ", ");

        if( !this->from.empty() ){
            query_str += " FROM ";
            query_str += this->implodeString(this->from, " ");
        }

        if( !this->where.empty() ){
            query_str += " WHERE ";
            query_str += this->implodeString(this->where, " ");
        }

        if( !this->group_by.empty() ){
            query_str += " GROUP BY ";
            query_str += this->implodeString(this->group_by, ", ");
        }

        if( !this->having.empty() ){
            query_str += " HAVING ";
            query_str += this->implodeString(this->having, " ");
        }

        if( !this->order_by.empty() ){
            query_str += " ORDER BY ";
            query_str += this->implodeString(this->order_by, ", ");
        }

        if( !this->limit.empty() ){
            query_str += " LIMIT ";
            query_str += this->implodeString(this->order_by, ", ");
        }

        if( !this->offset.empty() ){
            query_str += " OFFSET ";
            query_str += this->implodeString(this->order_by, ", ");
        }

        return query_str + ";";

    }


    json Query::asJson() const{

        json query_json = json::object();

        query_json["select"] = this->select;
        query_json["from"] = this->from;
        query_json["where"] = this->where;
        query_json["group_by"] = this->group_by;
        query_json["having"] = this->having;
        query_json["order_by"] = this->order_by;
        query_json["limit"] = this->limit;
        query_json["offset"] = this->offset;

        return query_json;

    }


    string Query::implodeString( const vector<string>& vec, const string& delimiter ) const{

        string str = "";
        size_t last = vec.size() - 1;
        int i = 0;
        for( auto& v : vec ){
            if( i == last ){
                str += v;
                break;
            }
            str += v + delimiter;
        }
        return str;

    }


}