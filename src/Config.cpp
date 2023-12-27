//
// Kapua Config class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#include "Config.hpp"

#include "Logger.hpp"

using namespace std;

namespace Kapua {

YamlNode::YamlNode() {}

YamlNode::~YamlNode() {
  for (YamlNode* child : children) delete child;
  children.empty();
}

Config::Config(Logger* logger, std::string filename = "config.yaml") {
  _logger = new ScopedLogger("Configuration", logger);
  _filename = filename;
  _yaml_root = new YamlNode();
}

Config::~Config() {
  delete _yaml_root;
  delete _logger;
}

int Config::load() {
  // Open and read the YAML file
  _logger->debug("Opening config file " + _filename);
  FILE* file = fopen(_filename.c_str(), "r");
  if (!file) {
    _logger->error("Error opening file " + _filename);
    return 1;
  }

  // Initialize libyaml parser
  yaml_parser_t parser;
  yaml_parser_initialize(&parser);
  yaml_parser_set_input_file(&parser, file);

  _logger->debug("Parsing config file");
  parse_yaml_document(&parser, _yaml_root);

  _logger->debug("Closing config file " + _filename);
  fclose(file);

  // Clean up
  yaml_parser_delete(&parser);

  return 0;
}

void Config::dump() {
  // Print the AST for debugging
  _logger->debug("Printing config");
  print_yaml_node(_yaml_root);
}

// Function to print the AST for debugging
void Config::print_yaml_node(YamlNode* node, int level) {
  for (int i = 0; i < level; i++) {
    std::cout << "  ";
  }
  std::cout << "Type: ";
  switch (node->type) {
    case YamlNode::SCALAR:
      std::cout << "SCALAR, ";
      break;
    case YamlNode::MAPPING:
      std::cout << "MAPPING, ";
      break;
    case YamlNode::SEQUENCE:
      std::cout << "SEQUENCE, ";
      break;
  }
  std::cout << "Anchor: " << node->anchor << ", ";
  std::cout << node->key << ": " << node->value << std::endl;
  for (YamlNode* child : node->children) {
    print_yaml_node(child, level + 1);
  }
}

ParserStatus Config::parse_yaml_document(yaml_parser_t* parser, YamlNode* node) {
  yaml_event_t event;
  ParserStatus status;

  bool loop = true;
  while (loop) {
    if (!yaml_parser_parse(parser, &event)) return ParserStatus::ERROR;

    // _logger->debug("Document YAML Event "+std::to_string(event.type));

    switch (event.type) {
      // Something has gone wrong
      case YAML_NO_EVENT:
        return ParserStatus::ERROR;

      // Ignore these events
      case YAML_STREAM_START_EVENT:
      case YAML_DOCUMENT_START_EVENT:
      case YAML_DOCUMENT_END_EVENT:
      // TODO: Support Alias Events
      case YAML_ALIAS_EVENT:
        break;

      // End of Stream
      case YAML_STREAM_END_EVENT:
        return ParserStatus::OK;

      // Whole doc is a mapping
      case YAML_MAPPING_START_EVENT:
        node->type = YamlNode::MAPPING;
        status = parse_yaml_mapping(parser, node);
        break;

      // Whole doc is a sequence
      case YAML_SEQUENCE_START_EVENT:
        node->type = YamlNode::SEQUENCE;
        status = parse_yaml_sequence(parser, node);
        break;

      default:
        _logger->error("YAML Parser - Unknown Event Type " + std::to_string(event.type));
        return ParserStatus::ERROR;
    }
  }

  return ParserStatus::ERROR;
}

ParserStatus Config::parse_yaml_mapping(yaml_parser_t* parser, YamlNode* parent) {
  yaml_event_t event;
  ParserStatus status;
  YamlNode* node;

  while (true) {
    if (!yaml_parser_parse(parser, &event)) return ParserStatus::ERROR;

    // _logger->debug("Mapping YAML Event "+std::to_string(event.type));

    switch (event.type) {
      // We found the end of the sequence
      case YAML_MAPPING_END_EVENT:
        return ParserStatus::OK;

      // A scalar value, the value of which can be a scalar, map or sequence.
      case YAML_SCALAR_EVENT:
        node = new YamlNode();
        node->key = reinterpret_cast<const char*>(event.data.scalar.value);
        status = parse_yaml_value(parser, node);
        if (status != ParserStatus::OK) {
          delete node;
          return status;
        }
        parent->children.push_back(node);
        break;

      default:
        _logger->error("YAML Parser - Unknown Event Type " + std::to_string(event.type));
        return ParserStatus::ERROR;
    }
  }
}

ParserStatus Config::parse_yaml_sequence(yaml_parser_t* parser, YamlNode* parent) {
  yaml_event_t event;

  YamlNode* node;

  while (true) {
    node = new YamlNode();
    ParserStatus status = parse_yaml_value(parser, node);
    if (status == ParserStatus::ERROR) {
      delete node;
      return status;
    }
    if (status == ParserStatus::STOP) return status;
    parent->children.push_back(node);
  }

  return ParserStatus::OK;
}

ParserStatus Config::parse_yaml_value(yaml_parser_t* parser, YamlNode* node) {
  yaml_event_t event;
  ParserStatus status;
  YamlNode* childnode;

  if (!yaml_parser_parse(parser, &event)) return ParserStatus::ERROR;

  // _logger->debug("Value YAML Event "+std::to_string(event.type));

  switch (event.type) {
    // Start of a key/value map
    case YAML_MAPPING_START_EVENT:
      node->type = YamlNode::MAPPING;
      status = parse_yaml_mapping(parser, node);
      if (status == ParserStatus::ERROR) {
        return status;
      }
      return ParserStatus::OK;

    case YAML_MAPPING_END_EVENT:
      return ParserStatus::STOP;

    // Start of a sequence
    case YAML_SEQUENCE_START_EVENT:
      node->type = YamlNode::SEQUENCE;
      status = parse_yaml_sequence(parser, node);
      if (status == ParserStatus::ERROR) return ParserStatus::ERROR;
      return ParserStatus::OK;

    case YAML_SEQUENCE_END_EVENT:
      return ParserStatus::STOP;

    // A scalar value, the value of which can be a scalar, map or sequence.
    case YAML_SCALAR_EVENT:
      node->type = YamlNode::SCALAR;
      node->value = reinterpret_cast<const char*>(event.data.scalar.value);
      return ParserStatus::OK;

      // TODO: Support Aliases
    case YAML_ALIAS_EVENT:
      return ParserStatus::ERROR;

    default:
      _logger->error("YAML Parser - Unknown Event Type " + std::to_string(event.type));
      return ParserStatus::ERROR;
  }
}

}  // namespace Kapua