#include "pch.h"

#include <gtest/gtest.h>
#include "GlobalEnvironment.h"

int main(int argc, char **argv) {
    ComDLLsLibraryTearDown* env = new ComDLLsLibraryTearDown();
    ::testing::AddGlobalTestEnvironment(env);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}