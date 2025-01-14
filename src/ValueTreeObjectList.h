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

namespace vtutil
{

// TODO: vt順との同期
// TODO: vt.addListener処理
template <typename WrappedTreeType>
class WrappedTreeList
: public WrappedTree // TODO: 継承しなくて良い
, public juce::ValueTree::Listener
{
public:
    WrappedTreeList() {}
    ~WrappedTreeList() override { clear(); }
    
    void wrapPropertiesAndChildren() override;
        
    WrappedTreeType* add(WrappedTreeType* t);
    void remove(WrappedTreeType* t);
    void clear() { valueTree.removeAllChildren(nullptr); }
    
    // TODO: getOwnedArray()などに名前を変える,もしくはiterator対応や[]を用意してこの関数を消す
    const juce::OwnedArray<WrappedTreeType>& getChildren() const { return children; }
    
protected:
    virtual juce::Identifier getTargetChildId() const = 0;
    virtual WrappedTreeType* createNewChild(juce::ValueTree& targetChild) const
    {
        auto newPtr = new WrappedTreeType();
        newPtr->wrap(targetChild, getTargetChildId(), undoManager);
        return newPtr;
    }
    
    juce::OwnedArray<WrappedTreeType> children;
    
private:
    void valueTreeChildAdded(juce::ValueTree& parent, juce::ValueTree& childWhichHasBeenAdded) override;
    void valueTreeChildRemoved(juce::ValueTree& parent, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override;
    void valueTreeChildOrderChanged(juce::ValueTree& parent, int oldIndex, int newIndex) override;
    
    void valueTreeRedirected(juce::ValueTree& parent) override
    {        
        jassertfalse;

        children.clear();
        for (auto vt: valueTree)
        {
            children.add(createNewChild(vt));
        }
    }
    
    bool ignoreCallback = false;
};

template <typename WrappedTreeType>
void WrappedTreeList<WrappedTreeType>::wrapPropertiesAndChildren()
{
    jassert(getTargetChildId().isValid());
    
    children.clear();
    for (auto vt: valueTree)
    {
        children.add(createNewChild(vt));
    }
}

template <typename WrappedTreeType>
WrappedTreeType* WrappedTreeList<WrappedTreeType>::add(WrappedTreeType* t)
{
    jassert(isValid());
    jassert(t != nullptr);
    juce::ScopedValueSetter<bool> svs(ignoreCallback, true);
    
    if (! t->isValid())
    {
        auto vtNewChild = juce::ValueTree(getTargetChildId());
        valueTree.appendChild(vtNewChild, undoManager);
        t->wrap(vtNewChild, getTargetChildId(), undoManager);
    }
    
    return children.add(t);
}

template <typename WrappedTreeType>
void WrappedTreeList<WrappedTreeType>::remove(WrappedTreeType* t)
{
    jassert(t != nullptr);
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
void WrappedTreeList<WrappedTreeType>::valueTreeChildAdded(juce::ValueTree& parent, juce::ValueTree& childWhichHasBeenAdded)
{
    if (ignoreCallback) return;
    if (parent != valueTree) return;
    
    auto ptr = createNewChild(childWhichHasBeenAdded);
    children.add(ptr);
}

template <typename WrappedTreeType>
void WrappedTreeList<WrappedTreeType>::valueTreeChildRemoved(juce::ValueTree& parent, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved)
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

} // namespace vtutil
