diff --git a/bench.cc b/bench.cc
index 448762a..e28d7d8 100644
--- a/bench.cc
+++ b/bench.cc
@@ -50,6 +50,9 @@ void test_custom_map(benchmark::State &state) {
         }
     }
 
+    // Dump stats.
+    printf("PSL min: %ld\n", my_map.get_psl_min());
+    printf("PSL max: %ld\n", my_map.get_psl_max());
     benchmark::DoNotOptimize(my_map);
 }
 
diff --git a/custom_map.hpp b/custom_map.hpp
index 1435c29..02ffd3b 100644
--- a/custom_map.hpp
+++ b/custom_map.hpp
@@ -1,8 +1,10 @@
 #ifndef CUSTOM_MAP
 #define CUSTOM_MAP
 
+#include <algorithm>
 #include <functional>
 #include <stdexcept>
+#include <limits>
 
 template <typename K, typename V> struct Slot {
     bool occupied = false;
@@ -26,6 +28,9 @@ static inline constexpr size_t nearest_power_of_two(size_t number) {
 template <typename K, typename V, const size_t __min_slots = 1> class MyMap {
   private:
     Slot<K, V> slots[nearest_power_of_two(__min_slots)];
+    size_t psl_min = std::numeric_limits<size_t>::max();  // Initialize to max value so any real PSL will be smaller
+    size_t psl_max = 0;
+    size_t occupied_buckets = 0;
 
     constexpr size_t capacity() const {
         return nearest_power_of_two(__min_slots);
@@ -37,11 +42,22 @@ template <typename K, typename V, const size_t __min_slots = 1> class MyMap {
         size_t bucket = hash & (capacity() - 1);
         size_t count = 0;
         while (count < capacity()) {
-            if (!slots[bucket].occupied)
+            if (!slots[bucket].occupied) {
+                // Found empty slot - update PSL stats and mark as occupied
+                psl_min = std::min(psl_min, count);
+                psl_max = std::max(psl_max, count);
+                if (slots[bucket].occupied) {
+                }
+                occupied_buckets += 1;
                 return slots[bucket];
+            }
             auto &candidate = slots[bucket];
-            if (candidate.key == key)
+            if (candidate.key == key) {
+                // Found existing key - update PSL stats
+                psl_min = std::min(psl_min, count);
+                psl_max = std::max(psl_max, count);
                 return candidate;
+            }
             count += 1;
             bucket = (bucket + 1) & (capacity() - 1);
         }
@@ -55,6 +71,15 @@ template <typename K, typename V, const size_t __min_slots = 1> class MyMap {
         slot.key = key;
         slot.occupied = true;
     }
+
+    size_t get_psl_min() const {
+        if (occupied_buckets == 0) return 0;
+        return psl_min;
+    }
+
+    size_t get_psl_max() const {
+        return psl_max;
+    }
 };
 
 #endif
\ No newline at end of file
