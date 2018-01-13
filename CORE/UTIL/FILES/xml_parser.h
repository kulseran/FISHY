/**
 * Basic XML parsing utility.
 */
#ifndef FISHY_SIMPLE_XML_PARSER_H
#define FISHY_SIMPLE_XML_PARSER_H

#include <CORE/BASE/status.h>
#include <CORE/types.h>

#include <map>
#include <string>
#include <vector>

namespace core {
namespace util {
namespace files {

/**
 * Simple XML parsing.
 */
struct XmlNode {
  public:
  XmlNode() {}

  /**
   * @return this node's name.
   */
  const std::string &getName() const;

  /**
   * @return this node's string content excluding child nodes.
   */
  const std::string &getContent() const;

  /**
   * Looks up a node's attribute by name
   *
   * @param name the name of the attribute to lookup.
   * @param value the value of the attribute, if return is true.
   * @return true if attribute {@code name} existed.
   */
  bool getAttribute(const std::string &name, std::string &value) const;

  /**
   * Looks up a child node by name
   *
   * @param name the name of the child to lookup.
   * @param index the node's index, in case this named child is repeated.
   * @param startIndex optional starting index, in case the named child is
   *     repeated.
   * @return true if the node {@code name} existed.
   */
  bool getChild(
      const std::string &name,
      size_t &index,
      const size_t startIndex = 0) const;

  /**
   * @param index the index of the node to find, must be less than
   *    {@link #getChildCount}.
   * @return a specific child {@link XmlNode}
   */
  const XmlNode &getChild(const size_t index) const;

  /**
   * @return the count of child nodes.
   */
  size_t getChildCount() const;

  /**
   * Parse the input {@code fileData} as nodes to be inserted with this as the
   * parent.
   */
  Status parse(const std::string &fileData);

  private:
  std::string m_name;
  std::string m_str;
  std::map< std::string, std::string > m_attributes;
  std::vector< XmlNode > m_children;

  friend class XmlNodeBuilder;

  XmlNode(
      const std::string &name,
      const std::string &content,
      std::map< std::string, std::string > &attributes,
      std::vector< XmlNode > &children);

  Status parse(
      std::string::const_iterator &begin,
      const std::string::const_iterator &end);
};

} // namespace files
} // namespace util
} // namespace core

#endif
