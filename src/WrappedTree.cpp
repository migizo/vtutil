/*
  ==============================================================================

    WrappedTree.cpp
    Author:  migizo

  ==============================================================================
*/

#include "WrappedTree.h"

namespace vtwrapper
{

//==============================================================================
void WrappedTree::wrap(juce::ValueTree targetTree, const juce::Identifier& targetType, juce::UndoManager* um, bool allowCreationIfInvalid, bool allowChildWrapping)
{
    jassert(targetType.isValid());
    
    typeId = targetType;
    undoManager = um;
    valueTree = targetTree;

    updateTreeIfNeeded(valueTree, typeId, undoManager, allowCreationIfInvalid, allowChildWrapping);
    
    if (valueTree.isValid() == false)
    {
        jassertfalse;
        return;
    }
    wrapPropertiesAndChildren();
}

void WrappedTree::copyPropertiesAndChildrenFrom(const WrappedTree& copySource)
{
    if (isValid() == false || copySource.isValid() == false || getTypeID() != copySource.getTypeID())
    {
        jassertfalse;
        return;
    }
    
    valueTree.copyPropertiesAndChildrenFrom(copySource.getValueTree(), undoManager);
    wrapPropertiesAndChildren();
}

bool WrappedTree::isValid() const
{
    return valueTree.isValid() && typeId.isValid() && valueTree.hasType(typeId);
}

void WrappedTree::updateTreeIfNeeded(juce::ValueTree& targetTree, const juce::Identifier& targetType, juce::UndoManager* um, bool allowCreationIfInvalid, bool allowChildWrapping) // static
{
    // 空の場合は新規作成する
    if (targetTree.isValid() == false && allowCreationIfInvalid)
    {
        targetTree = juce::ValueTree(targetType);
        return;
    }
    // Type有効であればラップする
    else if (targetTree.hasType(targetType))
    {
        return; // 何もしない
    }
    // Type無効な場合はType有効な子を探し見つかればラップする
    // Type有効な子が無ければ新規作成する
    else if (allowChildWrapping)
    {
        if (allowCreationIfInvalid || targetTree.getChildWithName(targetType).isValid())
        {
            targetTree = targetTree.getOrCreateChildWithName(targetType, um);
            return;
        }
    }
    
    targetTree = juce::ValueTree();
}

} // vtwrapper
