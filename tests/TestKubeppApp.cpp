#include "KubeppApp.h"
#include <gtest/gtest.h>

TEST(KubeppAppTest, RunMethod) {
    kubepp::KubeppApp app;
    EXPECT_EQ(app.run(), "KubeppApp is running!");
}

int main(int argc, char **argv){
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
