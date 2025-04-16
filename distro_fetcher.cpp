#include "distro_fetcher.hpp"

#include <cassert>              // assert
#include <cctype>               // toupper
#include <cstring>              // strstr
#include <ctime>                // localtime, mk_time
#include <algorithm>            // sort
#include <chrono>               // seconds
#include <iomanip>              // get_time
#include <stdexcept>            // runtime_error
#include <string>               // string, to_string
#include <sstream>              // stringstream
#include <thread>               // this_thread::sleep_for
#include <utility>              // forward
#include <vector>               // vector
#include <curl/curl.h>          // curl_easy_init
#include <json.h>               // json_tokener_parse
#include "Auto.h"               // The 'Auto' macro

using std::runtime_error;
using std::size_t;
using std::string;
using std::vector;

template<typename T>  // C++17 doesn't have 'requires'
/*constexpr*/ string Capitalize(T &&arg) noexcept(false)
{
    assert( false == arg.empty() );
    string result( std::forward<T>(arg) );
    result[0] = std::toupper( (char unsigned)result[0] );
    return result; 
}

template<typename T>  // C++17 doesn't have 'requires'
/*constexpr*/ string SizeToCol(T &&arg, unsigned len) noexcept(false)
{
    // allow an empty string
    string result( std::forward<T>(arg) );
    result.resize(len, ' ');
    return result;
}

static size_t WriteCallback(char const *const ptr,
                            size_t const size,
                            size_t const nmemb,
                            void *const userdata) noexcept
{
    try
    {
        static_cast<string*>(userdata)->append(ptr, size * nmemb);
        return size * nmemb;
    }
    catch(...){}

    return 0u;
}

string DistroFetcher::FetchJson(void) const noexcept(false)
{
    CURL *const curl = curl_easy_init();
    if ( nullptr == curl ) throw runtime_error("Failed to initialize Curl library for making HTTP requests");
    Auto( curl_easy_cleanup(curl) );

    string result;

    curl_easy_setopt( curl, CURLOPT_URL          , this->url.c_str() );
    curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, WriteCallback     );
    curl_easy_setopt( curl, CURLOPT_WRITEDATA    , &result           );

    for ( unsigned i = 1u; i <= 5u; ++i )  // Allow 5 attempts
    {
        if ( CURLE_OK == curl_easy_perform(curl) ) return result;
        std::this_thread::sleep_for( std::chrono::seconds(i) );
    }

    throw runtime_error("Failed to fetch JSON");
}

static bool HasDateExpired(char const *const str_date) noexcept
{
    using std::time_t;

    try
    {
        std::tm tm = {};

        // The next line checks that it's in the correct format
        // and also populates the 'tm' struct
        if ( (std::stringstream(str_date) >> std::get_time(&tm, "%Y-%m-%d")).fail() ) return true;

        tm.tm_hour = 0;
        tm.tm_min  = 0;
        tm.tm_sec  = 0;

        time_t const now_time = std::time(nullptr);

        std::tm *const now_tm = std::localtime(&now_time);

        now_tm->tm_hour = 0;
        now_tm->tm_min  = 0;
        now_tm->tm_sec  = 0;

        return std::mktime(&tm) < std::mktime(now_tm);
    }
    catch(...){}

    return true;
}

vector<string> DistroFetcher::GetSupportedReleases(size_t const max_count) const noexcept(false)
{
    string const json_text = FetchJson();

    json_object *const root = json_tokener_parse(json_text.c_str());
    if ( nullptr == root ) throw runtime_error("Failed to parse JSON");
    Auto( json_object_put(root) );

    json_object *products = nullptr;
    if ( false == json_object_object_get_ex(root, "products", &products) ) throw runtime_error("Missing 'products' in JSON");

    vector<string> releases;

    json_object_object_foreach(products, _, val)
    {
        (void)_;  // to suppress compiler warning

        json_object *support_eol_obj = nullptr;
        if ( false == json_object_object_get_ex(val, "support_eol", &support_eol_obj) ) continue;
        string const support_eol = json_object_get_string(support_eol_obj);
        if ( HasDateExpired(support_eol.c_str()) ) continue;

        json_object *release_title_obj = nullptr;
        if ( false == json_object_object_get_ex(val, "release_title", &release_title_obj) ) continue;
        bool const is_LTS = std::strstr( json_object_get_string(release_title_obj), "LTS" );

        json_object *versions = nullptr;
        if ( false == json_object_object_get_ex(val, "versions", &versions) ) continue;

        json_object_object_foreach(versions, date_key, version_val)
        {
            json_object *items = nullptr;
            if ( false == json_object_object_get_ex(version_val, "items", &items) ) continue;

            json_object *disk1 = nullptr;
            if ( false == json_object_object_get_ex(items, "disk1.img", &disk1) ) continue;

            json_object *path_obj = nullptr;
            if ( false == json_object_object_get_ex(disk1, "path", &path_obj) ) continue;

            string const path = json_object_get_string(path_obj);
            if ( string::npos == path.find("amd64") ) continue;

            json_object *pubname_obj = nullptr;
            if ( false == json_object_object_get_ex(version_val, "pubname", &pubname_obj) ) continue;
            string const pubname = json_object_get_string(pubname_obj);

            // Example: "ubuntu-oracular-24.10-amd64-server-20250305"
            vector<string> tokens;
            std::stringstream ss(pubname);
            string token;
            while ( std::getline(ss, token, '-') ) tokens.emplace_back(token);

            if ( tokens.size() < 6u ) continue;

            string const codename = tokens[1],
                         version  = tokens[2],
                         raw_date = date_key;

            string const formatted_date =
                raw_date.substr(0, 4) + "-" +
                raw_date.substr(4, 2) + "-" +
                raw_date.substr(6, 2);

            string const formatted =
                SizeToCol(formatted_date, 10) + "     " +
                SizeToCol(version       ,  5) + "     " +
                SizeToCol(Capitalize(codename), 12) + "  " +
                (is_LTS ? "LTS" : "");

            releases.emplace_back(formatted);
        }
    }

    std::sort( releases.rbegin(), releases.rend() );

    if ( releases.size() > max_count ) releases.resize(max_count);

    return releases;
}

string DistroFetcher::GetCurrentLTSVersion(void) const noexcept(false)
{
    vector<string> const releases = GetSupportedReleases();
    for ( auto const &r : releases )
    {
        if ( string::npos != r.find("LTS") ) return r;
    }
    return "unknown";
}

string DistroFetcher::GetDisk1Sha256(string_view const date) const noexcept(false)
{
    string digest = "unknown";

    string release(date);

    if ( 10u == release.size() )  // Remove the two hyphens from the date
    {
        release.erase(4u,1u);
        release.erase(6u,1u);
    }

    if ( 8u != release.size() ) return digest;
    
    string const json_text = FetchJson();

    json_object *const root = json_tokener_parse(json_text.c_str());
    if ( nullptr == root ) throw runtime_error("Failed to parse JSON");
    Auto( json_object_put(root) );

    json_object *products = nullptr;
    if ( false == json_object_object_get_ex(root, "products", &products) ) throw runtime_error("Missing 'products' in JSON");

    json_object_object_foreach(products, _, val)
    {
        (void)_;  // to suppress compiler warning

        bool double_break = false;

        json_object *versions = nullptr;
        if ( false == json_object_object_get_ex(val, "versions", &versions) ) continue;

        json_object_object_foreach(versions, date_key, version_val)
        {
            double_break = true;

            if ( date_key != release )
            {
                double_break = false;
                continue;
            }
 
            json_object *items = nullptr;
            if ( false == json_object_object_get_ex(version_val, "items", &items) ) break;

            json_object *disk1 = nullptr;
            if ( false == json_object_object_get_ex(items, "disk1.img", &disk1) ) break;

            json_object *sha256_obj = nullptr;
            if ( false == json_object_object_get_ex(disk1, "sha256", &sha256_obj) ) break;

            digest = json_object_get_string(sha256_obj);
            break;
        }

        if ( double_break ) break;
    }

    return digest;
}
