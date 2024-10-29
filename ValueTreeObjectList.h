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

template <typename WrappedTreeType>
class WrappedTreeList
: public WrappedTree
, public juce::ValueTree::Listener
{
public:
    WrappedTreeList() : WrappedTreeList(nullptr) {};
    WrappedTreeList(std::function<WrappedTreeType*()> creator) : createCallback(creator){}
    ~WrappedTreeList() override { clear(); }
    
    void wrapPropertiesAndChildren() override
    {
        jassert(targetChildId.isValid());
        
        children.clear();
        for (auto vt: valueTree)
        {
            children.add(createChildWithTree(vt));
        }
    }
        
    void add(WrappedTreeType* t)
    {
        jassert(isValid());
        jassert(t != nullptr);
        juce::ScopedValueSetter<bool> svs(ignoreCallback, true);
        
        if (! t->isValid())
        {
            auto vtNewChild = juce::ValueTree(targetChildId);
            valueTree.appendChild(vtNewChild, undoManager);
            t->wrapOrCreate(vtNewChild, targetChildId, undoManager);
        }
        
        children.add(t);
    }
    
    void remove(WrappedTreeType* t)
    {
        jassert(t != nullptr);
        int index = children.indexOf(t);
        
        if (index > 0)
        {
            valueTree.removeChild(index, nullptr);
        }
        else jassertfalse;
    }
    
    void clear()
    {
        valueTree.removeAllChildren(nullptr);
    }
    
    const juce::OwnedArray<WrappedTreeType>& getChildren() const { return children; }
    
protected:
    juce::OwnedArray<WrappedTreeType> children;
    juce::Identifier targetChildId;
    
private:
    void valueTreeChildAdded(juce::ValueTree& parent, juce::ValueTree& childWhichHasBeenAdded) override
    {
        if (ignoreCallback) return;
        if (parent != valueTree) return;
        
        auto ptr = createChildWithTree(childWhichHasBeenAdded);
        children.add(ptr);
    }
    
    void valueTreeChildRemoved(juce::ValueTree& parent, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override
    {
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
            children.add(createChildWithTree(vt));
        }
    }
    
    WrappedTreeType* createChildWithTree(juce::ValueTree& targetChild)
    {
        if (createCallback)
            return createCallback();

        auto newPtr = new WrappedTreeType();
        newPtr->wrapOrCreate(targetChild, targetChildId, undoManager);
        return newPtr;
    }
    
    bool ignoreCallback = false;
    std::function<WrappedTreeType*()> createCallback = nullptr;

};

} // namespace vtutil
