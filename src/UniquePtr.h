/*
  ==============================================================================

    UniquePtr.h
    Author:  migizo

  ==============================================================================
*/

#pragma once
#include <juce_data_structures/juce_data_structures.h>
#include "WrappedTree.h"

namespace vtutil
{

//! @brief juce::ValueTreeと同期可能なユニークポインタ
//! WrappedTreeType型はvtutil::WrappedTreeの派生クラスである必要がある。
//! 対象のtreeをリッスンし、有効無効状態および親への追加削除に応じてunique_ptrを同期させる

template <typename WrappedTreeType>
class UniquePtr
: private juce::ValueTree::Listener
{
    static_assert(std::is_base_of<WrappedTree, WrappedTreeType>::value == true,
                  "template parameter must be derived from vtutil::WrappedTree");
    
public:
    UniquePtr() : UniquePtr(nullptr) {};
    UniquePtr(std::function<WrappedTreeType*()> creator)
    : createCallback(creator){}
    ~UniquePtr() = default;
    
    UniquePtr<WrappedTreeType>& operator=(nullptr_t) noexcept { reset(nullptr); return *this; }
    const WrappedTreeType& operator*() const { jassert(ptr); return *ptr.get(); }
    WrappedTreeType* const operator->() const noexcept { return ptr.get(); }
    explicit operator bool() const noexcept { return (bool)ptr; }
    bool operator== (const UniquePtr<WrappedTreeType>& other) const { return ptr == other.ptr; }
    bool operator!= (const UniquePtr<WrappedTreeType>& other) const { return ! operator== (other); }

    void listen(const juce::ValueTree& parent, const juce::Identifier& ids, juce::UndoManager* um)
    {
        jassert(parent.isValid());
        
        juce::ScopedValueSetter<bool> svs(ignoreCallback, true);

        targetTree.removeListener(this);
        targetParent = parent;
        targetId = ids;
        undoManager = um;
        targetTree = targetParent.getChildWithName(targetId);
        updatePtrWithTree();
        targetTree.addListener(this);
    }
    
    void reset(WrappedTreeType* t)
    {
        juce::ScopedValueSetter<bool> svs(ignoreCallback, true);

        //------------------
        // valueTreeの更新
        //------------------
        if (t == nullptr)
        {
            if (targetTree.isAChildOf(targetParent))
                targetParent.removeChild(targetTree, undoManager);
        }
        else
        {
            // 初期化前なら初期化する
            if (t->isValid() == false)
            {
                t->wrapOrCreate(targetParent, targetId, undoManager);
            }
                
            // targetTree更新
            if (targetTree.isValid())
                targetTree.copyPropertiesAndChildrenFrom(t->getValueTree(), undoManager);
            else
                targetTree = t->getValueTree();
            
            // 有効な子がない場合に追加する
            if (! targetTree.isAChildOf(targetParent))
                targetParent.appendChild(targetTree, undoManager);
        }

        //------------------
        // ptrの更新
        //------------------
        ptr.reset(t);
    }
    
    WrappedTreeType const* get() const { return ptr.get(); }
    
private:
    void valueTreeParentChanged(juce::ValueTree& treeWhoseParentHasChanged)
    {
        if (ignoreCallback) return;
        if (targetTree != treeWhoseParentHasChanged) return;
        
        updatePtrWithTree();
    }
    
    void valueTreeRedirected(juce::ValueTree& treeWhichHasBeenChanged)
    {
        if (ignoreCallback) return;
        if (targetTree != treeWhichHasBeenChanged) return;
        // listen時(でのlistener解除時)以外にリダイレクト(valueTreeに対する「=」を使用した参照先変更処理)は行わない想定
        jassertfalse;
        
        listen(targetParent, targetId, undoManager);
    }
    
    void updatePtrWithTree()
    {
        auto targetChild = targetParent.getChildWithName(targetId);
        if (targetChild.isValid() == false || targetChild.isAChildOf(targetParent) == false)
            ptr = nullptr;
        else
        {
            WrappedTreeType* newPtr = nullptr;
            if (createCallback)
            {
                newPtr = createCallback();
            }
            else
            {
                newPtr = new WrappedTreeType();
                newPtr->wrapOrCreate(targetChild, targetId, undoManager);
            }
            ptr.reset(newPtr);
        }
    }
    
    std::function<WrappedTreeType*()> createCallback = nullptr;
    std::unique_ptr<WrappedTreeType> ptr;
    juce::ValueTree targetParent;
    juce::ValueTree targetTree;
    juce::Identifier targetId;
    juce::UndoManager* undoManager;
    bool ignoreCallback = false;
};

} // namespace vtutil
