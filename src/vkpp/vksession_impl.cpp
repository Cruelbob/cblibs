#include <vksession_impl.hpp>

#include <cstdarg>
#include <regex>
#include <ostream>
#include <fstream>

#include <curl\curl.h>
#include <md5.hpp>

static char errorBuffer[CURL_ERROR_SIZE];
int dummy_writer(char *data, size_t size, size_t nmemb, std::string *buffer) {
    return size * nmemb;
}
int writer(char *data, size_t size, size_t nmemb, std::string *buffer) {
    size_t result = size * nmemb;
    if(buffer) {
        buffer->append(data,result);
    }
    return result;
}
int stream_writer(char *data, size_t size, size_t nmemb, std::ostream *ostr)
{
    size_t result = size * nmemb;
    if(ostr) {
        ostr->write(data,result);
    }
    return result;
}

namespace cb {
    namespace vkpp {
        vksession::vksession_impl::vksession_impl(const std::string& login,const std::string& password,const std::string& api_key):
          curl(curl_easy_init(),curl_easy_cleanup) {
            info_.api_key = api_key;
            if(curl) {
                curl_easy_setopt(curl.get(), CURLOPT_ERRORBUFFER, errorBuffer);
                curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION,1);
                curl_easy_setopt(curl.get(), CURLOPT_COOKIEFILE, "");
                curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, dummy_writer);
                if(auth_user(login,password)) {
                    if(auth_api(api_key)) {
                        return;
                    }
                    else {
                        throw cb::error::error(err_codes::APP_AUTH_ERROR,"Error while app authorization.");
                    }
                }
                else {
                    throw cb::error::error(err_codes::USER_AUTH_ERROR,"Error while user authorization.");
                }
            }
            else {
                throw cb::error::error(err_codes::CURL_INIT_ERROR,"Error while initialize curl.");
            }
        }
        vksession::vksession_impl::~vksession_impl() {
        }
        bool vksession::vksession_impl::auth_user(const std::string& login,const std::string& pass) {
            std::string params = "vk.com/login.php?email="+login+"&pass="+pass;
            curl_easy_setopt(curl.get(), CURLOPT_URL,params.c_str());
            CURLcode res;
            res = curl_easy_perform(curl.get());
            if(res == CURLE_OK) {
                curl_slist *cookies = nullptr;
                res = curl_easy_getinfo(curl.get(), CURLINFO_COOKIELIST, &cookies);
                if(res == CURLE_OK) {
                    std::string cookie;
                    for(curl_slist *nc = cookies;nc != nullptr;nc = nc->next) {
                        cookie = nc->data;
                        if(cookie.find("remixsid") != std::string::npos) {
                            info_.remixsid = cookie.substr(cookie.rfind('\t')+1);
                            curl_slist_free_all(cookies);
                            return true;
                        }
                    }
                    curl_slist_free_all(cookies);
                }
            }
            return false;
        }
        bool vksession::vksession_impl::auth_api(const std::string& api_key) {
            char buf[100];
            _itoa_s(16383,buf,10);
            std::string params = "vk.com/login.php?app="+api_key+"&layout=popup&type=browser&settings="+buf;
            curl_easy_setopt(curl.get(), CURLOPT_URL, params.c_str());

            std::string buffer;
            curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, writer);
            curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &buffer);
           
            CURLcode res = curl_easy_perform(curl.get());
            curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, dummy_writer);
            if(res == CURLE_OK) {
                char *s = curl_unescape(buffer.c_str(),buffer.length());
                buffer = s;
                curl_free(s);
                if(buffer.find("Login success") != std::string::npos) {
                    res = curl_easy_getinfo(curl.get(),CURLINFO_EFFECTIVE_URL,&s);
                    if(res == CURLE_OK) {
                        s = curl_unescape(s,strlen(s));
                        std::string session = s;
                        session = session.substr(session.find('=')+1);
                        curl_free(s);
                        std::regex rex("\\{\"mid\":(\\d+),\"sid\":\"(\\w+)\",\"secret\":\"(\\w+)\",\"expire\":(\\d+),\"sig\":\"(\\w+)\"\\}");
                        std::smatch results;
                        if(std::regex_match(session,results,rex)) {
                            info_.mid = results[1].str().c_str();
                            info_.sid = results[2];
                            info_.secret = results[3];
                            info_.expire = results[4].str().c_str();
                            info_.sig = results[5];
                            return true;
                        }
                    }
                }
                std::string auth_hash,
                            app_settings_hash;
                std::regex rex("^var auth_hash = '(.+)';$");
                std::smatch results;
                if(std::regex_search(buffer,results,rex)) {
                    auth_hash = results[1];
                    rex = "^var app_settings_hash = '(.+)';$";
                    if(std::regex_search(buffer,results,rex)) {
                        app_settings_hash = results[1];
                        std::cout << auth_hash << std::endl << app_settings_hash << std::endl;
                        params = "vk.com/apps.php?act=a_save_settings&addMember=1";
                        params += "&app_settings_2=1";
                        params += "&app_settings_4=1";
                        params += "&app_settings_8=1";
                        params += "&app_settings_16=1";
                        params += "&app_settings_32=1";
                        params += "&app_settings_64=1";
                        params += "&app_settings_128=1";
                        params += "&app_settings_256=1";
                        params += "&app_settings_512=1";
                        params += "&app_settings_1024=1";
                        params += "&app_settings_2048=1";
                        params += "&app_settings_4096=1";
                        params += "&app_settings_8192=1";
                        params += "&hash="+app_settings_hash;
                        params += "&id="+api_key;
                        curl_easy_setopt(curl.get(), CURLOPT_URL, params.c_str());
                        res = curl_easy_perform(curl.get());
                        if(res == CURLE_OK) {
                            params = "vk.com/login.php?act=a_auth&app="+api_key+"&hash="+auth_hash+"&permanent=1";
                            curl_easy_setopt(curl.get(), CURLOPT_URL, params.c_str());
                            buffer.clear();
                            curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, writer);
                            curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &buffer);
                            res = curl_easy_perform(curl.get());
                            curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, dummy_writer);
                            
                            if(res == CURLE_OK) {
                                rex = "\\{\"mid\":(\\d+),\"sid\":\"(\\w+)\",\"secret\":\"(\\w+)\",\"expire\":(\\d+)\\}";
                                if(std::regex_match(buffer,results,rex)) {
                                    info_.mid = results[1];
                                    info_.sid = results[2];
                                    info_.secret = results[3];
                                    info_.expire = results[4];
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
            return false;
        }
        vksession::session_info vksession::vksession_impl::info() const {
            return info_;
        }
        std::string vksession::vksession_impl::raw_call(const std::string& method,const parameter_list& param_list) {
            MD5 md5;
            std::string params;
            for(auto it = begin(param_list);it != end(param_list);++it) {
                params+="&"+it->first+"="+it->second;
            }

            std::list<std::string> list;
            list.emplace_back("api_id="+info_.api_key);
            list.emplace_back("method="+method);
            list.emplace_back("v=3.0");
            list.emplace_back("format=json");
            for(auto it = begin(param_list);it != end(param_list);++it) {
                list.emplace_back(it->first+"="+it->second);
            }
            list.sort();
            std::string str_for_md5 = info_.mid;
            for(auto it = begin(list);it != end(list);++it) {
                str_for_md5+=*it;
            }
            str_for_md5+=info_.secret;
            std::string req = "api.vk.com/api.php?v=3.0&api_id="+info_.api_key+
                                 "&method="+method+
                                 "&format=json"+
                                 "&sid="+info_.sid+
                                 params+
                                 "&sig="+md5.digestString(const_cast<char*>(str_for_md5.c_str()));
            curl_easy_setopt(curl.get(),CURLOPT_URL,req.c_str());
            std::string buffer;
            curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, writer);
            curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &buffer);
            CURLcode res = curl_easy_perform(curl.get());
            curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, dummy_writer);
            if(res == CURLE_OK) {
                char *s = curl_unescape(buffer.c_str(),buffer.length());
                std::string buffer = s;
                curl_free(s);
                return buffer;
            }
            return "";
        }
    };
};