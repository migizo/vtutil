/*
  ==============================================================================

    ValueTreeObjectList.h
    Author:  migizo

  ==============================================================================
*/

#pragma once
#include <juce_data_structures/juce_data_structures.h>
#include "WrappedTree.h"
//#include "../ValueTreeConverter.h"

namespace vtwrapper
{

// TODO: Listenerおよびhoge(WrappedTreeList<WrappedTreeType> changedPtr)を用意、もしくはstd::function
template <typename WrappedTreeType>
class WrappedTreeList
: protected juce::ValueTree::Listener
{
public:
    WrappedTreeList() = default;
    ~WrappedTreeList() override { clear(); }
    
    void wrap(const juce::ValueTree& targetTree, const juce::Identifier& targetParentType, const juce::Identifier& targetChildType, juce::UndoManager* um, bool allowCreationIfInvalid = true, bool allowChildWrapping = true);
    
    WrappedTreeType* add(WrappedTreeType* t);
    void remove(WrappedTreeType* t);
    void clear() { valueTree.removeAllChildren(nullptr); }
    
    template<typename ElementComparator>
    void sort(ElementComparator& comparator, bool retainOrderOfEquivalentItems = false) noexcept;
    
    bool isEmpty() const { return children.isEmpty(); }
    int size() const { return children.size(); }
    
    inline WrappedTreeType* const getUnchecked(int index) const noexcept { return children[index]; }
    inline WrappedTreeType* const operator[](int index) const noexcept { return getUnchecked(index); }
    inline WrappedTreeType* getFirst() const noexcept { return children.getFirst(); }
    inline WrappedTreeType* getLast() const noexcept { return children.getLast(); }

    inline WrappedTreeType** begin() noexcept { return children.begin(); }
    inline WrappedTreeType* const* begin() const noexcept { return children.begin(); }
    inline WrappedTreeType** end() noexcept { return children.end(); }
    inline WrappedTreeType* const* end() const noexcept { return children.end(); }

    const juce::OwnedArray<WrappedTreeType>& getOwnedArray() const { return children; }
    
    //! @brief 既にwrap()による紐付け処理を行い有効な状態であるか
    bool isValid() const { return valueTree.isValid() && parentTypeId.isValid() && childTypeId.isValid() && valueTree.hasType(parentTypeId); }
    
    const juce::ValueTree& getValueTree() const noexcept { return valueTree; }
    const juce::Identifier& getParentTypeID() const noexcept { return parentTypeId; }
    const juce::Identifier& getChildTypeID() const noexcept { return childTypeId; }
    juce::UndoManager* getUndoManager() noexcept { return undoManager; }
    
protected:
    juce::OwnedArray<WrappedTreeType> children;
    
private:
    void valueTreeChildAdded(juce::ValueTree& parent, juce::ValueTree& childWhichHasBeenAdded) override;
    void valueTreeChildRemoved(juce::ValueTree& parent, juce::ValueTree& /*childWhichHasBeenRemoved*/, int indexFromWhichChildWasRemoved) override;
    void valueTreeChildOrderChanged(juce::ValueTree& parent, int oldIndex, int newIndex) override;
    
    WrappedTreeType* createNewChild(juce::ValueTree& targetChild) const;
    
    juce::ValueTree valueTree;
    juce::Identifier parentTypeId;
    juce::Identifier childTypeId;
    juce::UndoManager* undoManager;
    
    bool ignoreCallback = false;
};

template <typename WrappedTreeType>
void WrappedTreeList<WrappedTreeType>::wrap(const juce::ValueTree& targetTree, const juce::Identifier& targetParentType, const juce::Identifier& targetChildType, juce::UndoManager* um, bool allowCreationIfInvalid, bool allowChildWrapping)
{
    valueTree.removeListener(this);
    
    parentTypeId = targetParentType;
    childTypeId = targetChildType;
    undoManager = um;
    valueTree = targetTree;

    WrappedTree::updateTreeIfNeeded(valueTree, parentTypeId, undoManager, allowCreationIfInvalid, allowChildWrapping);
    
    children.clear();
    for (auto vt: valueTree)
    {
        children.add(createNewChild(vt));
    }
    
    valueTree.addListener(this);
}

template <typename WrappedTreeType>
WrappedTreeType* WrappedTreeList<WrappedTreeType>::add(WrappedTreeType* t)
{
    if (! isValid() || t == nullptr)
    {
        jassertfalse;
        return nullptr;
    }

    juce::ScopedValueSetter<bool> svs(ignoreCallback, true);
    
    if (! t->isValid())
    {
        auto vtNewChild = juce::ValueTree(childTypeId);
        valueTree.appendChild(vtNewChild, undoManager);
        t->wrap(vtNewChild, childTypeId, undoManager);
    }
    
    jassert(t->getTypeID() == childTypeId);
    
    return children.add(t);
}

template <typename WrappedTreeType>
void WrappedTreeList<WrappedTreeType>::remove(WrappedTreeType* t)
{
    if (! isValid() || t == nullptr || t->getTypeID() != childTypeId)
    {
        jassertfalse;
        return;
    }
    
    juce::ScopedValueSetter<bool> svs(ignoreCallback, true);

    int index = children.indexOf(t);
    if (index > 0)
    {
        valueTree.removeChild(index, nullptr);
    }
    else jassertfalse;
    
    children.removeObject(t);
}

template <typename WrappedTreeType>
template <typename ElementComparator>
void WrappedTreeList<WrappedTreeType>::sort(ElementComparator& comparator, bool retainOrderOfEquivalentItems) noexcept
{
    valueTree.sort(comparator, undoManager, retainOrderOfEquivalentItems);
}


template <typename WrappedTreeType>
void WrappedTreeList<WrappedTreeType>::valueTreeChildAdded(juce::ValueTree& parent, juce::ValueTree& childWhichHasBeenAdded)
{
    if (ignoreCallback) return;
    if (parent != valueTree) return;
    
    auto ptr = createNewChild(childWhichHasBeenAdded);
    children.add(ptr);
}

template <typename WrappedTreeType>
void WrappedTreeList<WrappedTreeType>::valueTreeChildRemoved(juce::ValueTree& parent, juce::ValueTree& /*childWhichHasBeenRemoved*/, int indexFromWhichChildWasRemoved)
{
    if (ignoreCallback) return;
    if (parent != valueTree) return;
    
    children.remove(indexFromWhichChildWasRemoved);
}

template <typename WrappedTreeType>
void WrappedTreeList<WrappedTreeType>::valueTreeChildOrderChanged(juce::ValueTree& parent, int oldIndex, int newIndex)
{
    if (parent != valueTree) return;
    
    children.move(oldIndex, newIndex);
}

template <typename WrappedTreeType>
WrappedTreeType* WrappedTreeList<WrappedTreeType>::createNewChild(juce::ValueTree& targetChild) const
{
    auto newPtr = new WrappedTreeType();
    newPtr->wrap(targetChild, childTypeId, undoManager);
    return newPtr;
}

} // namespace vtwrapper
