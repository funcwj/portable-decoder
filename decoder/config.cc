// wujian@2018

#include "decoder/config.h"

void ConfigureParser::LoadConfigure() {
  while (!conf_stream_.eof()) {
    std::string line;
    std::getline(conf_stream_, line);
    StripString(&line);
    // comment or space line
    if (line[0] == '#' || !line.size()) continue;
    std::string::iterator space_iter = std::find(line.begin(), line.end(), ' ');
    std::string::iterator equal_iter = std::find(line.begin(), line.end(), '=');
    if (space_iter != line.end() || equal_iter == line.end() ||
        line.substr(0, 2) != "--")
      LOG_FAIL << "Wrong configure line: \'" << line << "\'";
    if (std::count(line.begin(), line.end(), '=') != 1)
      LOG_FAIL << "Wrong configure line: \'" << line << "\'";
    Int32 point_pos = line.find_first_of("."),
          equal_pos = line.find_first_of("=");
    std::string conf_name = line.substr(2, point_pos - 2),
                arg_name =
                    line.substr(point_pos + 1, equal_pos - point_pos - 1),
                arg_value = line.substr(equal_pos + 1, line.size() - equal_pos);
    table_[conf_name].push_back(std::make_pair(arg_name, arg_value));
    std::string key = "--" + conf_name + "." + arg_name;
    if (status_.count(key)) LOG_FAIL << "Duplicated key " << key << " existed";
    status_[key] = false;
  }
}

void ConfigureParser::StripString(std::string *str) {
  ASSERT(str && "String point is NULL!");
  const char *white_chars = " \t\n\r\f";
  Int32 last = str->find_last_not_of(white_chars);
  if (last != std::string::npos) {
    str->erase(last + 1);
    Int32 first = str->find_first_not_of(white_chars);
    if (first != std::string::npos) str->erase(0, first);
  } else {
    // White line
    str->erase(str->begin(), str->end());
  }
}

std::string ConfigureParser::Configure() {
  std::ostringstream oss;
  for (ConfigureMap::iterator it = table_.begin(); it != table_.end(); it++) {
    const std::string &opt = it->first;
    const std::vector<std::pair<std::string, std::string> > &vec = it->second;
    for (std::pair<std::string, std::string> p : vec) {
      oss << "--" << opt << "." << p.first << "=" << p.second << std::endl;
    }
  }
  return oss.str();
}

void ConfigureParser::AddOptions(const std::string &opt,
                                 const std::string &name, Float32 *value) {
  if (!table_.count(opt)) return;
  const std::vector<std::pair<std::string, std::string> > &vec = table_[opt];
  for (std::pair<std::string, std::string> p : vec) {
    if (p.first == name) {
      *value = std::stof(p.second);
      std::string key = "--" + opt + "." + name;
      status_[key] = true;
      break;
    }
  }
}

void ConfigureParser::AddOptions(const std::string &opt,
                                 const std::string &name, Int32 *value) {
  if (!table_.count(opt)) return;
  const std::vector<std::pair<std::string, std::string> > &vec = table_[opt];
  for (std::pair<std::string, std::string> p : vec) {
    if (p.first == name) {
      *value = std::stoi(p.second);
      std::string key = "--" + opt + "." + name;
      status_[key] = true;
      break;
    }
  }
}

void ConfigureParser::AddOptions(const std::string &opt,
                                 const std::string &name, Bool *value) {
  if (!table_.count(opt)) return;
  const std::vector<std::pair<std::string, std::string> > &vec = table_[opt];
  for (std::pair<std::string, std::string> p : vec) {
    if (p.first == name) {
      if (p.second != "true" && p.second != "false")
        LOG_FAIL << "Invalid value for Bool type: " << p.second;
      if (p.second == "true") *value = true;
      if (p.second == "false") *value = false;
      std::string key = "--" + opt + "." + name;
      status_[key] = true;
      break;
    }
  }
}

void ConfigureParser::AddOptions(const std::string &opt,
                                 const std::string &name, std::string *value) {
  if (!table_.count(opt)) return;
  const std::vector<std::pair<std::string, std::string> > &vec = table_[opt];
  for (std::pair<std::string, std::string> p : vec) {
    if (p.first == name) {
      value->clear();
      value->assign(p.second);
      std::string key = "--" + opt + "." + name;
      status_[key] = true;
      break;
    }
  }
}