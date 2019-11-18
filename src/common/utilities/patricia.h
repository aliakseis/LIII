#pragma once

#include <memory>
#include <utility>

template <typename T>
class PatriciaTrie
{
public:
    struct TrieNode
    {
        const int  branch_bit;   // Stores index of next Bit to compare
        TrieNode*  children[2] {};  // left and right child

        TrieNode(std::unique_ptr<char[]> k, const T* v, int b = 0)
            : branch_bit(b)
            , key(std::move(k))
            , value(v ? std::make_unique<T>(*v) : nullptr)
        {
        }

        const char* get_key() { return key.get(); }
        const T* get_value()  { return value.get(); }

    private:
        std::unique_ptr<char[]> key;
        std::unique_ptr<T> value;
    };

private:

    enum
    {
        WORD_BITS = 8,                // Number of Bits in a Block 
        HI_WORD = WORD_BITS - 1,    // Hi-order bit, starting count from 0 
        BIT_MASK = WORD_BITS - 1,    // WORD SHIFT low-order bits enabled 
    };

    // Returns true if bit is set.
    template <typename C>
    static bool is_bit_set(C block, int shift) { return (block & ((1 << HI_WORD) >> shift)) != 0; }

    // Root for the entire Patricia tree, (actually a dummy header!). 
    mutable TrieNode* root;

    // Recursively free tree memory via modified post-order traversal. 
    void dispose_trie(TrieNode* root)
    {
        if (root->children[0]->branch_bit > root->branch_bit)
        {
            dispose_trie(root->children[0]);
        }
        if (root->children[1]->branch_bit > root->branch_bit)
        {
            dispose_trie(root->children[1]);
        }

        delete root;
    }

    void init() const
    {
        if (0 == root)
        {
            std::unique_ptr<char[]> empty(new char[1]);
            empty[0] = 0;
            root = new TrieNode(std::move(empty), nullptr);
            root->children[0]  = root;
            root->children[1] = root;
        }
    }

public:
    PatriciaTrie()
        : root(0)
    {
    }

    ~PatriciaTrie()
    {
        dispose();
    }

    template<typename C>
    TrieNode* find(const C* key, size_t key_len = 0, TrieNode** ret_parent = 0) const
    {
        init();

        TrieNode* parent;
        TrieNode* cur   = root->children[0]; // Root is *always* a dummy node, skip it 
        const C*      block = key;          // Pointer to Current Block of Bits 
        int        lower = 0;

        if (key_len != 0)
        {
            const C* end = block + key_len;
            do
            {
                parent  = cur;
                int bit = cur->branch_bit - lower;

                if (bit >= WORD_BITS)

                    while (lower + WORD_BITS <= cur->branch_bit)
                    {
                        lower += WORD_BITS;
                        ++block;
                        bit -= WORD_BITS;
                    }

                cur = cur->children[(block == end) ? 0 : is_bit_set(*block, bit)];
            }
            while (parent->branch_bit < cur->branch_bit);
        }
        else
            do
            {
                parent  = cur;
                int bit = cur->branch_bit - lower;

                if (bit >= WORD_BITS)

                    while (lower + WORD_BITS <= cur->branch_bit)
                    {
                        lower += WORD_BITS;
                        ++block;
                        bit -= WORD_BITS;
                    }

                cur = cur->children[is_bit_set(*block, bit)];
            }
            while (parent->branch_bit < cur->branch_bit);

        if (ret_parent != 0)
        {
            *ret_parent = parent;
        }

        return cur;                   // Let calling routine worry whether Keys matched! 
    }

    TrieNode* insert(const T& v, const char* key, size_t key_len = 0)
    {
        TrieNode* parent;
        TrieNode* cur = find(key, key_len, &parent);

        // Exclude duplicates 
        if ((0 == key_len) ? 0 == strcmp(key, cur->get_key())
                : (0 == strncmp(key, cur->get_key(), key_len) && 0 == cur->get_key()[key_len]))
        {
            return cur;
        }

        if (0 == key_len)
        {
            key_len = strlen(key);
        }
        std::unique_ptr<char[]> szKey(new char[key_len + 1]);
        memcpy(szKey.get(), key, key_len);
        szKey[key_len] = 0;

        key = szKey.get();

        const char* key_block = key;
        const char* cur_block = cur->get_key();
        int first_bit_diff;

        // Find the first word where Bits differ, skip common prefixes. 

        for (first_bit_diff = 0;
                *cur_block == *key_block;
                first_bit_diff += WORD_BITS)
        {
            cur_block++, key_block++;
        }

        // Now, find location of that Bit, xor does us a favor here. 

        for (int bit = *cur_block ^ *key_block;
                !(bit & (1 << HI_WORD));
                bit <<= 1)
        {
            first_bit_diff++;
        }

        // *Not* adding at a leaf node 
        if (parent->branch_bit > first_bit_diff)
        {
            // This is almost identical to the original Find above, however, we are 
            // guaranteed to end up at an internal node, rather than a leaf. 

            int lower = 0;
            cur = root;
            cur_block = key;
            do
            {
                parent  = cur;
                int bit = cur->branch_bit - lower;

                if (bit >= WORD_BITS)

                    while (lower + WORD_BITS <= cur->branch_bit)
                    {
                        lower += WORD_BITS;
                        ++cur_block;
                        bit -= WORD_BITS;
                    }

                cur = cur->children[is_bit_set(*cur_block, bit)];
            }
            while (cur->branch_bit < first_bit_diff);
        }

        auto t = new TrieNode(std::move(szKey), &v, first_bit_diff);

        // Key_Block was saved from above, this avoids some costly recomputation. 
        if (is_bit_set(*key_block, first_bit_diff & BIT_MASK))
        {
            t->children[0] = cur;
            t->children[1] = t;
        }
        else
        {
            t->children[0] = t;
            t->children[1] = cur;
        }

        // Determine whether the new node goes to the left or right of its parent 
        const char* block = key + (parent->branch_bit / WORD_BITS);

        parent->children[is_bit_set(*block, parent->branch_bit & BIT_MASK)] = t;

        return t;
    }

    // Frees all dynamic memory in Patricia Trie 
    void dispose()
    {
        if (root != 0)
        {
            TrieNode* left = root->children[0];
            dispose_trie(left);
            if (root != left)
            {
                delete root;
            }

            root = 0;
        }
    }
};
