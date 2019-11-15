#pragma once

#ifndef __GLOBAL_ENVIRONMNET_TEARDOWN_GTEST_HH
#define __GLOBAL_ENVIRONMNET_TEARDOWN_GTEST_HH

#include <gtest/gtest.h>
#include <SettingUtils.h>

class ComDLLsLibraryTearDown : public ::testing::Environment
{
public:
    virtual ~ComDLLsLibraryTearDown () = default;

    // Override this to define how to set up the environment.
    virtual void SetUp();

    // Override this to define how to tear down the environment.
    virtual void TearDown();
};

#endif // !__GLOBAL_ENVIRONMNET_GTEST_HH
