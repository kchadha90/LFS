#include "util/logging.h"
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include "exttool/gtest.h"
namespace lfs {

TEST(UtilTest, LoggingLevel) {

  EXPECT_EXIT({
    Logging::set_min_level(kLLInfo);
    Logging::Log(kLLInfo, 1, "%d", 1);
    Logging::Log(kLLWarn, 1, "%d", 2);
    Logging::Log(kLLDebug, 1, "%d", 3);
    std::exit(0);
  }, ::testing::ExitedWithCode(0), "^1\n2\n$");

}

};//namespace lfs
