// License: GPL-2.0

#include "xml/dom.h"

#include "gtest/gtest.h"


namespace {

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
