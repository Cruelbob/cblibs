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
std::string generate_scope_string(uint32_t scope) {
    std::string scope_string;
    scope_string += scope & cb::vkpp::rights::NOTIFY ? "notify," : "";
    scope_string += scope & cb::vkpp::rights::FRIENDS ? "friends," : "";
    scope_string += scope & cb::vkpp::rights::PHOTOS ? "photos," : "";
    scope_string += scope & cb::vkpp::rights::AUDIO ? "audio," : "";
    scope_string += scope & cb::vkpp::rights::VIDEO ? "video," : "";
    scope_string += scope & cb::vkpp::rights::DOCS ? "docs," : "";
    scope_string += scope & cb::vkpp::rights::NOTES ? "notes," : "";
    scope_string += scope & cb::vkpp::rights::PAGES ? "pages," : "";
    scope_string += scope & cb::vkpp::rights::STATUS ? "status," : "";
    scope_string += scope & cb::vkpp::rights::OFFERS ? "offers," : "";
    scope_string += scope & cb::vkpp::rights::QUESTIONS ? "questions," : "";
    scope_string += scope & cb::vkpp::rights::WALL ? "wall," : "";
    scope_string += scope & cb::vkpp::rights::GROUPS ? "groups," : "";
    scope_string += scope & cb::vkpp::rights::MESSAGES ? "messages," : "";
    scope_string += scope & cb::vkpp::rights::NOTIFICATIONS ? "notifications," : "";
    scope_string += scope & cb::vkpp::rights::STATS ? "stats," : "";
    scope_string += scope & cb::vkpp::rights::ADS ? "ads," : "";
    scope_string += scope & cb::vkpp::rights::OFFLINE ? "offline," : "";
    scope_string += scope & cb::vkpp::rights::NOHTTPS ? "nohttps," : "";
    return scope_string.substr(0,scope_string.length()-1);
}

namespace cb {
    namespace vkpp {
        vksession::vksession_impl::vksession_impl(const std::string& login,const std::string& password,const std::string& api_key,uint32_t scope):
          curl(curl_easy_init(),curl_easy_cleanup) {
            info_.api_key = api_key;
            if(curl) {
                curl_easy_setopt(curl.get(), CURLOPT_ERRORBUFFER, errorBuffer);
                curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION,1);
                curl_easy_setopt(curl.get(), CURLOPT_COOKIEFILE, "");
                curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, dummy_writer);
                curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 0L);
                curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 0L);
                if(auth_user(login,password)) {
                    if(auth_api(api_key,scope)) {
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
        bool vksession::vksession_impl::auth_api(const std::string& api_key,uint32_t scope) {
            std::string request = "oauth.vk.com/authorize?"
                                  "client_id="+api_key+"&"
                                  "scope="+generate_scope_string(scope)+"&" 
                                  "redirect_uri=http://oauth.vk.com/blank.html&"
                                  "display=popup&"
                                  "response_type=token";
            curl_easy_setopt(curl.get(),CURLOPT_URL, request.c_str());
            std::string response;
            curl_easy_setopt(curl.get(),CURLOPT_WRITEFUNCTION,writer);
            curl_easy_setopt(curl.get(),CURLOPT_WRITEDATA,&response);
            CURLcode res = curl_easy_perform(curl.get());
            curl_easy_setopt(curl.get(),CURLOPT_WRITEFUNCTION,dummy_writer);
            if(res == CURLE_OK) {
                std::regex rex("^.+location.href = \"(.+)\"\\+addr;$");
                std::smatch results;
                std::ofstream f("1.html");
                f << response;
                if(std::regex_search(response,results,rex)) {
                    std::string allow_request = results[1];
                    curl_easy_setopt(curl.get(), CURLOPT_URL,allow_request.c_str());
                    res = curl_easy_perform(curl.get());
                    if(res == CURLE_OK) {
                        char *raw_url = nullptr,
                             *unq_url = nullptr;
                        curl_easy_getinfo(curl.get(),CURLINFO_EFFECTIVE_URL,&raw_url);
                        int length = 0;
                        unq_url = curl_easy_unescape(curl.get(),raw_url,strlen(raw_url),&length);
                        std::string session = unq_url;
                        curl_free(unq_url);
                        rex = ".+access_token=(.+)&expires_in=(.+)&user_id=(.+)";
                        if(std::regex_search(session,results,rex)) {
                            info_.access_token = results[1];
                            info_.expires_in = results[2];
                            info_.user_id = results[3];
                            return true;
                        }
                        else {
                            throw cb::error::error(err_codes::ACCESS_TOKEN_NOT_RECEIVED,"Access token not received.");
                        }
                    }
                    else {
                        throw cb::error::error(err_codes::ALLOW_REQUEST_ERROR,std::string("Curl:")+errorBuffer);
                    }
                }
                else {
                    throw cb::error::error(err_codes::ALLOW_URL_NOT_FOUND,"Allow url not found.");
                }
            }
            else {
                throw cb::error::error(err_codes::API_AUTH_REQUEST_ERROR,std::string("Curl:")+errorBuffer);
            }
            return false;
        }
        vksession::session_info vksession::vksession_impl::info() const {
            return info_;
        }
        std::string vksession::vksession_impl::raw_call(const std::string& method,const parameter_list& param_list) {
            std::string request = "https://api.vk.com/method/"+method+"?";
            for(auto it = param_list.begin()++;it != param_list.end();++it) {
                request += it->first+"="+it->second+"&";
            }
            request += "access_token="+info_.access_token;
            curl_easy_setopt(curl.get(),CURLOPT_URL,request.c_str());
            std::string response;
            curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, writer);
            curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response);
            CURLcode res = curl_easy_perform(curl.get());
            curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, dummy_writer);
            if(res == CURLE_OK) {
                char *s = curl_unescape(response.c_str(),response.length());
                std::string response = s;
                curl_free(s);
                return response;
            }
            return "";
        }
    };
};