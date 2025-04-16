#include <cstdio>               // puts
#include <cstdlib>              // exit, EXIT_FAILURE
#include <cstring>              // strcmp
#include <exception>            // exception
#include <iostream>             // cout, endl
#include <string>               // string
#include "distro_fetcher.hpp"   // DistroFetcher

using std::string;
using std::cout;
using std::endl;

void ShowHelp(char const *const argv0, int const exit_code) noexcept
{
    
    std::printf(
              "Usage: %s [options]\n"
              "Options:\n"
              "  --help, -h                Show this help message\n"
              "  --list-releases           List all supported Ubuntu releases\n"
              "  --current-lts             Get the current Ubuntu LTS version\n"
              "  --hash <release date>     Get the SHA256 hash digest of the 'disk1.img' file for a specific release\n"
              "\n"
              "Examples:\n"
              "    %s --list-releases\n"
              "    %s --hash 2025-04-03\n", argv0, argv0, argv0);

    std::exit(exit_code);
}

constexpr char url[] = "https://cloud-images.ubuntu.com/releases/streams/v1/com.ubuntu.cloud:released:download.json";

int main_proper(int const argc, char **const argv)
{
    using std::strcmp;

    DistroFetcher fetcher(url);

    if ( (3 == argc) && (0 == strcmp(argv[1], "--hash")) )
    {
        string const digest = fetcher.GetDisk1Sha256(argv[2]);
        cout << "Hash SHA256 digest of file 'disk1.img' for release date " << argv[2] << ": " << digest << endl;
        return (digest == "unknown") ? EXIT_FAILURE : 0;
    }
    else if ( 2 == argc )
    {
        if ( (0==strcmp(argv[1], "--help")) || (0==strcmp(argv[1], "-h")) )
        {
            ShowHelp(argv[0], 0);
        }
        else if ( 0 == strcmp(argv[1], "--list-releases") )
        {
            cout << "Date           Version   Codename\n"
                    "------------------------------------------\n";
            auto const releases = fetcher.GetSupportedReleases();
            for ( auto const &r : releases ) cout << r << '\n';
            return 0;
        }
        else if ( 0 == strcmp(argv[1], "--current-lts") )
        {
            cout << "Current Ubuntu LTS version: " << fetcher.GetCurrentLTSVersion() << endl;
            return 0;
        }   
    }

    ShowHelp(argv[0], EXIT_FAILURE);
}

int main(int const argc, char **const argv)
{
    try
    {
        return main_proper(argc, argv);
    }
    catch (std::exception const &e)
    {
        cout << "Error : Unhandled Exception : " << e.what() << endl;
    }
    catch (...)
    {
        cout << "Error : Unhandled Exception : Unknown\n";
    }

    return EXIT_FAILURE;
}

