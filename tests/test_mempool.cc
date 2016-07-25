#include "defs.h"
#include "utils.h"
#include "pool_alloc.h"
#include "gtest/gtest.h"

TEST(TEST_ALIGN, mempool)
{
  ASSERT_EQ(ALIGN(14,16), 16);
  ASSERT_EQ(ALIGN(24,16), 32);
  ASSERT_EQ(ALIGN(56,16), 64);
}

TEST(TEST_CREATE, mempool)
{
  pool_t *pool = create_pool(2048);
  ASSERT_EQ(pool->max_size, 2048);
  u_char* p = (u_char *) pool;
  ASSERT_EQ(p + sizeof(pool_t), pool->d.last);
  ASSERT_TRUE(pool->d.next ==  NULL);
  ASSERT_EQ(pool->current, pool);
  destroy_pool(pool);
}

TEST(TEST_ALLOC_SMALL, mempool)
{
  pool_t *pool = create_pool(2048);
  u_char *p = (u_char*) pool_alloc(pool, 8);
  u_char *pdata = (u_char*)pool + sizeof(pool_t);
  ASSERT_EQ(p, pdata);
  ASSERT_EQ(pdata + 8, pool->d.last);
  u_char *p1 = (u_char *) pool_alloc(pool, 16);
  ASSERT_EQ(p1, pdata + 8);
  ASSERT_EQ(pool->d.last, pdata + 24);
  destroy_pool(pool);
}

int main(int argc, char* argv[])
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
