#ifndef RING_QUEUE_H_
#define RING_QUEUE_H_

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>

namespace container {

template <class T, uint32_t Num> class Allocator {
public:
  Allocator() {
    chunk_size_ = (sizeof(T) + 7) & (~7);
    InitOnePool();
  }
  ~Allocator() { DestroyAlloctor(); }

  T *GetNode() noexcept {
    T *res = nullptr;

    if (start_ == end_) {
      res = (T *)start_;

      if (InitOnePool() != 0) {
        return nullptr;
      }

      return res;
    }
    res = (T *)start_;
    start_ = start_->next;

    return res;
  }

  void PutNode(T *chunk) noexcept {
    end_->next = (Chunk *)chunk;
    end_ = (Chunk *)chunk;
  }

private:
  union Chunk {
    union Chunk *next{nullptr};
    char data[1];
  };

  static const int MAXPOOLNUM = 10;

  void *headers_[MAXPOOLNUM];
  int pool_num_{0};
  uint32_t chunk_size_{0};
  Chunk *start_{nullptr};
  Chunk *end_{nullptr};

  int InitOnePool() noexcept {
    if (pool_num_ >= MAXPOOLNUM) {
      return -1;
    }

    Chunk *mem = (Chunk *)calloc(1, chunk_size_ * Num);
    if (mem == nullptr) {
      printf("memory init error\n");
      exit(-1);
    }

    Chunk *cur = mem;
    Chunk *next = mem;
    for (int i = 1;; ++i) {
      cur = next;
      next = (Chunk *)((char *)next + chunk_size_);
      if (i == Num) {
        cur->next = nullptr;
        break;
      } else {
        cur->next = next;
      }
    }

    start_ = mem;
    end_ = cur; //(Chunk*)((char*)cur + sizeof(T));
    headers_[pool_num_++] = mem;

    return 0;
  }

  void DestroyAlloctor() noexcept {
    for (int i = 0; i < pool_num_; ++i) {
      free(headers_[i]);
    }
  }
};

template <class T> struct QueueNode {
  char data[sizeof(T)];
  QueueNode<T> *prev{nullptr};
  QueueNode<T> *next{nullptr};
};

template <class T> struct Iterator {
  using Self = Iterator<T>;

  Iterator(QueueNode<T> *node) : node_(node) {}

  T &operator*() const noexcept { return *(T *)node_->data; }

  T *operator->() const noexcept { return (T *)node_->data; }

  Self &operator++() noexcept {
    node_ = node_->next;
    return *this;
  }

  Self &operator++(int) noexcept {
    Self tmp = *this;
    node_ = node_->next;
    return tmp;
  }

  Self &operator--() noexcept {
    node_ = node_->prev;
    return *this;
  }

  Self &operator--(int) noexcept {
    Self tmp = *this;
    node_ = node_->prev;
    return tmp;
  }

  bool operator==(const Self &other) const noexcept {
    return node_ == other.node_;
  }

  bool operator!=(const Self &other) const noexcept {
    return node_ != other.node_;
  }

  QueueNode<T> *node_{nullptr};
};

template <class T, uint32_t Size = 10, class Compare = std::less<T>>
class RingQueue {
public:
  using iterator = Iterator<T>;
  using Node = QueueNode<T>;

public:
  RingQueue() { Init(); }
  ~RingQueue() { clear(); }

  inline bool empty() const { return dummy_->next == dummy_; }

  uint32_t size() { return length_; }

  iterator begin() noexcept { return iterator(dummy_->next); }

  iterator end() noexcept { return iterator(dummy_); }

  T &front() const noexcept { return *(T *)dummy_->next->data; }

  void push(const T &value) {
    // 'value' is biger than all element in the queue mostly, hence, we should
    // find the location from tail, and this runs well in
    // x64(Intel i7-9700K)

    // 'node' is the first one smaller than 'value' in inverse order
    Node *node = nullptr;
    for (node = dummy_->prev; node != dummy_; node = node->prev) {
      if (value_compare_(*(T *)node->data, value)) {
        break;
      }
    }

    Node *new_node = alloc_.GetNode();
    _construct((T *)new_node->data, value);

    node->next->prev = new_node;
    new_node->next = node->next;
    new_node->prev = node;
    node->next = new_node;
    length_++;

    // if queue is full, delete head node
    if (length_ > Size) {
      Node *head = dummy_->next;
      _destroy((T *)head->data);

      dummy_->next = head->next;
      head->next->prev = dummy_;
      length_--;

      alloc_.PutNode(head);
    }
  }

  template <class... Args> void emplace(Args &&... args) {
    Node *new_node = alloc_.GetNode();
    _construct((T *)new_node->data, std::forward<Args>(args)...);

    Node *node = nullptr;
    for (node = dummy_->prev; node != dummy_; node = node->prev) {
      if (value_compare_(*(T *)node->data, *(T *)new_node->data)) {
        break;
      }
    }

    node->next->prev = new_node;
    new_node->next = node->next;
    new_node->prev = node;
    node->next = new_node;
    length_++;

    // if queue is full, delete head node
    if (length_ > Size) {
      Node *head = dummy_->next;
      _destroy((T *)head->data);

      dummy_->next = head->next;
      head->next->prev = dummy_;
      length_--;

      alloc_.PutNode(head);
    }
  }

  // pop front
  void pop() noexcept {
    if (length_ > 0) {
      Node *head = dummy_->next;
      _destroy((T *)head->data);

      dummy_->next = head->next;
      head->next->prev = dummy_;
      length_--;

      alloc_.PutNode(head);
    }
  }

  iterator erase(const iterator &position) noexcept {
    iterator next(position.node_->next);

    if (length_ > 0) {
      position.node_->next->prev = position.node_->prev;
      position.node_->prev->next = position.node_->next;
      length_--;

      _destroy((T *)position.node_->data);
      alloc_.PutNode(position.node_);
    }

    return next;
  }

  void clear() noexcept {
    for (Node *tmp = dummy_->next; tmp != dummy_; tmp = tmp->next) {
      _destroy((T *)tmp->data);
      alloc_.PutNode(tmp);
    }

    dummy_->next = dummy_;
    dummy_->prev = dummy_;
    length_ = 0;
  }

private:
  Compare value_compare_;
  Allocator<Node, Size + 2> alloc_;
  Node *dummy_{nullptr};
  int length_{0};

private:
  void Init() {
    dummy_ = alloc_.GetNode();

    dummy_->next = dummy_;
    dummy_->prev = dummy_;
  }

  template <class... Args> inline void _construct(T *_p, Args &&... _args) {
    ::new (static_cast<void *>(_p)) T(std::forward<Args>(_args)...);
  }

  inline void _destroy(T *_p) { _p->~T(); }
};

} // namespace container

#endif
