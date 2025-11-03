#include <functional>
#include <iostream>
#include <string>

#include "mystl/__hash_table.h"

// 1. 定义要存储的值类型
struct MyPair {
  int key;
  std::string value;

  // 辅助函数，用于从值中提取键
  static int GetKey(const MyPair &p) { return p.key; }
};

// 2. 定义支持异构查找的哈希函数对象
// 它既可以对 MyPair 进行哈希，也可以对 int 类型的键进行哈希。
struct MyPairHash {
  using is_transparent = void; // 标记，用于启用异构查找

  // a. 用于查找/删除: 传入 int 类型的键
  size_t operator()(int k) const { return std::hash<int>()(k); }

  // b. 用于插入: 传入完整的 MyPair 对象
  size_t operator()(const MyPair &p) const {
    return std::hash<int>()(MyPair::GetKey(p));
  }
};

// 3. 定义支持异构查找的相等比较函数对象
struct MyPairEqual {
  using is_transparent = void;

  // a. 用于查找/删除: 比较 MyPair 和 int 类型的键
  bool operator()(const MyPair &lhs, int rhs_key) const {
    return MyPair::GetKey(lhs) == rhs_key;
  }

  // b. 用于内部比较: 比较两个 MyPair 对象
  bool operator()(const MyPair &lhs, const MyPair &rhs) const {
    return MyPair::GetKey(lhs) == MyPair::GetKey(rhs);
  }
};

int main() {
  std::cout << "--- HashTable Usage Example (Corrected) ---" << std::endl;

  // 定义哈希表类型别名，方便使用
  using MyTable = mystl::hash_table<MyPair, MyPairHash, MyPairEqual,
                                    std::allocator<MyPair>>;

  // 4. 正确的初始化步骤
  // a. 使用接受 hasher 和 key_equal 的构造函数创建一个空表
  MyTable ht{MyPairHash(), MyPairEqual()};

  // b. 手动调用 rehash_unique 来设置初始桶数
  ht.rehash_unique(10);
  std::cout << "HashTable created. Initial bucket count: " << ht.bucket_count()
            << std::endl;

  std::cout << "\n1. Inserting elements..." << std::endl;
  ht.insert_unique({1, "apple"});
  ht.insert_unique({2, "banana"});
  ht.insert_unique({10, "orange"});
  ht.insert_unique({15, "grape"});

  auto result = ht.insert_unique({2, "cherry"});
  if (!result.second) {
    std::cout << "Insertion failed for key 2: Key already exists." << std::endl;
  }
  std::cout << "Current size: " << ht.size() << std::endl;

  std::cout << "\n2. Finding elements (with integer key)..." << std::endl;
  auto it = ht.find(10); // 现在可以直接用 int 查找
  if (it != ht.end()) {
    std::cout << "Found key 10 with value: " << it->value << std::endl;
  }

  auto it_missing = ht.find(99);
  if (it_missing == ht.end()) {
    std::cout << "Key 99 not found, as expected." << std::endl;
  }

  std::cout << "\n3. Erasing elements (with integer key)..." << std::endl;
  size_t erased_count = ht.erase_unique(2); // 现在可以直接用 int 删除
  if (erased_count > 0) {
    std::cout << "Erased key 2. Number of elements erased: " << erased_count
              << std::endl;
  }
  std::cout << "Current size: " << ht.size() << std::endl;

  std::cout << "\n4. Iterating through remaining elements..." << std::endl;
  for (const auto &elem : ht) {
    std::cout << "  - Key: " << elem.key << ", Value: " << elem.value
              << std::endl;
  }

  std::cout << "\n--- Example Finished ---" << std::endl;
  return 0;
}
