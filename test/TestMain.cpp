#include "UnitTesting.h"
#include "TestFstAudioEffect.h"
#include "TestAudioeffectx.h"

int main(int argc, char **argv)
{
    TEST_INIT();
    RUN_TEST_SUITE(TestFstAudioEffect);
    RUN_TEST_SUITE(TestAudioeffectx);
    TEST_END();
}