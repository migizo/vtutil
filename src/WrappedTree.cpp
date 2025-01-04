/*
  ==============================================================================

    WrappedTree.cpp
    Author:  migizo

  ==============================================================================
*/

#include "WrappedTree.h"

namespace vtutil
{

//==============================================================================
void WrappedTree::wrapOrCreate(juce::ValueTree targetTree, const juce::Identifier& targetId, juce::UndoManager* um)
{
    jassert(targetId.isValid());

    undoManager = um;
    typeId = targetId;
    
    resetTree(targetTree);
}

void WrappedTree::resetTree(juce::ValueTree targetTree)
{
    // 空の場合は新規作成する
    if (targetTree.isValid() == false)
    {
        targetTree = juce::ValueTree(typeId);
        valueTree = targetTree;
    }
    // Type有効であればラップする
    else if (targetTree.hasType(typeId))
    {
        valueTree = targetTree;
    }
    // Type無効な場合はType有効な子を探し見つかればラップする
    // Type有効な子が無ければ新規作成する
    else
    {
        valueTree = targetTree.getOrCreateChildWithName(typeId, undoManager);
    }
    
    jassert(targetTree.isValid() && valueTree.isValid());
    wrapPropertiesAndChildren();
}

void WrappedTree::deepCopyFrom(WrappedTree* copySource)
{
//    auto vtParent = valueTree.getParent();
//    if (vtParent.isValid()) vtParent.removeChild(valueTree, undoManager);
    auto vtNew = copySource->getValueTree().createCopy();
    jassert(valueTree.getType() == vtNew.getType());
//    valueTree = vtNew;
//    vtParent.appendChild(valueTree, undoManager);
//    resetTree(valueTree);
    valueTree.copyPropertiesAndChildrenFrom(vtNew, undoManager);
    jassert(valueTree.isValid());
    wrapPropertiesAndChildren();
}

bool WrappedTree::isValid() const
{
    return valueTree.isValid() && typeId.isValid() && valueTree.hasType(typeId);
}


} // vtutil
