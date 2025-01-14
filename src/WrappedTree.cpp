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
void WrappedTree::wrap(juce::ValueTree targetTree, const juce::Identifier& targetType, juce::UndoManager* um, bool allowCreationIfInvalid, bool allowChildWrapping)
{
    jassert(targetType.isValid());
    
    valueTree = juce::ValueTree();
    typeId = targetType;
    undoManager = um;
    
    // 空の場合は新規作成する
    if (targetTree.isValid() == false && allowCreationIfInvalid)
    {
        targetTree = juce::ValueTree(targetType);
        valueTree = targetTree;
    }
    // Type有効であればラップする
    else if (targetTree.hasType(targetType))
    {
        valueTree = targetTree;
    }
    // Type無効な場合はType有効な子を探し見つかればラップする
    // Type有効な子が無ければ新規作成する
    else if (allowChildWrapping)
    {
        if (allowCreationIfInvalid || targetTree.getChildWithName(targetType).isValid())
        {
            valueTree = targetTree.getOrCreateChildWithName(targetType, undoManager);
        }
    }
    
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

} // vtutil
