#include <gtest/gtest.h>
#include <list>
#include <ring_queue.h>
#include <set>
#include <stdio.h>
#include <string.h>

class NonDefaultConstructor {
public:
  NonDefaultConstructor() = delete;
};

class Base {
public:
  Base(int index = 0, const char *p = "nullptr") : index_(index) {
    // printf("con\n");
    if (p == nullptr) {
      str_ = new char[1];
      *str_ = '\0';
    } else {
      int len = strlen(p);
      str_ = new char[len + 1];
      memcpy(str_, p, len + 1);
    }
  }
  Base(const Base &other) {
    // printf("copy con\n");
    int len = strlen(other.str_);
    str_ = new char[len + 1];
    memcpy(str_, other.str_, len + 1);

    index_ = other.index_;
  }
  Base(Base &&other) {
    // printf("move copy con\n");
    str_ = other.str_;
    other.str_ = nullptr;

    index_ = other.index_;
  }
  Base &operator=(const Base &other) {
    // printf("operator=\n");
    if (&other != this) {
      delete[] str_;
      int len = strlen(other.str_);
      str_ = new char[len + 1];
      memcpy(str_, other.str_, len + 1);

      index_ = other.index_;
    }
    return *this;
  }
  ~Base() {
    // printf("destru\n");
    delete[] str_;
  }

  bool operator==(int x) const noexcept { return index_ == x; }

  int index_{0};
  char *str_{nullptr};

  friend bool operator<(const Base &x, const Base &y);
};

bool operator<(const Base &x, const Base &y) { return x.index_ < y.index_; }

class AllocatorTest : public testing::Test {
public:
  container::Allocator<Base, 5> alloc;
};

TEST_F(AllocatorTest, GetNodeFunc) {
  int chunk_size = sizeof(Base);

  Base *p1 = alloc.GetNode();
  Base *p2 = alloc.GetNode();
  Base *p3 = alloc.GetNode();
  alloc.PutNode(p1);

  ::new ((void *)p1) Base();
  ::new ((void *)p2) Base();

  static_cast<Base *>(p1)->~Base();
  static_cast<Base *>(p2)->~Base();

  EXPECT_EQ((char *)p2 - (char *)p1, chunk_size);
  EXPECT_EQ((char *)p3 - (char *)p1, chunk_size * 2);

  Base *p4 = alloc.GetNode();
  Base *p5 = alloc.GetNode();
  Base *p6 = alloc.GetNode();
  EXPECT_EQ(p1, p6);
}

TEST_F(AllocatorTest, MaxGetNode) {
  for (int i = 0; i < 1000; ++i) {
    Base *p = alloc.GetNode();
    if (p == nullptr) {
      EXPECT_EQ(i, 49);
      break;
    }
  }
}

class RingQueueTest : public testing::Test {
public:

public:
  container::RingQueue<int>  int_queue_;
  container::RingQueue<Base> base_queue_;
  container::RingQueue<NonDefaultConstructor> non_default_con_;
};

TEST_F(RingQueueTest, type_int) {
  int_queue_.push(2);
  int_queue_.push(3);
  int_queue_.push(1);

  EXPECT_EQ(int_queue_.front(), 1);
  int_queue_.pop();
  EXPECT_EQ(int_queue_.front(), 2);
  int_queue_.clear();
  EXPECT_TRUE(int_queue_.empty());
}

TEST_F(RingQueueTest, Init) {
  EXPECT_TRUE(base_queue_.empty());
  EXPECT_EQ(base_queue_.size(), 0);
}

TEST_F(RingQueueTest, push_3_element_order) {
  Base a(2);
  Base b(5);
  Base c(8);

  EXPECT_EQ(a.index_, 2);
  EXPECT_EQ(b.index_, 5);
  EXPECT_EQ(c.index_, 8);

  base_queue_.push(a);
  base_queue_.push(b);
  base_queue_.push(c);

  EXPECT_FALSE(base_queue_.empty());
  EXPECT_EQ(base_queue_.size(), 3);

  auto it = base_queue_.begin();
  EXPECT_EQ(it->index_, 2);
  ++it;
  EXPECT_EQ(it->index_, 5);
  ++it;
  EXPECT_EQ(it->index_, 8);
}

TEST_F(RingQueueTest, push_3_element_unorder) {
  Base a(5);
  Base b(8);
  Base c(2);

  base_queue_.push(a);
  base_queue_.push(b);
  base_queue_.push(c);

  EXPECT_FALSE(base_queue_.empty());
  EXPECT_EQ(base_queue_.size(), 3);

  auto it = base_queue_.begin();
  EXPECT_EQ(it->index_, 2);
  ++it;
  EXPECT_EQ(it->index_, 5);
  ++it;
  EXPECT_EQ(it->index_, 8);
}

TEST_F(RingQueueTest, push_15_element_order) {
  for (int i = 0; i < 15; ++i) {
    base_queue_.push(Base(i + 1));
  }

  EXPECT_FALSE(base_queue_.empty());
  EXPECT_EQ(base_queue_.size(), 10);

  auto it = base_queue_.begin();
  int i = 6;
  for (; it != base_queue_.end(); ++it, ++i) {
    EXPECT_EQ(it->index_, i);
  }
}

TEST_F(RingQueueTest, push_15_element_unorder) {
  for (int i = 15; i > 0; --i) {
    base_queue_.push(Base(i));
  }

  EXPECT_FALSE(base_queue_.empty());
  EXPECT_EQ(base_queue_.size(), 10);

  auto it = base_queue_.begin();
  int i = 6;
  for (; it != base_queue_.end(); ++it, ++i) {
    EXPECT_EQ(it->index_, i);
  }
}

TEST_F(RingQueueTest, preorder_emplace_time) {
  for (int i = 0; i < 1000000; ++i) {
    base_queue_.emplace(i);
  }

  EXPECT_FALSE(base_queue_.empty());
  EXPECT_EQ(base_queue_.size(), 10);
}

TEST_F(RingQueueTest, reverse_emplace_time) {
  for (int i = 1000000; i > 0; --i) {
    base_queue_.emplace(i);
  }

  EXPECT_FALSE(base_queue_.empty());
  EXPECT_EQ(base_queue_.size(), 10);
}

TEST_F(RingQueueTest, clear) {
  base_queue_.emplace(2);
  base_queue_.emplace(5);
  base_queue_.emplace(8);
  base_queue_.clear();

  EXPECT_TRUE(base_queue_.empty());
  EXPECT_EQ(base_queue_.size(), 0);
}

TEST(std__list, emplace_time) {
  std::list<Base> base_queue_;

  for (int i = 0; i < 1000000; ++i) {
    if (base_queue_.size() > 10) {
      base_queue_.clear();
    }
    base_queue_.emplace_back(i);
    base_queue_.sort();
  }
}

TEST(std__set, emplace_time) {
  std::set<Base> se;

  for (int i = 0; i < 1000000; ++i) {
    if (se.size() > 10) {
      se.clear();
    }
    se.emplace(i);
  }
}

TEST_F(RingQueueTest, push_pop) {
  base_queue_.emplace(3);
  base_queue_.emplace(2, "two");

  EXPECT_EQ(base_queue_.size(), 2);
  EXPECT_STREQ(base_queue_.front().str_, "two");
  EXPECT_EQ(base_queue_.front().index_, 2);

  base_queue_.pop();
  EXPECT_EQ(base_queue_.size(), 1);
  EXPECT_STREQ(base_queue_.front().str_, "nullptr");
  EXPECT_EQ(base_queue_.front().index_, 3);
}

TEST_F(RingQueueTest, pushes_popes) {
  base_queue_.emplace();
  base_queue_.pop();
  EXPECT_EQ(base_queue_.size(), 0);
  base_queue_.emplace();
  base_queue_.pop();
  EXPECT_EQ(base_queue_.size(), 0);
  base_queue_.emplace();
  base_queue_.pop();
  base_queue_.pop();
  base_queue_.pop();
  base_queue_.pop();
  EXPECT_EQ(base_queue_.size(), 0);
}

TEST_F(RingQueueTest, emplace) {
  base_queue_.emplace(1, "hello");
  EXPECT_FALSE(base_queue_.empty());
  EXPECT_EQ(base_queue_.size(), 1);
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
