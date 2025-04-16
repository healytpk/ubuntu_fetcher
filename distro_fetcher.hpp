#ifndef HEADER_INCLUSION_GUARD_4CBE0D66_3092_4C4A_B05A_64A4E5DCBF47
#define HEADER_INCLUSION_GUARD_4CBE0D66_3092_4C4A_B05A_64A4E5DCBF47

#include <cstddef>              // size_t
#include <string>               // string
#include <string_view>          // string_view
#include <vector>               // vector

class IDistroFetcher {
public:
    using size_t      = std::size_t;
    using string      = std::string;
    using string_view = std::string_view;
    template<typename... Ts> using vector = std::vector<Ts...>;

    virtual ~IDistroFetcher(void) noexcept = default;

    virtual vector<string> GetSupportedReleases(size_t max_count) const noexcept(false) = 0;
    virtual string         GetCurrentLTSVersion(void            ) const noexcept(false) = 0;
    virtual string         GetDisk1Sha256      (string_view date) const noexcept(false) = 0;
};

class DistroFetcher : public IDistroFetcher {
protected:
    string url;
public:
    DistroFetcher(string_view const arg_url) noexcept : url(arg_url) {}

    virtual vector<string> GetSupportedReleases(size_t max_count = -1) const noexcept(false) override;
    virtual string         GetCurrentLTSVersion(void                 ) const noexcept(false) override;
    virtual string         GetDisk1Sha256      (string_view date     ) const noexcept(false) override;
protected:
    string FetchJson(void) const noexcept(false);
};

#endif
