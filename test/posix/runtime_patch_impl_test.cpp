// Copyright 2014 Louix Gu
// Author: gzc9047@gmail.com (Louix Gu)

// CppFreeMock: a tool for mock global function, member function, class static function.
//
// RuntimePatcherImpl under posix test.

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "posix/runtime_patch_impl.h"

// Use CppFreeMock to mock some global function to test itself.
#include "cpp_free_mock.h"

#include <iostream>
#include <vector>

using namespace ::std;
using namespace ::testing;
using namespace ::CppFreeMock;

TEST(RuntimePatcherImpl, AlignAddress) {
    cout << "PageSize: " << RuntimePatcherImpl::PageSize << endl;
    for (size_t i = 0; i < RuntimePatcherImpl::PageSize; ++i) {
        EXPECT_EQ(0, RuntimePatcherImpl::AlignAddress(i, RuntimePatcherImpl::PageSize));
    }
}

TEST(RuntimePatcherImpl, mprotect) {
    CREATE_MOCKER(mprotectMocker, mprotect);
    EXPECT_CALL(*mprotectMocker, MOCK_FUNCTION(nullptr, 0, 0))
        .Times(Exactly(1))
        .WillOnce(Return(9047));
    EXPECT_EQ(9047, mprotect(nullptr, 0, 0));
}

TEST(RuntimePatcherImpl, UnprotectMemory) {
    CREATE_MOCKER(alignAddressMocker, RuntimePatcherImpl::AlignAddress);
    void* address = nullptr;
    size_t length = 0;
    EXPECT_CALL(*alignAddressMocker, MOCK_FUNCTION(0, RuntimePatcherImpl::PageSize))
        .Times(Exactly(1))
        .WillOnce(Return(0));
    EXPECT_EQ(0, RuntimePatcherImpl::UnprotectMemory(address, length));
}

TEST(RuntimePatcherImpl, UnprotectMemoryForOnePage) {
    CREATE_MOCKER(unprotectMemoryMocker, RuntimePatcherImpl::UnprotectMemory);
    EXPECT_CALL(*unprotectMemoryMocker, MOCK_FUNCTION(nullptr, RuntimePatcherImpl::PageSize))
        .Times(Exactly(1))
        .WillOnce(Return(9047));
    EXPECT_EQ(9047, RuntimePatcherImpl::UnprotectMemoryForOnePage(nullptr));
}

TEST(RuntimePatcherImpl, IsDistanceOverflow) {
    // Non overflow.
    for (long long i = INT32_MIN; i <= INT32_MAX; i += 0x10000000) {
        EXPECT_FALSE(RuntimePatcherImpl::IsDistanceOverflow(i));
    }
    EXPECT_FALSE(RuntimePatcherImpl::IsDistanceOverflow(INT32_MAX));
    // Overflow/
    long long distance = INT32_MIN;
    --distance;
    EXPECT_TRUE(RuntimePatcherImpl::IsDistanceOverflow(distance));
    distance = INT32_MAX;
    ++distance;
    EXPECT_TRUE(RuntimePatcherImpl::IsDistanceOverflow(distance));
    EXPECT_TRUE(RuntimePatcherImpl::IsDistanceOverflow(INT64_MIN));
    EXPECT_TRUE(RuntimePatcherImpl::IsDistanceOverflow(INT64_MAX));
}

TEST(RuntimePatcherImpl, SetJump64bitAddress) {
    char data[16];
    void* function = reinterpret_cast<void*>(data);
    CREATE_MOCKER(patchFunction64bitAddressMocker, RuntimePatcherImpl::PatchFunction64bitAddress);
    CREATE_MOCKER(calculateDistanceMocker, RuntimePatcherImpl::CalculateDistance);
    EXPECT_CALL(*calculateDistanceMocker, MOCK_FUNCTION(function, nullptr))
        .Times(Exactly(1))
        .WillOnce(Return(INT64_MAX));
    EXPECT_CALL(*patchFunction64bitAddressMocker, MOCK_FUNCTION(reinterpret_cast<char*>(function), nullptr))
        .Times(Exactly(1));
    vector<char> binary_backup;
    EXPECT_EQ(0, RuntimePatcherImpl::SetJump(function, nullptr, binary_backup));
}

TEST(RuntimePatcherImpl, SetJump) {
    char data[16];
    char data2[16];
    void* function = reinterpret_cast<void*>(data);
    void* destination = reinterpret_cast<void*>(data2);
    CREATE_MOCKER(patchFunction64bitAddressMocker, RuntimePatcherImpl::PatchFunction64bitAddress);
    CREATE_MOCKER(patchFunction32bitDistanceMocker, RuntimePatcherImpl::PatchFunction32bitDistance);
    EXPECT_CALL(*patchFunction64bitAddressMocker, MOCK_FUNCTION(_, _))
        .Times(Exactly(0));
    EXPECT_CALL(*patchFunction32bitDistanceMocker, MOCK_FUNCTION(reinterpret_cast<char*>(function), _))
        .Times(Exactly(1));
    vector<char> binary_backup;
    EXPECT_EQ(0, RuntimePatcherImpl::SetJump(function, destination, binary_backup));
}
