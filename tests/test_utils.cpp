#include "gtest/gtest.h"
<<<<<<< HEAD
#include "utilities.h"
=======
#include "prototypes.h"
>>>>>>> 6aca630b002cca78d5634cadbcd83271fba000e5


TEST(TestWallClock, one)
{
    double t_test=-wallclock();
    doublesleep(0.1);
    t_test += wallclock();
    ASSERT_TRUE((t_test>=0.1));
    ASSERT_TRUE((t_test<1.1));
}

TEST(TestAzzero, doubles)
{
    double *buf = new double[10];
    for (int i=0; i<10; ++i) buf[i]=i+1;
    ASSERT_DOUBLE_EQ(buf[1],2.0);
    ASSERT_DOUBLE_EQ(buf[5],6.0);
    ASSERT_DOUBLE_EQ(buf[9],10.0);

    azzero(buf,10);
    ASSERT_DOUBLE_EQ(buf[1],0.0);
    ASSERT_DOUBLE_EQ(buf[5],0.0);
    ASSERT_DOUBLE_EQ(buf[9],0.0);
}
