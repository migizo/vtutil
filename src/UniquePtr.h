/*
  ==============================================================================

    UniquePtr.h
    Author:  migizo

  ==============================================================================
*/

#pragma once
#include <juce_data_structures/juce_data_structures.h>
#include "WrappedTree.h"

namespace vtwrapper
{

//! @brief juce::ValueTreeと同期可能なユニークポインタ
//! WrappedTreeType型はvtwrapper::WrappedTreeの派生クラスである必要がある。
//! 対象のtreeをリッスンし、有効無効状態および親への追加削除に応じてunique_ptrを同期させる
//! juce::ValueTree::isValid()では無い場合はnullptrを指す。
// TODO: ListenerおよびUniquePtrChanged(UniquePtr<WrappedTreeType> changedPtr)を用意、もしくはstd::function
template <typename WrappedTreeType>
class UniquePtr
: private juce::ValueTree::Listener
{
    static_assert(std::is_base_of<WrappedTree, WrappedTreeType>::value == true,
                  "template parameter must be derived from vtwrapper::WrappedTree");
    
public:
    UniquePtr() : UniquePtr(nullptr) {}
    UniquePtr(std::function<WrappedTreeType*()> creator) : createCallback(creator) {}
    ~UniquePtr() override = default;
    
    UniquePtr<WrappedTreeType>& operator=(nullptr_t) noexcept { reset(nullptr); return *this; }
    const WrappedTreeType& operator*() const { jassert(ptr); return *ptr.get(); }
    WrappedTreeType* const operator->() const noexcept { return ptr.get(); }
    explicit operator bool() const noexcept { return (bool)ptr; }
    bool operator== (const UniquePtr<WrappedTreeType>& other) const { return ptr == other.ptr; }
    bool operator!= (const UniquePtr<WrappedTreeType>& other) const { return ! operator== (other); }
    bool operator== (nullptr_t) const { return ptr == nullptr; }
    bool operator!= (nullptr_t) const { return ! operator== (nullptr); }
    
    // TODO: 親要素も追加できるようにし、その親が有効な場合にreset時に追加できるようにする
    //! 与えられたValueTreeのtargetTypeを参照するようにし、ValueTreeの状態に応じてポインタの有効状態を同期できるようにする
    //! 与えられたValueTreeに対しては以下の操作を行う。
    //! - targetTreeが有効かつtargetTypeと同じTypeを持つ場合...targetTreeおよびその親を保持する
    //! - targetTreeが有効かつtargetTypeと同じTypeを持たないが子が同じTypeを持つ場合...targetTreeを親としtargetTypeを持つ子も保持する
    //! - targetTreeが有効だがtargetTypeと同じTypeを持たず子も同じTypeを持たない場合...親Treeのみを保持する
    void referTo(const juce::ValueTree& targetTree, const juce::Identifier& targetType, juce::UndoManager* um);
        
    //! std::unique_ptr<>::reset()と同様、解放および新たなリソースの所有権を設定する
    //! 先にreferToによる紐付けを行なっている必要がある。
    //! 与えられたWrappedTree<>がnullではないがwrap()による初期化処理が行われていない場合はWrappedTreeの初期化を行うようする
    void reset(WrappedTreeType* t);
    
    void activate() { reset(true); }
    void deactivate() { reset(false); }

    WrappedTreeType const* get() const { return ptr.get(); }
    
private:
    void valueTreeParentChanged(juce::ValueTree& treeWhoseParentHasChanged) override;
    
    void reset(bool isOn);
    void updatePtrWithTree();
    
    std::function<WrappedTreeType*()> createCallback = nullptr;
    std::unique_ptr<WrappedTreeType> ptr = nullptr;
    juce::ValueTree parentTree;
    juce::ValueTree valueTree;
    juce::Identifier typeId;
    juce::UndoManager* undoManager;
    bool ignoreCallback = false;
};


template <typename WrappedTreeType>
void UniquePtr<WrappedTreeType>::referTo(const juce::ValueTree& targetTree, const juce::Identifier& targetType, juce::UndoManager* um)
{
    jassert(targetType.isValid());
    jassert(targetTree.isValid());

    juce::ScopedValueSetter<bool> svs(ignoreCallback, true);

    valueTree.removeListener(this);
    
    typeId = targetType;
    undoManager = um;
    
    if (targetType.isValid() && targetTree.isValid())
    {
        // target自身が有効なTypeを持つ場合
        if (targetTree.hasType(typeId))
        {
            parentTree = targetTree.getParent();
            valueTree = targetTree;
        }
        // 子が有効なTypeを持つ場合
        else if (targetTree.getChildWithName(typeId).isValid())
        {
            parentTree = targetTree;
            valueTree = targetTree.getChildWithName(typeId);
        }
        // 有効なValueTreeだが対象のTypeを持たない場合
        else
        {
            parentTree = targetTree;
            valueTree = {};
        }
    }
    // 無効なValueTreeの場合
    else
    {
        valueTree = {};
        parentTree = {};
    }
    
    updatePtrWithTree();
    valueTree.addListener(this);
}

template <typename WrappedTreeType>
void UniquePtr<WrappedTreeType>::reset(WrappedTreeType* t)
{
    juce::ScopedValueSetter<bool> svs(ignoreCallback, true);

    //------------------
    // valueTreeの更新
    //------------------
    if (t == nullptr)
    {
        if (valueTree.isAChildOf(parentTree))
            parentTree.removeChild(valueTree, undoManager);
    }
    else
    {
        // referToを呼んでいなかったら呼ぶ
        if (typeId.isValid() == false && t->isValid())
        {
            // 仕様としては既に呼ばれているべき
            jassertfalse;
            
            referTo(t->getValueTree(), t->getTypeID(), t->getUndoManager());
        }
        // WrappedTreeが初期化前なら初期化する
        else if (typeId.isValid() && t->isValid() == false)
        {
            t->wrap(parentTree, typeId, undoManager);
        }
            
        // targetTree更新
        valueTree = t->getValueTree();
        
        // 親に追加されていない場合に追加する
        if (! valueTree.isAChildOf(parentTree))
            parentTree.appendChild(valueTree, undoManager);
    }

    //------------------
    // ptrの更新
    //------------------
    ptr.reset(t);
}

template <typename WrappedTreeType>
void UniquePtr<WrappedTreeType>::reset(bool isOn)
{
    WrappedTreeType* newPtr = nullptr;

    if (isOn)
    {
        if (createCallback)
        {
            newPtr = createCallback();
        }
        else
        {
            newPtr = new WrappedTreeType();
            newPtr->wrap(valueTree, typeId, undoManager);
        }
    }
    reset(newPtr);
}

template <typename WrappedTreeType>
void UniquePtr<WrappedTreeType>::valueTreeParentChanged(juce::ValueTree& treeWhoseParentHasChanged)
{
    if (ignoreCallback) return;
    if (valueTree != treeWhoseParentHasChanged) return;
    
    updatePtrWithTree();
}

template <typename WrappedTreeType>
void UniquePtr<WrappedTreeType>::updatePtrWithTree()
{
    // 無効なValueTreeの場合はnullをセット
    if (valueTree.isValid() == false || valueTree.isAChildOf(parentTree) == false)
    {
        ptr = nullptr;
    }
    // 有効な場合はコールバックがあればそちらを呼び出し、無ければwrap処理を呼んだ後スマートポインタに格納する
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
            newPtr->wrap(valueTree, typeId, undoManager);
        }
        ptr.reset(newPtr);
    }
}


} // namespace vtwrapper
