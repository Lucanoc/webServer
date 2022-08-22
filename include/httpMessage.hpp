#pragma once

#include <string>

class httpMessage {
public:
    explicit httpMessage() = default;

    void setMethod(char const * method_);
    void setUrl(char const * url_);
    void setVersion(char const * version_);

    auto getMethod() const -> std::string const &;
    auto getUrl() const -> std::string const &;
    auto getVersion() const -> std::string const &;
private:
    std::string method;
    std::string url;
    std::string version;
};

inline
void httpMessage::setMethod(const char *method_) {
    method = method_;
}

inline
void httpMessage::setUrl(const char *url_) {
    url = url_;
}

inline
void httpMessage::setVersion(const char *version_) {
    version = version_;
}

inline
auto httpMessage::getMethod() const -> std::string const & {
    return method;
}

inline
auto httpMessage::getUrl() const -> std::string const & {
    return url;
}

inline
auto httpMessage::getVersion() const -> std::string const & {
    return version;
}