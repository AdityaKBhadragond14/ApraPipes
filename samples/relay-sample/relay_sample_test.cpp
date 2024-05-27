#include "relay_sample.h"
#include <boost/test/unit_test.hpp>
#include <chrono>

BOOST_AUTO_TEST_SUITE(relay_sample)

BOOST_AUTO_TEST_CASE(relaySample) {
  RelayPipeline relayPipeline;
  relayPipeline.setupPipeline();
  relayPipeline.testPipeline();
}

BOOST_AUTO_TEST_SUITE_END()
