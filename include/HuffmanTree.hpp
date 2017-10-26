#ifndef HUFFMAN_TREE_HPP
#define HUFFMAN_TREE_HPP

#include <string>
#include <memory>

#include "Types.hpp"

namespace kpeg
{
    struct Node
    {
        Node() :
         root{ false } ,
         leaf{ false } ,
         code{ "" } ,
         value{ 0x00 } ,
         lChild{ nullptr } ,
         rChild{ nullptr } ,
         parent{ nullptr }
        {}
        
        Node( const std::string _code, const UInt16 _val ) :
         root{ false } ,
         leaf{ false } ,
         code{ _code } ,
         value{ _val } ,
         lChild{ nullptr } ,
         rChild{ nullptr } ,
         parent{ nullptr }
        {}
        
        bool root;    // Only the root node is set to true
        bool leaf;    // Only the leaves with a Huffman code value are set to true
        std::string code; // The bitstream binary code (e.g., 1101011...)
        UInt16 value; // The Huffman code (e.g., 0xC3)
        std::shared_ptr<Node> lChild, rChild; // The left & right children of the node
        std::shared_ptr<Node> parent; // Parent of the node, makes it easier to traverse backwards in the tree
    };
    
    typedef std::shared_ptr<Node> NodePtr;
    
    /** Node helpers */
    
    inline NodePtr createRootNode( const UInt16 value )
    {
        NodePtr root = std::make_shared<Node>( "", value );
        root->root = true;
        return root;
    }
    
    inline NodePtr createNode()
    {
        return std::make_shared<Node>();
    }
    
    void insertLeft( NodePtr node, const UInt16 value );
    
    void insertRight( NodePtr node, const UInt16 value );
    
    NodePtr getRightLevelNode( NodePtr node );
    
    void inOrder( NodePtr node );
    
    /**
     * @brief HuffmanTree is an abstract way to manage the binary tree constructed from the specified Huffman table.
     * 
     * This class abstracts away the algorithm for generating the Huffman
     * binary tree for the Huffman tables found in the JFIF file.
     */
    
    class HuffmanTree
    {
        public:
            
            HuffmanTree();
            
            HuffmanTree( const HuffmanTable& htable );
            
            void constructHuffmanTree( const HuffmanTable& htable );
            
            const NodePtr getTree() const;
            
            /**
             * @brief Checks whether a given Huffman code is present in the tree or not.
             */
            
            // NOTE: std::string is used as the return type because 0x0000 and 0xFFFF
            // are both values that are used in the normal range. So using them is not 
            // possible to indicate special conditions (e.g., code not found in tree).
            const std::string contains( const std::string& huffCode );
            
        private:
            
            NodePtr m_root; // Root of the binary tree
    };
}

#endif // HUFFMAN_TREE_HPP
