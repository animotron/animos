/* ���� ���-���� */
static void test_foo(void) {
   /* ��� ���-���� */
}

/* ��ன ���-���� */
static void test_foo2(void) {
   /* ��� ���-���� */
}

void runSuite(void) {
}


#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>

#include "utils.h"

TEST_FUNCT(foo) {
    printf("test case 1\n");
     /* ������� ��� */
    CU_ASSERT_EQUAL(0, 1);
}
TEST_FUNCT(foo2) {
   printf("test case 2\n");
    /* ������� ��� */
    CU_ASSERT_EQUAL(1, 1);
}

void runSuite(void) {
    /* ��� ���-���� */

    printf("test suite\n");

    CU_pSuite suite = CUnitCreateSuite("Suite1");
    if (suite) {
        ADD_SUITE_TEST(suite, foo)
        ADD_SUITE_TEST(suite, foo2)
    }
}
