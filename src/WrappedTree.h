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

//==============================================================================
/**
 @brief juce::ValueTreeをラップする基底クラス
 このクラスを継承することでjuce::ValueTreeをメンバに持つ静的型付なクラスとして扱うことが可能になる。
 wrapPropertiesAndChildren()をoverrideし、ValueTreeがセットされた時の各プロパティや子要素の紐付けを行うことが可能。
 */
class WrappedTree
{
public:
    //! デフォルトコンストラクタ。紐付けされていないためwrap()を呼び出す必要がある
    WrappedTree() = default;
    
    virtual ~WrappedTree() = default;
    
    /**
     @brief 引数に与えられた情報を紐付けし,場合によってはValueTreeを構築する初期化処理。
     与えられたValueTreeに対しては以下の操作を行う。
     - 無効なValueTree...ValueTreeを新規作成する。
     - (引数に与えられた)targetTypeと同じTypeのValueTree...ラップする。
     - targetTypeと同じTypeを持たない&子がtargetTypeと同じTypeを持つValueTree...子をラップする
     - targetTypeと同じTypeを持たない&子がtargetTypeと同じTypeを持たないValueTree...子を新規作成しラップする
    
     @n 上記により有効なValueTreeおよびTypeを持つ場合は有効な状態となり、wrapPropertiesAndChildren()の呼び出しを行う。
     @n なお、既にwrap()が呼び出されWrappedTreeが有効な場合に、再度無効なValueTreeなどがWrappedTreeで渡された場合はWrappedTreeは無効になる。
     @n 内部で仮想関数を呼び出すためWrappedTreeを継承したクラスのコンストラクタおよびデストラクタで呼び出すことはできない。
     @param createIfInvalid 対象のValueTreeが無効な場合にValueTreeを作成するかどうか。既に有効なValueTreeであることが明確な場合はfalseに指定する。
     @param allowChildWrapping targetTreeの子ValueTreeをwrapの探索対象に含めるかどうか。子を対象に含めないことが明確な場合はfalseに指定する。
     */
    void wrap(juce::ValueTree targetTree, const juce::Identifier& targetType, juce::UndoManager* um, bool allowCreationIfInvalid = true, bool allowChildWrapping = true);
    
    //! @brief コピーソースのプロパティと子で置き換えた後にwrapPropertiesAndChildren()を呼び出す
    //! 既にコピー元およびコピー先が有効な状態のWrappedTreeかつ同じTypeを持つ必要があり、そうでない場合は何も行わない。
    void copyPropertiesAndChildrenFrom(const WrappedTree& copySource);
    
    //! @brief 既にwrap()による紐付け処理を行い有効な状態であるか
    bool isValid() const;
    
    const juce::ValueTree& getValueTree() const noexcept { return valueTree; }
    const juce::Identifier& getTypeID() const noexcept { return typeId; }
    juce::UndoManager* getUndoManager() noexcept { return undoManager; }
    
    static void updateTreeIfNeeded(juce::ValueTree& targetTree, const juce::Identifier& targetType, juce::UndoManager* um, bool allowCreationIfInvalid, bool allowChildWrapping);
    
protected:
    //! @brief wrap()でValueTreeがセットされた時の各プロパティや子要素の紐付けを行う初期化処理
    virtual void wrapPropertiesAndChildren() = 0;
    
    juce::ValueTree valueTree;
    juce::UndoManager* undoManager = nullptr;
    juce::Identifier typeId;
    
private:
    
};

} // namespace vtutil
