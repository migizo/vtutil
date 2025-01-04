/*
  ==============================================================================

    WrappedTree.h
    Author:  migizo

  ==============================================================================
*/

#pragma once
#include <juce_data_structures/juce_data_structures.h>

namespace vtutil
{

//! リダイレクト(valueTreeに対する「=」を使用した参照先変更処理)を使用すると管理が複雑になるため
//! 間違って使用してしまわないように監視するためのクラス
class RedirectChecker final
: public juce::ValueTree::Listener
{
public:
    ~RedirectChecker() override = default;
    
    // juce::ValueTree::Listener
    void valueTreeRedirected(juce::ValueTree& changedTree) override
    {
        DBG("[RedirectChecker] : " << (changedTree.getType().isValid() ? changedTree.getType() : "invalid type") << " is redirected.");
        jassertfalse;
    }
};

//==============================================================================
/**
 @brief juce::ValueTreeをラップする基底クラス
 juce::ValueTree,juce::UndoManager,juce::Identifier型のValueTreeのTypeIDをメンバに持つため、派生クラスのメンバにそれぞれの用意は不要になる。
 wrapOrCreate()でValueTreeとの紐付けを行い、派生クラスでwrapPropertiesAndChildren()に各プロパティや子要素の紐付けを用意することで、管理が容易になる。
 redirect時にwrapOrCreate()を呼ぶ実装も考えられるが,現在の想定ではこの対応は行わず呼び出し側での責務とする。
 */
struct WrappedTree
{
public:
    //! デフォルトコンストラクタ。紐付けされていないためwrapOrCreate()を呼び出す必要がある
    WrappedTree() = default;
    
    virtual ~WrappedTree() = default;
    
    /**
     @brief 引数に与えられた情報を紐付けし,場合によってはValueTreeを構築する。
     与えられたValueTreeに対しては以下の操作を行う。
     - 空...新規作成する。
     - targetIdを持つ...ラップする。
     - targetIdを持たない&子がtargetIdを持つ...子をラップする
     - targetIdを持たない&子もtargetIdを持たない...子を新規作成しラップする
     */
    void wrapOrCreate(juce::ValueTree targetTree, const juce::Identifier& targetId, juce::UndoManager* um);
    
    //! wrapOrCreate()で呼ばれ、呼び出し側でpropertyやchildrenの構築や紐付けを行う
    virtual void wrapPropertiesAndChildren() = 0;
    
    void deepCopyFrom(WrappedTree* copySource);
    
    bool isValid() const;
    const juce::ValueTree& getValueTree() const noexcept { return valueTree; }
    const juce::Identifier& getTypeID() const noexcept { return typeId; }
    juce::UndoManager* getUndoManager() noexcept { return undoManager; }
    
protected:
    juce::ValueTree valueTree;
    juce::UndoManager* undoManager = nullptr;
    juce::Identifier typeId;
    
private:
    void resetTree(juce::ValueTree targetTree);
};

} // namespace vtutil
