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

#ifdef _WIN32
#include <windows.h>
#endif

int main() {
#ifdef _WIN32
  // 在 Windows 平台上，设置控制台输出编码为 UTF-8
  SetConsoleOutputCP(CP_UTF8);
#endif

  std::cout << "--- 哈希表使用示例 ---" << std::endl;

  // 定义哈希表类型别名，方便使用
  using MyTable = mystl::hash_table<MyPair, MyPairHash, MyPairEqual,
                                    std::allocator<MyPair>>;

  MyTable ht{MyPairHash(), MyPairEqual()};
  ht.rehash_unique(10);
  std::cout << "哈希表已创建。初始桶数量: " << ht.bucket_count() << std::endl;

  std::cout << "\n1. 插入元素..." << std::endl;
  ht.insert_unique({1, "苹果"});
  ht.insert_unique({2, "香蕉"});
  ht.insert_unique({10, "橙子"});
  ht.insert_unique({15, "葡萄"});

  auto result = ht.insert_unique({2, "樱桃"});
  if (!result.second) {
    std::cout << "{2, \"樱桃\"} 插入失败：键已存在。" << std::endl;
  }
  std::cout << "当前大小: " << ht.size() << std::endl;

  std::cout << "\n2. 查找元素 (使用整数键)..." << std::endl;
  auto it = ht.find(10); // 现在可以直接用 int 查找
  if (it != ht.end()) {
    std::cout << "找到键 10，值为: " << it->value << std::endl;
  }

  auto it_missing = ht.find(99);
  if (it_missing == ht.end()) {
    std::cout << "键 99 未找到，符合预期。" << std::endl;
  }

  std::cout << "\n3. 删除元素 (使用整数键)..." << std::endl;
  size_t erased_count = ht.erase_unique(2); // 现在可以直接用 int 删除
  if (erased_count > 0) {
    std::cout << "已删除键 2。删除的元素数量: " << erased_count << std::endl;
  }
  std::cout << "当前大小: " << ht.size() << std::endl;

  std::cout << "\n4. 遍历剩余元素..." << std::endl;
  for (const auto &elem : ht) {
    std::cout << "  - 键: " << elem.key << ", 值: " << elem.value << std::endl;
  }

  std::cout << "\n--- 示例结束 ---" << std::endl;
  return 0;
}
