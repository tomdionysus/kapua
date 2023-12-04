#include "Config.hpp"

#include "Logger.hpp"

using namespace std;

namespace Kapua {

YamlNode::YamlNode() {}

YamlNode::~YamlNode() {
  for (YamlNode* child : children) {
    delete child;
  }
}

Config::Config(Logger* logger, std::string filename = "config.yaml") {
  _logger = new ScopedLogger("Configuration", logger);
  _filename = filename;
}

Config::~Config() { delete _logger; }

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
  YamlNode root_node;
  parse_yaml_document(&parser, &root_node);

  _logger->debug("Closing config file " + _filename);
  fclose(file);

  // Print the AST for debugging
  _logger->debug("Printing config");
  print_yaml_node(&root_node);

  // Clean up
  yaml_parser_delete(&parser);

  return 0;
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
  std::cout << node->key << ": " << node->value
            << std::endl;
  for (YamlNode* child : node->children) {
    print_yaml_node(child, level + 1);
  }
}

void Config::dump() {}

ParserStatus Config::parse_yaml_document(yaml_parser_t* parser,
                                         YamlNode* node) {
  cout << "> parse_yaml_document\n";
  yaml_event_t event;
  ParserStatus status;

  bool loop = true;
  while (loop) {
    if (!yaml_parser_parse(parser, &event)) return ParserStatus::ERROR;

    switch (event.type) {
      // Ignore these events
      case YAML_NO_EVENT:
        cout << "YAML_NO_EVENT\n";
        break;
      case YAML_STREAM_START_EVENT:
        cout << "YAML_STREAM_START_EVENT\n";
        break;
      case YAML_DOCUMENT_START_EVENT:
        cout << "YAML_DOCUMENT_START_EVENT\n";
        break;
      case YAML_DOCUMENT_END_EVENT:
        cout << "YAML_DOCUMENT_END_EVENT\n";
        break;

      // End of Stream
      case YAML_STREAM_END_EVENT:
        cout << "YAML_STREAM_END_EVENT\n";
        return ParserStatus::OK;

      // No idea what this is
      case YAML_ALIAS_EVENT:
        cout << "YAML_ALIAS_EVENT\n";
        break;

      // Whole doc is a mapping
      case YAML_MAPPING_START_EVENT:
        cout << "YAML_MAPPING_START_EVENT\n";
        node->type = YamlNode::MAPPING;
        status = parse_yaml_mapping(parser, node);
        break;

      // Whole doc is a sequence
      case YAML_SEQUENCE_START_EVENT:
        cout << "YAML_SEQUENCE_START_EVENT\n";
        node->type = YamlNode::SEQUENCE;
        status = parse_yaml_sequence(parser, node);
        break;

      default:
        cout << "Unknown Event Type " + std::to_string(event.type) + "\n";
        return ParserStatus::ERROR;
    }
  }

  return ParserStatus::ERROR;
}

ParserStatus Config::parse_yaml_mapping(yaml_parser_t* parser,
                                        YamlNode* parent) {
  cout << "> parse_yaml_mapping\n";
  yaml_event_t event;
  ParserStatus status;
  YamlNode* node;

  while (true) {
    if (!yaml_parser_parse(parser, &event)) return ParserStatus::ERROR;

    switch (event.type) {
      // We found the end of the sequence
      case YAML_MAPPING_END_EVENT:
        cout << "YAML_MAPPING_END_EVENT - parse_yaml_mapping\n";
        return ParserStatus::OK;

      // A scalar value, the value of which can be a scalar, map or sequence.
      case YAML_SCALAR_EVENT:
        cout << "YAML_SCALAR_EVENT - parse_yaml_mapping\n";

        node = new YamlNode();
        node->key = reinterpret_cast<const char*>(event.data.scalar.value);
        cout << " - Key: " + node->key + "\n";
        status = parse_yaml_value(parser, node);
        if (status != ParserStatus::OK) {
          delete node;
          return status;
        }
        cout << "->Push Child\n";
        parent->children.push_back(node);
        break;

      default:
        cout << "Unknown Event Type " + std::to_string(event.type) + "\n";
        return ParserStatus::ERROR;
    }
  }
}

ParserStatus Config::parse_yaml_sequence(yaml_parser_t* parser,
                                         YamlNode* parent) {
  cout << "> parse_yaml_sequence\n";
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
    cout << "-> Push Child - parse_yaml_sequence\n";
    parent->children.push_back(node);
  }

  return ParserStatus::OK;
}

ParserStatus Config::parse_yaml_value(yaml_parser_t* parser, YamlNode* node) {
  cout << "> parse_yaml_value\n";
  yaml_event_t event;
  ParserStatus status;
  YamlNode* childnode;

  if (!yaml_parser_parse(parser, &event)) return ParserStatus::ERROR;

  switch (event.type) {
    // Start of a key/value map
    case YAML_MAPPING_START_EVENT:
      cout << "YAML_MAPPING_START_EVENT - parse_yaml_value\n";
      node->type = YamlNode::MAPPING;
      status = parse_yaml_mapping(parser, node);
      if (status == ParserStatus::ERROR) {
        return status;
      }
      cout << "->Push Child\n";
      return ParserStatus::OK;

    case YAML_MAPPING_END_EVENT:
      cout << "YAML_MAPPING_END_EVENT - parse_yaml_value SHOULD NOT HAPPEN\n";
      return ParserStatus::STOP;

    // Start of a sequence
    case YAML_SEQUENCE_START_EVENT:
      node->type = YamlNode::SEQUENCE;
      cout << "YAML_SEQUENCE_START_EVENT - parse_yaml_value\n";
      status = parse_yaml_sequence(parser, node);
      if (status == ParserStatus::ERROR) return ParserStatus::ERROR;
      return ParserStatus::OK;

    case YAML_SEQUENCE_END_EVENT:
      cout << "YAML_SEQUENCE_END_EVENT - parse_yaml_value\n";
      return ParserStatus::STOP;

    // A scalar value, the value of which can be a scalar, map or sequence.
    case YAML_SCALAR_EVENT:
      cout << "YAML_SCALAR_EVENT - parse_yaml_value\n";
      node->type = YamlNode::SCALAR;
      node->value = reinterpret_cast<const char*>(event.data.scalar.value);
      cout << " - Value: " + node->value + "\n";
      return ParserStatus::OK;

    default:
      cout << "Unknown Event Type " + std::to_string(event.type) + "\n";
      return ParserStatus::ERROR;
  }
}

}  // namespace Kapua