// wujian@2018

#ifndef CONFIG_H
#define CONFIG_H

#include <map>
#include "decoder/common.h"

// Parse configure for acoustic feature cause too many parameters

class ConfigureParser {

public:
    ConfigureParser(const std::string &conf): conf_stream_(conf, std::ios::in) { 
        if (!conf_stream_.is_open())
            LOG_FAIL << "Open configure file " << conf_path_ << " failed";
        LoadConfigure();
    }

    ~ConfigureParser() { 
        if (conf_stream_.is_open())
            conf_stream_.close();    
    }

    // Could be optimized 
    void AddOptions(const std::string &opt, const std::string &name, Float32 *value);

    void AddOptions(const std::string &opt, const std::string &name, Int32 *value);

    void AddOptions(const std::string &opt, const std::string &name, Bool *value);

    void AddOptions(const std::string &opt, const std::string &name, std::string *value);

    // Generate configures
    std::string Configure();

private:
    typedef std::map<std::string, std::vector<std::pair<std::string, std::string> > > ConfigureMap;

    // Load configure from file stream
    void LoadConfigure();

    // Like python's strip()
    void StripString(std::string *str);

    std::string conf_path_;
    std::fstream conf_stream_;

    ConfigureMap table_;
};

#endif 