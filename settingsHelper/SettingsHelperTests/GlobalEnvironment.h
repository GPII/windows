#pragma once

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
