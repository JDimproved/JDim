// License: GPL-2.0

#include "xml/dom.h"

#include "gtest/gtest.h"


namespace {

class XML_DomGetXml : public ::testing::Test {};

TEST_F(XML_DomGetXml, xml_empty)
{
    XML::Dom dom{ XML::NODE_TYPE_UNKNOWN, "test" };
    std::string xml = dom.get_xml();
    EXPECT_EQ( "", xml );
}

TEST_F(XML_DomGetXml, xml_empty_document)
{
    XML::Dom dom{ XML::NODE_TYPE_DOCUMENT, "document" };
    std::string xml = dom.get_xml();
    EXPECT_EQ( "", xml );
}

TEST_F(XML_DomGetXml, xml_single_element)
{
    XML::Dom dom{ XML::NODE_TYPE_DOCUMENT, "document" };
    XML::Dom* root = dom.appendChild( XML::NODE_TYPE_ELEMENT, "root" );
    root->setAttribute( "name", "value" );
    std::string xml = dom.get_xml();
    EXPECT_EQ( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<root name=\"value\" />\n", xml );
}

TEST_F(XML_DomGetXml, xml_single_child_element)
{
    XML::Dom dom{ XML::NODE_TYPE_DOCUMENT, "document" };
    XML::Dom* root = dom.appendChild( XML::NODE_TYPE_ELEMENT, "root" );
    root->appendChild( XML::NODE_TYPE_ELEMENT, "child" );
    std::string xml = dom.get_xml();
    EXPECT_EQ( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<root>\n  <child />\n</root>\n", xml );
}

TEST_F(XML_DomGetXml, xml_single_child_text)
{
    XML::Dom dom{ XML::NODE_TYPE_DOCUMENT, "document" };
    XML::Dom* root = dom.appendChild( XML::NODE_TYPE_ELEMENT, "root" );
    XML::Dom* child = root->appendChild( XML::NODE_TYPE_TEXT, "child" );
    child->nodeValue( "hello world" );
    std::string xml = dom.get_xml();
    EXPECT_EQ( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<root>\n  hello world\n</root>\n", xml );
}

TEST_F(XML_DomGetXml, xml_nesting_child_element)
{
    XML::Dom dom{ XML::NODE_TYPE_DOCUMENT, "document" };
    XML::Dom* root = dom.appendChild( XML::NODE_TYPE_ELEMENT, "root" );
    XML::Dom* child = root->appendChild( XML::NODE_TYPE_ELEMENT, "first" );
    child->appendChild( XML::NODE_TYPE_ELEMENT, "second" );
    std::string xml = dom.get_xml();
    std::string expect = R"=(<?xml version="1.0" encoding="UTF-8"?>
<root>
  <first>
    <second />
  </first>
</root>
)=";
    EXPECT_EQ( expect, xml );
}


class XML_DomProperty : public ::testing::Test {};

TEST_F(XML_DomProperty, node_type)
{
    const auto types = {
        XML::NODE_TYPE_UNKNOWN,
        XML::NODE_TYPE_ELEMENT,
        XML::NODE_TYPE_TEXT,
        XML::NODE_TYPE_DOCUMENT,
    };
    for( const auto t : types ) {
        XML::Dom dom{ t, "test" };
        int result = dom.nodeType();
        EXPECT_EQ( t, result );
    }
}

TEST_F(XML_DomProperty, node_name)
{
    XML::Dom dom{ XML::NODE_TYPE_UNKNOWN, "test" };
    std::string name = dom.nodeName();
    EXPECT_EQ( "test", name );
}

TEST_F(XML_DomProperty, node_value)
{
    XML::Dom dom{ XML::NODE_TYPE_UNKNOWN, "test" };
    std::string value = dom.nodeValue();
    EXPECT_EQ( "", value );
}


class XML_DomGetElement : public ::testing::Test {};

TEST_F(XML_DomGetElement, empty_id)
{
    XML::Dom dom{ XML::NODE_TYPE_UNKNOWN, "test" };
    XML::Dom* result = dom.getElementById( "" );
    EXPECT_EQ( nullptr, result );
}

TEST_F(XML_DomGetElement, not_found_id)
{
    XML::Dom dom{ XML::NODE_TYPE_UNKNOWN, "test" };
    XML::Dom* child = dom.appendChild( XML::NODE_TYPE_UNKNOWN, "unknown" );
    child->setAttribute( "id", "the-id" );
    child = dom.appendChild( XML::NODE_TYPE_TEXT, "text" );
    child->setAttribute( "id", "the-id" );
    child = dom.appendChild( XML::NODE_TYPE_DOCUMENT, "document" );
    child->setAttribute( "id", "the-id" );
    XML::Dom* result = dom.getElementById( "the-id" );
    EXPECT_EQ( nullptr, result );
}

TEST_F(XML_DomGetElement, found_first_id)
{
    XML::Dom dom{ XML::NODE_TYPE_UNKNOWN, "test" };
    XML::Dom* child = dom.appendChild( XML::NODE_TYPE_ELEMENT, "first" );
    child->setAttribute( "id", "the-id" );
    XML::Dom* expect = child;
    child = dom.appendChild( XML::NODE_TYPE_ELEMENT, "second" );
    child->setAttribute( "id", "the-id" );
    XML::Dom* result = dom.getElementById( "the-id" );
    EXPECT_EQ( expect, result );
}

TEST_F(XML_DomGetElement, empty_tag_name)
{
    XML::Dom dom{ XML::NODE_TYPE_UNKNOWN, "test" };
    const std::list<XML::Dom*> result = dom.getElementsByTagName( "" );
    EXPECT_TRUE( result.empty() );
}

TEST_F(XML_DomGetElement, not_found_tag_name)
{
    XML::Dom dom{ XML::NODE_TYPE_UNKNOWN, "test" };
    dom.appendChild( XML::NODE_TYPE_UNKNOWN, "the-tag" );
    dom.appendChild( XML::NODE_TYPE_TEXT, "the-tag" );
    dom.appendChild( XML::NODE_TYPE_DOCUMENT, "the-tag" );
    const std::list<XML::Dom*> result = dom.getElementsByTagName( "" );
    EXPECT_TRUE( result.empty() );
}

TEST_F(XML_DomGetElement, found_tag_name)
{
    XML::Dom dom{ XML::NODE_TYPE_UNKNOWN, "test" };
    XML::Dom* first = dom.appendChild( XML::NODE_TYPE_ELEMENT, "the-tag" );
    dom.appendChild( XML::NODE_TYPE_ELEMENT, "another-tag" );
    XML::Dom* second = dom.appendChild( XML::NODE_TYPE_ELEMENT, "the-tag" );
    const std::list<XML::Dom*> expect = { first, second };
    const std::list<XML::Dom*> result = dom.getElementsByTagName( "the-tag" );
    EXPECT_EQ( expect, result );
}


class XML_DomChildren : public ::testing::Test {};

TEST_F(XML_DomChildren, has_child_nodes)
{
    XML::Dom dom{ XML::NODE_TYPE_UNKNOWN, "test" };
    bool result = dom.hasChildNodes();
    EXPECT_FALSE( result );

    dom.appendChild( XML::NODE_TYPE_ELEMENT, "child" );
    result = dom.hasChildNodes();
    EXPECT_TRUE( result );
}

TEST_F(XML_DomChildren, child_nodes)
{
    XML::Dom dom{ XML::NODE_TYPE_UNKNOWN, "unknown" };
    XML::Dom* first = dom.appendChild( XML::NODE_TYPE_ELEMENT, "element" );
    XML::Dom* second = dom.appendChild( XML::NODE_TYPE_TEXT, "text" );
    XML::Dom* third = dom.appendChild( XML::NODE_TYPE_DOCUMENT, "document" );
    const std::list<XML::Dom*> expect = { first, second, third };
    const std::list<XML::Dom*> result = dom.childNodes();
    EXPECT_EQ( expect, result );
}

TEST_F(XML_DomChildren, append_child)
{
    XML::Dom dom{ XML::NODE_TYPE_UNKNOWN, "test" };
    XML::Dom* child = dom.appendChild( XML::NODE_TYPE_ELEMENT, "child" );
    EXPECT_NE( nullptr, child );
    EXPECT_EQ( XML::NODE_TYPE_ELEMENT, child->nodeType() );
    EXPECT_EQ( "child", child->nodeName() );
}


TEST_F(XML_DomChildren, first_child)
{
    XML::Dom dom{ XML::NODE_TYPE_UNKNOWN, "test" };
    XML::Dom* child = dom.firstChild();
    EXPECT_EQ( nullptr, child );

    dom.appendChild( XML::NODE_TYPE_ELEMENT, "child" );
    child = dom.firstChild();
    EXPECT_NE( nullptr, child );
    EXPECT_EQ( XML::NODE_TYPE_ELEMENT, child->nodeType() );
    EXPECT_EQ( "child", child->nodeName() );
}

TEST_F(XML_DomChildren, remove_child)
{
    XML::Dom dom{ XML::NODE_TYPE_UNKNOWN, "test" };
    bool result = dom.removeChild( nullptr );
    EXPECT_FALSE( result );

    XML::Dom* child = dom.appendChild( XML::NODE_TYPE_ELEMENT, "child" );
    result = dom.removeChild( child );
    EXPECT_TRUE( result );
    EXPECT_FALSE( dom.hasChildNodes() );
}

TEST_F(XML_DomChildren, emplace_front)
{
    XML::Dom dom{ XML::NODE_TYPE_UNKNOWN, "test" };

    dom.appendChild( XML::NODE_TYPE_ELEMENT, "child1" );
    XML::Dom* second_child = dom.emplace_front( XML::NODE_TYPE_ELEMENT, "child2" );
    EXPECT_EQ( dom.firstChild(), second_child );

    XML::Dom* third_child = dom.emplace_front( XML::NODE_TYPE_ELEMENT, "child2" );
    EXPECT_EQ( dom.firstChild(), third_child );
}

TEST_F(XML_DomChildren, children_clear)
{
    XML::Dom dom{ XML::NODE_TYPE_UNKNOWN, "test" };
    dom.appendChild( XML::NODE_TYPE_ELEMENT, "child1" );
    dom.appendChild( XML::NODE_TYPE_ELEMENT, "child2" );
    dom.clear();
    EXPECT_FALSE( dom.hasChildNodes() );
}


class XML_DomAttribute : public ::testing::Test {};

TEST_F(XML_DomAttribute, set_not_element_type)
{
    XML::Dom dom_unknown{ XML::NODE_TYPE_UNKNOWN, "unknown" };
    bool result = dom_unknown.setAttribute( "name", "value" );
    EXPECT_FALSE( result );
    XML::Dom dom_text{ XML::NODE_TYPE_TEXT, "text" };
    result = dom_text.setAttribute( "name", "value" );
    EXPECT_FALSE( result );
    XML::Dom dom_doc{ XML::NODE_TYPE_DOCUMENT, "document" };
    result = dom_doc.setAttribute( "name", "value" );
    EXPECT_FALSE( result );
}

TEST_F(XML_DomAttribute, set_empty_name_with_string_value)
{
    XML::Dom dom{ XML::NODE_TYPE_ELEMENT, "test" };
    bool result = dom.setAttribute( "", "value" );
    EXPECT_FALSE( result );
}

TEST_F(XML_DomAttribute, set_empty_name_with_integer_value)
{
    XML::Dom dom{ XML::NODE_TYPE_ELEMENT, "test" };
    bool result = dom.setAttribute( "", 123 );
    EXPECT_FALSE( result );
}

TEST_F(XML_DomAttribute, set_empty_string_value)
{
    XML::Dom dom{ XML::NODE_TYPE_ELEMENT, "test" };
    bool result = dom.setAttribute( "name", "" );
    EXPECT_FALSE( result );
}

TEST_F(XML_DomAttribute, get_empty)
{
    XML::Dom dom{ XML::NODE_TYPE_ELEMENT, "test" };
    std::string value = dom.getAttribute( "" );
    EXPECT_EQ( "", value );
}

TEST_F(XML_DomAttribute, get_not_found)
{
    XML::Dom dom{ XML::NODE_TYPE_ELEMENT, "test" };
    std::string value = dom.getAttribute( "not-found" );
    EXPECT_EQ( "", value );
}

TEST_F(XML_DomAttribute, set_string_value)
{
    XML::Dom dom{ XML::NODE_TYPE_ELEMENT, "test" };
    bool result = dom.setAttribute( "hello", "world" );
    std::string value = dom.getAttribute( "hello" );
    EXPECT_TRUE( result );
    EXPECT_EQ( "world", value );
}

TEST_F(XML_DomAttribute, set_string_value_twice)
{
    XML::Dom dom{ XML::NODE_TYPE_ELEMENT, "test" };
    dom.setAttribute( "hello", "world" );
    bool result = dom.setAttribute( "hello", "jdim" );
    std::string value = dom.getAttribute( "hello" );
    EXPECT_TRUE( result );
    EXPECT_EQ( "world", value );
}

TEST_F(XML_DomAttribute, set_positive_integer_value)
{
    XML::Dom dom{ XML::NODE_TYPE_ELEMENT, "test" };
    bool result = dom.setAttribute( "width", 32767 );
    std::string value = dom.getAttribute( "width" );
    EXPECT_TRUE( result );
    EXPECT_EQ( "32767", value );
}

TEST_F(XML_DomAttribute, set_negative_integer_value)
{
    XML::Dom dom{ XML::NODE_TYPE_ELEMENT, "test" };
    bool result = dom.setAttribute( "width", -32768 );
    std::string value = dom.getAttribute( "width" );
    EXPECT_TRUE( result );
    EXPECT_EQ( "-32768", value );
}

TEST_F(XML_DomAttribute, set_integer_value_twice)
{
    XML::Dom dom{ XML::NODE_TYPE_ELEMENT, "test" };
    dom.setAttribute( "width", 12321 );
    bool result = dom.setAttribute( "width", 0 );
    std::string value = dom.getAttribute( "width" );
    EXPECT_TRUE( result );
    EXPECT_EQ( "12321", value );
}

TEST_F(XML_DomAttribute, set_uppercase_name_with_string_value)
{
    XML::Dom dom{ XML::NODE_TYPE_ELEMENT, "test" };
    bool result = dom.setAttribute( "UPPER", "hello world" );
    EXPECT_TRUE( result );
    std::string value = dom.getAttribute( "UPPER" );
    EXPECT_EQ( "", value );
    value = dom.getAttribute( "upper" );
    EXPECT_EQ( "hello world", value );
}

TEST_F(XML_DomAttribute, set_uppercase_name_with_integer_value)
{
    XML::Dom dom{ XML::NODE_TYPE_ELEMENT, "test" };
    bool result = dom.setAttribute( "UPPER", 256 );
    EXPECT_TRUE( result );
    std::string value = dom.getAttribute( "UPPER" );
    EXPECT_EQ( "", value );
    value = dom.getAttribute( "upper" );
    EXPECT_EQ( "256", value );
}

} // namespace
