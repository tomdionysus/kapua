//
// Kapua Config class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#pragma once

#include <yaml.h>

#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

#include "Logger.hpp"

namespace Kapua {

// Structure to represent YAML nodes
class YamlNode {
 public:
  YamlNode();
  ~YamlNode();

  enum NodeType { SCALAR, MAPPING, SEQUENCE };

  NodeType type;
  std::string anchor;
  std::string key;
  std::string value;  // Only valid if type == SCALAR
  std::vector<YamlNode*> children;
};

enum ParserStatus { OK, STOP, ERROR };

class Config {
 public:
  Config(Logger* logger, std::string filename);
  ~Config();

  int load();
  void dump();

 protected:
  Logger* _logger;
  std::string _filename;
  YamlNode* _yaml_root;

  void print_yaml_node(YamlNode* node, int level = 0);
  ParserStatus parse_yaml_document(yaml_parser_t* parser, YamlNode* parent);
  ParserStatus parse_yaml_sequence(yaml_parser_t* parser, YamlNode* parent);
  ParserStatus parse_yaml_mapping(yaml_parser_t* parser, YamlNode* parent);
  ParserStatus parse_yaml_value(yaml_parser_t* parser, YamlNode* parent);

  YamlNode* _root;
};

};  // namespace Kapua
