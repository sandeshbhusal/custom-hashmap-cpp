Sure! here is a well-crafted ... wait a minute. This is not written by GPT! 100% organic. 100% stupid.
This is a tale of consistently falling down rabbit holes, and my love for high-perf code for which I have
been constantly nerd-sniped, be it with or without my consent.

This was actually going to be a section in a much bigger blog post. But I looked at my analytics and I figured
that people don't stick around for more than 3 minutes on the post. Welp. That means no one is reading 3000 words I write.
Which means I need smaller blog posts -- or an audience with a longer attention span. Considering myself to be a person
who struggles with the latter these days - a lot - I guess the first is the way to go.

## Mission, vision.

Our mission and vision this time around is to write a Map. A very performant one, tailored to our specific needs. For this,
we have everything at our disposal - alas - except any third party libraries. Which means, we get a C++20 compiler, and
hopes and dreams. Oh and also, we get to peek at the data. At least at the top 20% of the total actionable data anyways (I 
did this to prevent myself from writing a generic solution).

We have a file with 1 billion rows ([oh boy here we go again](insert link here)). The task is to create a hashmap, so that, for each row that contains a city(string);measurement(floating point number - 1 digit precision), we add the measurement to the hashmap. Just to make things easier, we are allowed to work on a dataset for now, with 1 billion rows, but each time we write to the hashmap, the previous value for the city can be overwritten (so that I can solely focus on the task of optimizing the map, instead of struggling with scaled ints).

TL;DR: Have 1 billion pairs (city, number), add all pairs to map, overwrite (city, number_old) if (city, number_old) already in map. No deletions, no rebalancing.

## Baseline

Every good performance story needs a good baseline. Fortunately, for me, the task of generating the dataset and the baseline can be done in a single benchmark binary in C++.

```c++
#ifndef BASELINE 
#define BASELINE 

#include <forward_list>
#include <vector>

template<typename K, typename V>
class HashMap {
	using Entry = std::pair<K, V>;

	std::vector<std::forward_list<Entry>> m_store;
	size_t m_capacity;
	size_t m_count;

	private:
	inline size_t hash_key(const K& key) const {
		return std::hash<K>{}(key);
	}

	inline size_t find_slot(const K& key) const {
		return hash_key(key) % m_capacity;
	}

	public:
	HashMap(size_t capacity) {
		m_store = std::vector<std::forward_list<Entry>>(capacity);
		m_capacity = capacity;
		m_count = 0;
	}
	HashMap() = delete;
	HashMap(const HashMap&) = delete;
	HashMap(HashMap&&) = delete;
	HashMap operator=(const HashMap&) = delete;
	HashMap operator=(HashMap&&) = delete;

	void insert(const K& key, V value) {
		size_t index = find_slot(key);
		auto& chain = m_store[index];
		for (auto& entry: chain) {
			if (entry.first == key) {
				entry.second = std::move(value);
				return;
			}
		}

		chain.push_front({key, std::move(value)});
		m_count += 1;
	}

	V* get_value(const K& key) const {
		size_t index = find_slot(key);
		auto& chain = m_store[index];
		for (auto& item: chain) {
			if (item.first == key) {
				return item.second;
			}
		}
		return nullptr;
	}

	size_t size() const {
		return m_count;
	}
};

#endif
```

This is a pretty-standard chained hashmap implementation. The Stdlib does it in a similar way, but of course, we don't have a lot of things the stdlib has underneath the hood. But as a baseline, this will suffice. Also, remember that there is no need to "read" the data per-se. We don't call the "get_value" function in the benchmarks. It's just there to demonstrate how it works (for now).

The performance, is strikingly similar to the stdmap! I ran this map and the stdlib map on a workload of size 64000, 512000, 4096000 and finally 32768000. In fact, this code is _slightly_ faster than stdmap itself across all workloads. Here's the log-scaled results:

![](../results/baseline%20vs%20stdmap%20time.png)

Welp, we're there. We are already faster than stdmap. But there's a lot more to do, still. Now, we delve into the nitty-gritty details and start to actually optimize this code according to our requirements.

## Profiling

In order to profile the code, I used `perf`. All measurements are done on the biggest workload, i.e. 32768000 lines.

```
Performance counter stats for './build/bench 32768000 --benchmark_filter=TestBaseline':

           2591.49 msec task-clock                       #    0.999 CPUs utilized
                62      context-switches                 #   23.924 /sec
                42      cpu-migrations                   #   16.207 /sec
            542543      page-faults                      #  209.356 K/sec
       20652537536      instructions                     #    1.52  insn per cycle
                                                  #    0.21  stalled cycles per insn     (35.70%)
       13556022973      cycles                           #    5.231 GHz                         (35.74%)
        4403523394      stalled-cycles-frontend          #   32.48% frontend cycles idle        (35.77%)
        4372389105      branches                         #    1.687 G/sec                       (35.78%)
         209661059      branch-misses                    #    4.80% of all branches             (35.79%)
        8710648078      L1-dcache-loads                  #    3.361 G/sec                       (35.77%)
         291661279      L1-dcache-load-misses            #    3.35% of all L1-dcache accesses   (35.72%)
        1440164607      L1-icache-loads                  #  555.728 M/sec                       (35.70%)
            988414      L1-icache-load-misses            #    0.07% of all L1-icache accesses   (35.68%)
          47283668      dTLB-loads                       #   18.246 M/sec                       (35.67%)
           2523791      dTLB-load-misses                 #    5.34% of all dTLB cache accesses  (35.67%)
             16501      iTLB-loads                       #    6.367 K/sec                       (35.68%)
            411102      iTLB-load-misses                 # 2491.38% of all iTLB cache accesses  (35.66%)
          78665738      L1-dcache-prefetches             #   30.355 M/sec                       (35.67%)

       2.593698113 seconds time elapsed

       1.766003000 seconds user
       0.826533000 seconds sys
```

This is the perf data I get. From Mike Kulukundis' talk, as he says, first focus on cache and then we can focus on decreasing the instructions' count. At this point, the iTLB performance is horrible. We are missing a lot of the instructions.
