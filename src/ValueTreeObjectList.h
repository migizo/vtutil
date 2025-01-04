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

// TODO: vt.addListener処理
template <typename WrappedTreeType>
class WrappedTreeList
: public WrappedTree
, public juce::ValueTree::Listener
{
public:
    WrappedTreeList() {}
    ~WrappedTreeList() override { clear(); }
    
    void wrapPropertiesAndChildren() override
    {
        jassert(getTargetChildId().isValid());
        
        children.clear();
        for (auto vt: valueTree)
        {
            children.add(createNewChild(vt));
        }
    }
        
    void add(WrappedTreeType* t)
    {
        jassert(isValid());
        jassert(t != nullptr);
        juce::ScopedValueSetter<bool> svs(ignoreCallback, true);
        
        if (! t->isValid())
        {
            auto vtNewChild = juce::ValueTree(getTargetChildId());
            valueTree.appendChild(vtNewChild, undoManager);
            t->wrapOrCreate(vtNewChild, getTargetChildId(), undoManager);
        }
        
        children.add(t);
    }
    
    void remove(WrappedTreeType* t)
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
    
    void clear()
    {
        valueTree.removeAllChildren(nullptr);
    }
    
    const juce::OwnedArray<WrappedTreeType>& getChildren() const { return children; }
    
protected:
    virtual juce::Identifier getTargetChildId() const = 0;
    virtual WrappedTreeType* createNewChild(juce::ValueTree& targetChild) const
    {
        auto newPtr = new WrappedTreeType();
        newPtr->wrapOrCreate(targetChild, getTargetChildId(), undoManager);
        return newPtr;
    }
    
    juce::OwnedArray<WrappedTreeType> children;
    
private:
    void valueTreeChildAdded(juce::ValueTree& parent, juce::ValueTree& childWhichHasBeenAdded) override
    {
        if (ignoreCallback) return;
        if (parent != valueTree) return;
        
        auto ptr = createNewChild(childWhichHasBeenAdded);
        children.add(ptr);
    }
    
    void valueTreeChildRemoved(juce::ValueTree& parent, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override
    {
        if (ignoreCallback) return;
        if (parent != valueTree) return;
        
        children.remove(indexFromWhichChildWasRemoved);
    }
    
    void valueTreeChildOrderChanged(juce::ValueTree& parent, int oldIndex, int newIndex) override
    {
        if (parent != valueTree) return;
        
        children.move(oldIndex, newIndex);
    }
    
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

} // namespace vtutil
