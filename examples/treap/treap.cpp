#include "sgcl/sgcl.h"

#include <chrono>
#include <iostream>

using namespace sgcl;

class treap {
    struct Node;
    using Root = root_ptr<Node>;
    using Ptr = tracked_ptr<Node>;
    using Ref = unsafe_ptr<Node>;

    struct Node {
        Node(int v): value(v) {}
        const int value;
        const int priority = rand();
        Ptr left;
        Ptr right;
    };

    Root _root;

    inline static auto _make = [](int value) {
        return make_tracked<Node>(value);
    };

public:
    void insert(int value) {
        auto [lower, equal, greater] = _split(value);
        if (!equal) {
            equal = _make(value);
        }
        _root = _merge(lower, equal, greater);
    }

    void erase(int value) {
        auto [lower, equal, greater] = _split(value);
        _root = _merge(lower, greater);
    }

    bool has_value(int value) {
        auto [lower, equal, greater] = _split(value);
        _root = _merge(lower, equal, greater);
        return equal != nullptr;
    }

private:
    static Ref _merge(Ref lower, Ref greater) {
        if (!lower) {
            return greater;
        }
        if (!greater) {
            return lower;
        }
        if (lower->priority < greater->priority) {
            lower->right = _merge(lower->right, greater);
            return lower;
        } else {
            greater->left = _merge(lower, greater->left);
            return greater;
        }
    }

    static Ref _merge(Ref lower, Ref equal, Ref greater) {
        return _merge(_merge(lower, equal), greater);
    }

    std::array<Root, 3> _split(int value) const {
        struct Local {
            static void split(Ref orig, Ptr& lower, Ptr& greater, int value) {
                if (!orig) {
                    lower = nullptr;
                    greater = nullptr;
                } else if (orig->value < value) {
                    lower = orig;
                    split(lower->right, lower->right, greater, value);
                } else {
                    greater = orig;
                    split(greater->left, lower, greater->left, value);
                }
            }
        };
        Root lower, equal, equal_or_greater, greater;
        Local::split(_root, lower, equal_or_greater, value);
        Local::split(equal_or_greater, equal, greater, value + 1);
        return {lower, equal, greater};
    }
};

int main() {
    using std::chrono::high_resolution_clock;
    using std::chrono::duration;
    auto t = high_resolution_clock::now();

    treap treap;
    int value = 5;
    int result = 0;

    for(int i = 1; i < 1000000; ++i) {
        value = (value * 57 + 43) % 10007;
        switch(i % 3) {
        case 0:
            treap.insert(value);
            break;
        case 1:
            treap.erase(value);
            break;
        case 2:
            result += treap.has_value(value);
            break;
        }
    }

    std::cout << result << std::endl;
    std::cout << duration<double, std::milli>(high_resolution_clock::now() - t).count() << "ms\n";
}
