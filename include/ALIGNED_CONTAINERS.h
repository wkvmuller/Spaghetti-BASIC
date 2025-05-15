#ifndef ALIGNED_CONTAINERS_H
#define ALIGNED_CONTAINERS_H

#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <cstddef>  // for std::max_align_t

// This header defines a struct whose standard containers are aligned to 16-byte boundaries.
// You can adjust the alignment value (bytes) as needed.

struct AlignedContainers {
    // Align each container member on a 16-byte boundary:
    alignas(16) std::vector<int>             vec;
    alignas(16) std::map<int, std::string>   mp;
    alignas(16) std::unordered_map<int, std::string> um;

    // You can add more aligned containers here:
    // alignas(16) std::deque<double> dq;
    // alignas(16) std::list<float>  lst;
};

#endif // ALIGNED_CONTAINERS_H
