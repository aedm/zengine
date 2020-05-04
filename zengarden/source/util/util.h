#pragma once

#include <zengine.h>
#include <QtCore/QString>
#include <memory>
#include <vector>
#include <algorithm>

using std::map;
using std::set;
using std::vector;

namespace Util {

  /// Creates a shader pass
  std::shared_ptr<Pass> LoadShader(const char* VertexFile, const char* FragmentFile);
  std::shared_ptr<StubNode> LoadStub(const QString& fileName);

  /// Reads in a file using Qt
  OWNERSHIP char* ReadFileQt(const char* FileName);
  OWNERSHIP char* ReadFileQt(const QString& FileName);

  /// Loads a static mesh from a Wavefront .obj file
  std::shared_ptr<Mesh> LoadMesh(const QString& fileName);

  /// Disposes a set of nodes
  void DisposeNodes(const std::set<std::shared_ptr<Node>>& nodes);

  /// Calculate removed and and newly added items
  template<typename K, typename V>
  void CollectChanges(const map<K, V>& itemMap, const vector<K>& newItems, 
    vector<K>& oNewItems, vector<K>& oDeletedItems) {
    /// Collect new items
    for (const K& key: newItems) {
      if (itemMap.find(key) == itemMap.end()) {
        oNewItems.push_back(key);
      }
    }

    /// Collect deleted items, assumes a small set of newItems
    for (auto const& [key, value] : itemMap) {
      if (std::find(newItems.begin(), newItems.end(), key) == newItems.end()) {
        oDeletedItems.push_back(key);
      }
    }
  }
}
