#define BOOST_TEST_MODULE cr_network_unittest
#include <boost/test/included/unit_test.hpp>

#include <google/protobuf/stubs/common.h>

struct ShutdownProtobufLibrary
{
    ~ShutdownProtobufLibrary()
    {
        google::protobuf::ShutdownProtobufLibrary();
    }
};

BOOST_GLOBAL_FIXTURE(ShutdownProtobufLibrary);