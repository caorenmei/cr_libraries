#include <boost/test/unit_test.hpp>

#include <string>
#include <vector>

#include <boost/lexical_cast.hpp>

#include <cr/common/streams.h>

BOOST_AUTO_TEST_SUITE(cr_common_streams)

BOOST_AUTO_TEST_CASE(from_array)
{
    int arr[] = { 1,2,3 };
    int sum = 0;
    cr::from(arr).forEach([&](auto&& e) {sum += e; });
    BOOST_CHECK_EQUAL(sum, 6);
}

BOOST_AUTO_TEST_CASE(from_stl_container)
{
    std::vector<int> ivec = { 1,2,3 };
    int sum = 0;
    cr::from(ivec).forEach([&](auto&& e) {sum += e; });
    BOOST_CHECK_EQUAL(sum, 6);
}

BOOST_AUTO_TEST_CASE(from_initializer_list)
{
    int sum = 0;
    cr::from({ 1,2,3 }).forEach([&](auto&& e) {sum += e; });
    BOOST_CHECK_EQUAL(sum, 6);
}

BOOST_AUTO_TEST_CASE(empty)
{
    int sum = 0;
    cr::empty<int>().forEach([&](auto&& e) {sum += e; });
    BOOST_CHECK_EQUAL(sum, 0);
}

BOOST_AUTO_TEST_CASE(filter)
{
    int sum = 0;
    cr::from({ 1,3,2,3 })
        .filter([](auto&& e) { return e < 3; })
        .forEach([&](auto&& e) { sum += e; });
    BOOST_CHECK_EQUAL(sum, 3);
}

BOOST_AUTO_TEST_CASE(deepClone)
{
    std::vector<std::string> svec = { "hello", "world" };
    std::size_t totalLength = 0;
    cr::from(svec) .deepClone().forEach([&](auto&& e) { e.clear(); });
    BOOST_CHECK_EQUAL(svec[1], "world");
}

BOOST_AUTO_TEST_CASE(map)
{
    int sum = 0;
    cr::from({ 1,2,3 }).map([](auto&& e) {return e * 10; }).forEach([&](auto&& e) { sum += e; });
    BOOST_CHECK_EQUAL(sum, 60);
}

BOOST_AUTO_TEST_CASE(flatMap)
{
    std::vector<std::string> svec = { "123", "456", "789" };
    BOOST_CHECK_EQUAL(cr::from(svec).flatMap([](auto&& e) { return cr::from(e); }).map([](auto&& e) { return e - '0'; }).sum().value_or(0), 45);
}

BOOST_AUTO_TEST_CASE(reduce)
{
    int sum = cr::from({ 1,2,3 }).reduce(0, [](auto&& a, auto&& b) { return a + b; });
    BOOST_CHECK_EQUAL(sum, 6);
    sum = cr::empty<int>().reduce(0, [](auto&& a, auto&& b) { return a + b; });
    BOOST_CHECK_EQUAL(sum, 0);
}

BOOST_AUTO_TEST_CASE(reduce_no_inivalue)
{
    int sum = cr::from({ 1,2,3 }).reduce([](auto&& a, auto&& b) { return a + b; }).value_or(0);
    BOOST_CHECK_EQUAL(sum, 6);
    sum = cr::empty<int>().reduce(0, [](auto&& a, auto&& b) { return a + b; });
    BOOST_CHECK_EQUAL(sum, 0);
}

BOOST_AUTO_TEST_CASE(concat)
{
    int sum = cr::from({ 1,2,3 }).concat(cr::from({ 4,5,6 })).reduce(0, [](auto&& a, auto&& b) { return a + b; });
    BOOST_CHECK_EQUAL(sum, 21);
}

BOOST_AUTO_TEST_CASE(distinct)
{
    int sum = cr::from({ 1,1,1,2,3,3 }).distinct().reduce(0, [](auto&& a, auto&& b) { return a + b; });
    BOOST_CHECK_EQUAL(sum, 6);
    sum = cr::from({ 1 }).distinct().reduce(0, [](auto&& a, auto&& b) { return a + b; });
    BOOST_CHECK_EQUAL(sum, 1);
    sum = cr::empty<int>().distinct().reduce(0, [](auto&& a, auto&& b) { return a + b; });
    BOOST_CHECK_EQUAL(sum, 0);
}

BOOST_AUTO_TEST_CASE(sorted)
{
    std::vector<int> ivec = { 1,4,3,2,5 };
    std::vector<int> ivec2;
    cr::from(ivec).sorted().forEach([&](auto&& e) { ivec2.push_back(e); });
    BOOST_CHECK_EQUAL(ivec2[1], 2);
    int sum = cr::empty<int>().sorted().distinct().reduce(0, [](auto&& a, auto&& b) { return a + b; });
    BOOST_CHECK_EQUAL(sum, 0);
}

BOOST_AUTO_TEST_CASE(equals)
{
    BOOST_CHECK(cr::from({ 1,2,3 }).equals(cr::from({ 1,2,3 })));
    BOOST_CHECK(!cr::from({ 1,2,3 }).equals(cr::from({ 1,2 })));
    BOOST_CHECK(!cr::from({ 1,2 }).equals(cr::from({ 1,2,3 })));
    BOOST_CHECK(!cr::from({ 1,2,3 }).equals(cr::empty<int>()));
    BOOST_CHECK(cr::empty<int>().equals(cr::empty<int>()));
}

BOOST_AUTO_TEST_CASE(reverse)
{
    BOOST_CHECK_EQUAL(cr::from({ 1,2,3 }).reverse().findFirst().value_or(0), 3);
}

BOOST_AUTO_TEST_CASE(findFirst)
{
    BOOST_CHECK_EQUAL(cr::from({ 1,2,3 }).findFirst().value_or(0), 1);
    BOOST_CHECK_EQUAL(cr::empty<int>().findFirst().value_or(0), 0);
}

BOOST_AUTO_TEST_CASE(findLast)
{
    BOOST_CHECK_EQUAL(cr::from({ 1,2,3 }).findLast().value_or(0), 3);
    BOOST_CHECK_EQUAL(cr::empty<int>().findLast().value_or(0), 0);
}

BOOST_AUTO_TEST_CASE(peek)
{
    std::vector<int> ivec = { 1,2,3 };
    cr::from(ivec).peek([](auto&& e) { e *= 10; }).count();
    BOOST_CHECK_EQUAL(ivec[1], 20);
}

BOOST_AUTO_TEST_CASE(limit)
{
    BOOST_CHECK_EQUAL(cr::from({ 1,2,3 }).limit(2).findLast().value_or(0), 2);
    BOOST_CHECK_EQUAL(cr::from({ 1,2,3 }).limit(0).findLast().value_or(0), 0);
    BOOST_CHECK_EQUAL(cr::empty<int>().limit(1).findLast().value_or(0), 0);
}

BOOST_AUTO_TEST_CASE(skip)
{
    BOOST_CHECK_EQUAL(cr::from({ 1,2,3 }).skip(2).findFirst().value_or(0), 3);
    BOOST_CHECK_EQUAL(cr::from({ 1,2,3 }).skip(3).findFirst().value_or(0), 0);
    BOOST_CHECK_EQUAL(cr::empty<int>().skip(1).findLast().value_or(0), 0);
}

BOOST_AUTO_TEST_CASE(until)
{
    BOOST_CHECK_EQUAL(cr::generate(0, 1).until([](auto&& e) { return e >= 3; }).sum().value_or(0), 3);
    BOOST_CHECK_EQUAL(cr::empty<int>().until([](auto&& e) { return e >= 3; }).sum().value_or(0), 0);
}

BOOST_AUTO_TEST_CASE(min)
{
    BOOST_CHECK_EQUAL(cr::from({ 3,1,2 }).min().value_or(0), 1);
}

BOOST_AUTO_TEST_CASE(max)
{
    BOOST_CHECK_EQUAL(cr::from({ 3,1,2 }).max().value_or(0), 3);
}

BOOST_AUTO_TEST_CASE(sum)
{
    BOOST_CHECK_EQUAL(cr::from({ 3,1,2 }).sum().value_or(0), 6);
    BOOST_CHECK_EQUAL(cr::empty<int>().sum().value_or(0), 0);
}

BOOST_AUTO_TEST_CASE(count)
{
    BOOST_CHECK_EQUAL(cr::from({ 3,1,2 }).count(), 3);
    BOOST_CHECK_EQUAL(cr::empty<int>().count(), 0);
}

BOOST_AUTO_TEST_CASE(anyMatch)
{
    BOOST_CHECK(cr::from({ 3,1,2 }).anyMatch([](auto&& e) { return e % 2 == 0; }));
    BOOST_CHECK(!cr::from({ 3,1,5 }).anyMatch([](auto&& e) { return e % 2 == 0; }));
    BOOST_CHECK(!cr::empty<int>().anyMatch([](auto&& e) { return e % 2 == 0; }));
}

BOOST_AUTO_TEST_CASE(allMatch)
{
    BOOST_CHECK(cr::from({ 3,1,2 }).allMatch([](auto&& e) { return e >= 1; }));
    BOOST_CHECK(!cr::from({ 3,1,2 }).allMatch([](auto&& e) { return e >= 2; }));
    BOOST_CHECK(cr::empty<int>().allMatch([](auto&& e) { return e >= 1; }));
}

BOOST_AUTO_TEST_CASE(noneMatch)
{
    BOOST_CHECK(!cr::from({ 3,1,2 }).noneMatch([](auto&& e) { return e >= 1; }));
    BOOST_CHECK(cr::from({ 3,1,2 }).noneMatch([](auto&& e) { return e >= 2; }));
    BOOST_CHECK(cr::from({ 3,1,2 }).noneMatch([](auto&& e) { return e > 3; }));
    BOOST_CHECK(!cr::empty<int>().noneMatch([](auto&& e) { return e >= 1; }));
}

BOOST_AUTO_TEST_CASE(generate)
{
    int initValue = 0;
    BOOST_CHECK_EQUAL(cr::generate([&] { return initValue++; }).skip(2).limit(3).sum().value_or(0), 9);
}

BOOST_AUTO_TEST_CASE(generate_step)
{
    int initValue = 0;
    BOOST_CHECK_EQUAL(cr::generate(0, 1).limit(3).sum().value_or(0), 3);
}

BOOST_AUTO_TEST_CASE(rangeFor)
{
    int sum = 0;
    for (int e : cr::from({ 1,2,3 }))
    {
        sum += e;
    }
    BOOST_CHECK_EQUAL(sum, 6);
}

BOOST_AUTO_TEST_SUITE_END()