#include <boost/test/unit_test.hpp>

#include <memory>

#include <cr/protobuf/any.h>

#include "unittest.pb.h"

BOOST_AUTO_TEST_SUITE(any)

BOOST_AUTO_TEST_CASE(getAnyTypeName)
{
    cr::unittest::HelloWorld helloWorld;
    cr::unittest::Any any;

    auto typeName = cr::protobuf::getAnyTypeName(any.value());
    BOOST_CHECK_EQUAL(typeName, "");

    any.mutable_value()->PackFrom(helloWorld);
    typeName = cr::protobuf::getAnyTypeName(any.value());
    BOOST_CHECK_EQUAL(typeName, helloWorld.GetTypeName());
}

BOOST_AUTO_TEST_SUITE_END()
